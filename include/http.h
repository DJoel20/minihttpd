#ifndef HTTP_H
#define HTTP_H

// Estructura que almacena los datos de la petición HTTP parseada del cliente
typedef struct {
    char method[16];      // Almacena el método HTTP solicitado (ej: "GET")
    char uri[256];        // Almacena la ruta del recurso web (ej: "/index.html")
    char host[256];       // Guarda el encabezado obligatorio Host (dominio/puerto del servidor)
    char connection[64];  // Guarda el estado de la conexión enviado por el cliente (ej: "keep-alive")
    char user_agent[512]; // Almacena la cadena de identificación del navegador del cliente
    int is_valid;         // Bandera de control para determinar si la petición cumple con el protocolo
} http_request_t;

int parse_http_request(const char *raw_request, http_request_t *parsed_req); // Extrae de forma híbrida la línea inicial y los encabezados básicos del buffer de red
void send_http_response(int client_socket, int status_code, const char *filepath, long file_size); // Construye los encabezados HTTP y transmite el archivo mediante copia cero con sendfile

#endif