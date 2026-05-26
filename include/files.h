#ifndef FILES_H
#define FILES_H

#define PUBLIC_DIR "./www" // Carpeta raíz y jaula de seguridad del servidor

int get_safe_path(const char *uri, char *safe_path); // Traduce la URI web en una ruta física absoluta y valida su seguridad contra saltos de directorio
long get_file_size(const char *safe_path);           // Obtiene el tamaño exacto en bytes del archivo para el encabezado Content-Length

#endif