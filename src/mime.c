#include <string.h>
#include "../include/mime.h"

const char *get_mime_type(const char *filepath) {
    const char *ext = strrchr(filepath, '.'); // Busca la última aparición del carácter punto para aislar la extensión del archivo

    if (ext == NULL || ext == filepath) {
        return "text/plain"; // Retorna tipo texto plano si el archivo carece de extensión o empieza directamente con un punto
    }

    if (strcmp(ext, ".html") == 0) return "text/html"; // Asocia la extensión dot-html con el tipo de contenido estructurado hipertexto
    if (strcmp(ext, ".css") == 0) return "text/css";   // Mapea la extensión dot-css con las hojas de estilo en cascada del servidor
    if (strcmp(ext, ".js") == 0) return "application/javascript"; // Vincula los archivos JavaScript al identificador MIME oficial de scripts ejecutable
    if (strcmp(ext, ".png") == 0) return "image/png";   // Identifica los gráficos de red portátiles para su renderizado directo de imagen
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";  // Formatea las imágenes de mapas de bits JPEG bajo su estándar de codificación

    return "application/octet-stream"; // Devuelve flujo de bytes genérico por defecto para forzar la descarga de tipos de archivos desconocidos
}