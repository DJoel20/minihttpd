#ifndef SERVER_H
#define SERVER_H

int iniciar_servidor(const char *puerto); // Crea el socket principal, configura SO_REUSEADDR, enlaza el puerto mediante bind y lo pone en modo escucha activa con listen

#endif