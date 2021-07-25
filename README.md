# EOS

Ejemplo académico de la implementación de un sistema operativo expropiativo desarrollado para un procesador Cortex-M4.

## Acerca de este proyecto

La primera versión de este proyecto se desarrolló un ejemplo para el dictado de las siguiente asignaturas de postgrado:

- "Sistemas operativos de Tiempo Real" de la Especialización en Sistemas Embebidos, dictada por la Facultad de Ingeniería, en la Universidad Nacional de Entre Rios

- "Sistemas embebidos de Tiempo Real" de la Especialización en Integración de Tecnologías Informáticas, dictada por la Facultad de Ciencias Exactas y Tecnología, en la Universidad Nacional de Tucumán

En el mismo se muestra la creación de dos tareas y la implementación de un cambio de contexto expropiativo en un procesador Cortex-M4 utilizando la interrupción del SysTick. El ejemplo implementado está desarrollado casi completamente en lenguaje `C`, utilizando muy pocas lineas de lenguaje ensamblador embebido dentro de la misma rutina de servicio, la cual utiliza una directiva de `GCC` para eliminar el prologo y el epílogo incluido por el compilador en las funciones normales.

A partir de este ejemplo básico se comienza a desarrollar un sistema operativo más completo, el cual se presentará como trabajo para aprobar la asignatura “Implementación de Sistemas Operativos”, cursada en la Maestría de Sistemas Embebidos, dictada por la Facultad de Ingeniería, de la Universidad de Buenos Aires.

Para mantener el carácter académico del proyecto, en este repositorio se generarán una serie de etiquetas de `release` para marcar cada nuevo hito del mismo, lo que permita a cualquier interesado ver claramente los cambios necesarios para agregar cada nueva funcionalidad que se desarrolla.

## Para compilar este proyecto

Este proyecto está desarrollado dentro del entorno del proyecto (CIAA)[www.proyecto-ciaa.com.ar] y utiliza el firmware oficial (CIAA-V2)[https://github.com/ciaa/firmware_v2] como base que proporciona una capa de abstracción de acceso al hardware y el proceso compilación. Para compilar el proyecto es necesario:

- Clonar el firmware oficial CIAA-V2.

- Clonar este repositorio.

- Crear un archivo `project.mk` en la carpeta que contiene el firmware CIAA, donde se define la variable `PROJECT=carpeta`, donde `carpeta` es la ruta absoluta o relativa de la carpeta que contiene a este proyecto.

- Ejecutar el comando `make` en la carpeta que contiene al firmware de CIAA para compilar el proyecto.

- Ejecutar el comando `make download` para grabar el proyecto en la placa.

## En este release

En esta versión del proyecto se mejora la implementación utilizando las caracteristicas del Cortex-M4 para proteger al sistema operativo: se utiliza un puntero de pila indepentdiente (PSP) para lar tareas y se activa el modo de ejecución no privilegiado al terminar el cambio de contexto.
