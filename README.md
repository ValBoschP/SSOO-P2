# Sistemas Operativos  2022-2023
## Proyecto SHELL: Práctica de progamación de aplicaciones
### _Introducción_
Vamos a desarrollar una pequeña **Shell**, similar en funcionalidad, en un nivel **muy básico**, con C++. Crearemos el programa **copyfile** con una funcionalidad similar a la de los comandos cp y mv. Y después profundizamos en hacer la **Shell** como tal.

### _Primer objetivo: Copyfile_
Crear un programa llamado _copyfile_, muy similar al conocido cp. Al invocarlo así:
```
$ ./copyfile ruta/de/origen raut/de/destino
```
Este copiará el archivo _ruta/de/origen_ en _ruta/de/destino_, sobreescribiendo el archivo