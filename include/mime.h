#ifndef MIME_H
#define MIME_H

const char *get_mime_type(const char *filepath); // Analiza la extensión del archivo y devuelve su tipo de contenido estándar (ej: "text/html", "image/png") para la cabecera Content-Type

#endif