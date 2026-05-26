#include <stdio.h>
#include <unistd.h>
#include "../include/server.h"
#include <string.h>
#include <sys/socket.h>
#include "../include/http.h"
#include "../include/files.h"
#include <limits.h>
#include <sys/epoll.h>
#include <fcntl.h>
#define MAX_EVENTS 64

int hacer_no_bloqueante(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK); // Modifica las banderas del descriptor para operar en modo asíncrono no bloqueante
}

int main() {
    printf("Iniciando el servidor MiniHTTPd Concurrente\n");

    int server_fd = iniciar_servidor("8080");
    if (server_fd < 0) {
        fprintf(stderr, "Error fatal: No se pudo iniciar el servidor.\n");
        return 1;
    }

    // socket principal como no-bloqueante
    hacer_no_bloqueante(server_fd);

    printf("¡Éxito! Servidor asíncrono escuchando en el puerto 8080.\n");

    // Crear la instancia de epoll
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("Error al crear la instancia de epoll");
        close(server_fd);
        return 1; // Solicita al núcleo de Linux la creación de un contexto de monitorización de eventos epoll
    }

    // Registrar el socket del servidor en epoll
    struct epoll_event ev, eventos[MAX_EVENTS]; 
    ev.events = EPOLLIN; // lectura
    ev.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        perror("Error en epoll_ctl para el server_fd");
        close(server_fd);
        close(epoll_fd);
        return 1; // Añade el socket pasivo principal a la lista de interés de epoll bajo eventos de lectura por nivel
    }

    while (1) {
        // epoll_wait se duerme 
        int num_eventos = epoll_wait(epoll_fd, eventos, 64, -1);
        if (num_eventos == -1) {
            perror("Error en epoll_wait");
            break; // Bloquea el hilo de ejecución de forma indefinida hasta que ocurran eventos de E/S en los descriptores registrados
        }

        // Iterar en los sockets que esten activos
        for (int i = 0; i < num_eventos; i++) {
            
            // Primer caso llega una nueva conexión de un cliente
            if (eventos[i].data.fd == server_fd) {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd == -1) continue; // Extrae de la cola la conexión TCP entrante devolviendo un descriptor único para el cliente

                hacer_no_bloqueante(client_fd);
                
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
                printf("CONEXIÓN-> Cliente aceptado y registrado en Epoll (FD: %d)\n", client_fd); // Registra el nuevo socket del cliente activando el modo por flanco EPOLLET para maximizar el rendimiento con epoll
            } 
            
            // segundo caso el cliente que ya estaba conectado envia una petición HTTP
            else {
                int client_fd = eventos[i].data.fd;
                char buffer[4096];
                memset(buffer, 0, sizeof(buffer));

                // Lee los datos del cliente
                ssize_t bytes_leidos = read(client_fd, buffer, sizeof(buffer) - 1);
                
                if (bytes_leidos <= 0) {
                    // El cliente cerró la conexión o hubo error
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd);
                    printf("DESCONEXIÓN-> Cliente desconectado (FD: %d)\n", client_fd); // Remueve el descriptor del árbol de monitorización de epoll y libera los recursos del socket cerrado
                } else {
                    // Procesar la petición HTTP de manera segura
                    http_request_t peticion;
                    int estado = parse_http_request(buffer, &peticion);

                    if (estado == 0) {
                        printf("\n PETICIÓN PARSEADA (FD: %d)\n", client_fd);
                        printf("Ruta (URI): %s\n", peticion.uri);
                        // encabezados básicos 
                        printf("Host       : %s\n", peticion.host);
                        printf("Connection : %s\n", peticion.connection);
                        printf("User-Agent : %s\n", peticion.user_agent);

                        char ruta_segura[PATH_MAX];
                        int estado_archivo = get_safe_path(peticion.uri, ruta_segura);

                        if (estado_archivo == 0) {
                            long tamano = get_file_size(ruta_segura);
                            printf("[OK] Enviando 200 OK (%ld bytes)\n", tamano);
                            send_http_response(client_fd, 200, ruta_segura, tamano); // Despacha de forma asíncrona la cabecera HTTP 200 seguida del archivo estático solicitado
                        } else {
                            printf("[ERROR] Código de error HTTP: %d\n", estado_archivo);
                            send_http_response(client_fd, estado_archivo, NULL, 0); // Despacha una respuesta HTTP estructurada con el código de error correspondiente (ej: 403 o 404)
                        }
                    }
                    
                    // cerramos el cliente de epoll
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
                    close(client_fd); // Termina la sesión con el cliente eliminándolo del bucle tras procesar su petición transaccional estática
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}