#include <stdio.h>
#include <string.h>
#include "../include/http.h"
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include "../include/mime.h"

int parse_http_request(const char *raw_request, http_request_t *parsed_req) {
    memset(parsed_req, 0, sizeof(http_request_t)); // Inicializa la estructura de destino limpiando toda la memoria residual
    parsed_req->is_valid = 0;
    if(strstr(raw_request, "\r\n\r\n") == NULL) {
       return 400; // Valida que la petición contenga el doble salto de línea que separa las cabeceras del cuerpo
    }

    char version[16];
    int extraidos = sscanf(raw_request, "%15s %255s %15s",
               parsed_req->method,
               parsed_req->uri,
               version);
    if(extraidos != 3){
       return 400; // Extrae los tres componentes esenciales de la línea de estado inicial mediante sscanf
    }

    if(strcmp(parsed_req->method, "GET") != 0){
    fprintf(stderr, "Metodo no permitido: %s\n", parsed_req->method);
    return 405; // Restringe el procesamiento del servidor validando únicamente peticiones bajo el método GET
}

    char *host_ptr = strstr(raw_request, "Host:");
    if (host_ptr != NULL) {
        sscanf(host_ptr, "Host: %255s", parsed_req->host); // Indexa y extrae el valor del encabezado Host para la validación del dominio solicitado
    }

    //  Extracción de Connection:
    char *conn_ptr = strstr(raw_request, "Connection:");
    if (conn_ptr != NULL) {
        sscanf(conn_ptr, "Connection: %31s", parsed_req->connection); // Captura el parámetro de persistencia de la conexión TCP solicitado por el cliente
    }

    //  Extracción de User-Agent: (Se lee la línea completa hasta el \r\n)
    char *ua_ptr = strstr(raw_request, "User-Agent:");
    if (ua_ptr != NULL) {
        char *start = ua_ptr + 11; // Saltamos "User-Agent:"
        // Quitamos espacios en blanco iniciales si existen
        while (*start == ' ') start++; 
        
        char *end = strstr(start, "\r\n");
        if (end != NULL) {
            int length = end - start;
            if (length > 255) length = 255; // Evitamos desbordar el vector
            snprintf(parsed_req->user_agent, length + 1, "%s", start); // Copia de forma segura el string completo del navegador evitando desbordamientos de memoria
        }
    }

    return 0;
}

void send_http_response(int client_socket, int status_code, const char *filepath, long file_size) {
    char headers[2048];
    const char *status_text;
    const char *content_type = "text/html";
    long content_length = 0;

    switch(status_code) {
        case 200: status_text = "OK"; break;
        case 400: status_text = "Bad Request"; break;
        case 403: status_text = "Forbidden"; break;
        case 404: status_text = "Not Found"; break;
        case 405: status_text = "Method Not Allowed"; break;
        case 500: status_text = "Internal Server Error"; break;
        default:  status_text = "Bad Request"; status_code = 400; break; // Asigna el texto descriptivo correspondiente al código de estado HTTP provisto
    }

    if (status_code == 200) {
        content_type = get_mime_type(filepath); // Resuelve el tipo de contenido dinámicamente llamando al módulo MIME
        content_length = file_size;
    } else {
        content_length = 150; 
    }

    snprintf(headers, sizeof(headers),
             "HTTP/1.1 %d %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: keep-alive\r\n"
             "\r\n",
             status_code, status_text, content_type, content_length); // Estructura el buffer con las cabeceras HTTP formateadas según el estándar del protocolo

    write(client_socket, headers, strlen(headers)); // Transmite el bloque de cabeceras HTTP de forma directa a través del socket del cliente
    if (status_code == 200) {
        int file_fd = open(filepath, O_RDONLY);
        if (file_fd != -1) {
            off_t offset = 0;
            sendfile(client_socket, file_fd, &offset, file_size); // Realiza transferencia de copia cero en espacio de núcleo para enviar el archivo de forma eficiente
            close(file_fd);
        }
    } else {
        char error_body[256];
        snprintf(error_body, sizeof(error_body),
                 "<html><head><title>Error %d</title></head>"
                 "<body><h1>%d %s</h1><p>MiniHTTPd Server</p></body></html>",
                 status_code, status_code, status_text); // Construye una estructura HTML estática para notificar fallos en el navegador del cliente
        
        write(client_socket, error_body, strlen(error_body)); // Despacha el cuerpo del documento de error a través de la red hacia el descriptor activo
    }
}