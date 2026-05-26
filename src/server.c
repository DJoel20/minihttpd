#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include "../include/server.h"

int iniciar_servidor(const char *puerto){

    struct addrinfo hints;
    struct addrinfo *result;
    memset(&hints, 0, sizeof(hints)); // Limpia la estructura hints para evitar basura en la configuración de red

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int estado;
    estado = getaddrinfo(NULL, puerto, &hints, &result);

    if(estado != 0){
      fprintf(stderr, "Error en getaddrinfo: %s/n", gai_strerror(estado));
      return -1; // Traduce el puerto string en estructuras de dirección de red compatibles e independientes de IPv4/IPv6
    }

    int server_fd;
    server_fd = socket(result->ai_family, result->ai_socktype,result->ai_protocol);
    if(server_fd < 0){
      perror("Error al crear el socket");
      freeaddrinfo(result);
      return -1; // Solicita al núcleo de Linux la creación del descriptor de archivo del socket principal del servidor
    }

    int activado = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado)) < 0) {
    perror("Error en setsockopt");
    close(server_fd);
    freeaddrinfo(result);
    return -1; // Configura la opción SO_REUSEADDR para liberar y reutilizar el puerto de forma inmediata tras un reinicio
    }

    if(bind(server_fd, result->ai_addr, result->ai_addrlen) < 0){
      perror("Error en el bind");
      close(server_fd);
      freeaddrinfo(result);
      return -1; // Enlaza el descriptor del socket con la dirección IP y puerto asignados en la interfaz de red
    }

    freeaddrinfo(result); // Libera la memoria dinámica reservada previamente por la función getaddrinfo

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    
    if(sigaction(SIGPIPE, &sa, NULL) < 0){
      perror("Error al configurar sigaction para SIGPIPE");
      close(server_fd);
      return -1; // Ignora la señal SIGPIPE para evitar el colapso del proceso si un cliente cierra abruptamente la conexión TCP
    }

    if(listen(server_fd, SOMAXCONN) < 0){
      perror("Error en poner el socket en esucha");
      close(server_fd);
      return -1; // Define el socket en modo pasivo estableciendo el límite máximo de conexiones en cola con SOMAXCONN
    }
    return server_fd;
}