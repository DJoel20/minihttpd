#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include "../include/files.h"

int get_safe_path(const char *uri, char *safe_path){
    char resolved_public[PATH_MAX];
    char temp_path[PATH_MAX];
    char resolved_target[PATH_MAX];
    
    if(realpath(PUBLIC_DIR, resolved_public) == NULL){
        perror("No se encuenta la carpeta publica www");
        return 500; // Resuelve la ruta absoluta del directorio raíz web de seguridad y maneja fallos críticos del sistema
    }

    const char *target_uri;
    if(strcmp(uri, "/") == 0){
        target_uri= "/index.html"; // Mapea de forma automática la raíz de la URL hacia el archivo por defecto index.html
    }else {
        target_uri = uri;
    }

    snprintf(temp_path, PATH_MAX, "%s%s", resolved_public, target_uri); // Concatena el directorio base con el recurso solicitado en una cadena temporal

    if (realpath(temp_path, resolved_target) == NULL){
        return 404; // Evalúa la existencia real del archivo y expande todos los enlaces y puntos relativos mediante realpath
    }

    if (strncmp(resolved_public, resolved_target, strlen(resolved_public)) != 0){
        return 403; // Compara los prefijos de las rutas absolutas para bloquear intrusiones por escalado de directorios
    }

    strncpy(safe_path, resolved_target, PATH_MAX); // Copia la ruta absoluta validada y segura en el vector de destino para su apertura
    return 0;

}

long get_file_size(const char *safe_path){
    struct stat file_stat;
    if(stat(safe_path, &file_stat) == 0){
        return file_stat.st_size; // Utiliza la estructura nativa stat para consultar los metadatos del nodo-i y extraer su tamaño exacto en bytes
    }
    return -1;
}