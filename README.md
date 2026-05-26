# Instrucciones de Ejecucion y Pruebas

## Compilacion y Ejecucion

```bash
gcc src/main.c src/server.c src/http.c src/files.c src/mime.c -o servidor_minihttp
./servidor_minihttp
```

### Pruebas de Funcionamiento
## 1. Acceso estandar:
```bash
curl -i [http://127.0.0.1:8080/](http://127.0.0.1:8080/)
```
## 3. Escalado de directorios (Directory Traversal):
```bash
curl -i --path-as-is [http://127.0.0.1:8080/../../../../../../etc/passwd](http://127.0.0.1:8080/../../../../../../etc/passwd)
```
## 4. Metodo no permitido (POST)
```bash
curl -i -X POST [http://127.0.0.1:8080/index.html](http://127.0.0.1:8080/index.html)
```
## 5. Recurso inexistente (404)
```bash
curl -i [http://127.0.0.1:8080/archivo_fantasma.html](http://127.0.0.1:8080/archivo_fantasma.html)
