---
title: Inicio
authors:
- Andrés Leonardo Hernández Bermúdez
---

# Repositorio de actividades

## Sistemas Operativos 2025-1

En este repositorio estaremos manejando las tareas de la materia de [Sistemas Operativos][pagina-curso] que se imparte en la Facultad de Ciencias, UNAM en el [semestre 2025-1][detalles].

## Entregas

<!--
- La entrega de cada actividad es **individual**
- Esto quedó definido en la [presentación del curso][presentacion]
-->

- Cada actividad (tarea, práctica o proyecto) especifica si la entrega será individual o en equipo.
- Las entregas **en equipo** de `4` personas

!!! danger
    - **No se aceptan entregas individuales si la actividad se pidió para ser entregada en equipo**
    - Esto quedó definido en la [presentación del curso][presentacion]

## Organización de las entregas

Cada entrega debe cumplir con los siguientes lineamientos:

- Crear una rama para cada actividad utilizando como base la rama `entregas`
    - `git checkout entregas`
    - `git checkout -b ACTIVIDAD`
- Levantar un [_merge request_][merge-requests] para entregar la actividad
    - El _flujo de trabajo_ viene documentado en [la sección `workflow`][flujo-de-trabajo] de la página del curso
- Los alumnos entregarán sus actividades (tareas, prácticas y proyectos) dentro de la carpeta correspondiente
- Cada tarea o práctica deberá tener lo siguiente:
    - Un archivo `README.md` donde se entregará la documentación o reporte en formato [_markdown_][guia-markdown]
    - Código de los programas solicitados en lenguaje `C`
    - Archivo `Makefile` con instrucciones para compilación
    - _Script_ de _shell_ y archivos necesarios para ejecutar las pruebas del programa
    - Incluir un directorio `img` para guardar las imágenes o capturas de pantalla necesarias para la documentación
    - Incluir un directorio `files` para guardar los archivos adicionales de la entrega en caso de ser necesario

### Estructura de directorios

La estructura de directorios que se manejará para las entregas tiene una carpeta principal llamada `docs` y dentro una carpata para cada tipo de actividad: `tareas`, `practicas` o `proyectos`.

- Dentro de cada carpeta se creará una sub-carpeta para identificar la actividad (`tarea-1`, `practica-2`, etc.) y dentro otra carpeta con el nombre del alumno o el nombre del equipo según sea el caso.

| Actividad	| Tipo de entrega		| Nombre de carpeta
|:-------------:|:-----------------------------:|------------------:|
| Tareas	| **Entrega individual**	| `tareas/tarea-1/ApellidoApellidoNombreNombre`
| Prácticas	| <span class="red">Entrega en equipo</span>	| `practicas/practica-2/Equipo-AAAA-BBBB-CCCC-DDDD`
| Examenes	| **Entrega individual**	| `examenes/examen-3/ApellidoApellidoNombreNombre`
<!--
| Proyectos	| <span class="red">Entrega en equipo</span>	| `proyectos/proyecto-final/Equipo-AAAA-BBBB-CCCC-DDDD`
-->

- Restricciones

    - No utilizar **espacios**, **acentos**, **eñe**, **diéresis** ni **caracteres especiales** en ningún nombre de archivo o carpeta
    - Cada carpeta de `tarea`, `práctica` o proyecto deberá tener un archivo `README.md` donde los alumnos listan la documentación que se entrega con la actividad solicitada
    - Cada alumno creará una carpeta con su nombre `ApellidoApellidoNombreNombre`, donde realizará la entrega de la actividad
    - Cada equipo creará una carpeta llamada `Equipo-AAAA-BBBB-CCCC-DDDD`, donde se realizará la entrega
        - `AAAA`, `BBBB`, `CCCC` y `DDDD` son las iniciales de los integrantes del equipo iniciando con los apellidos

Los directorios están organizados de la siguiente manera:

```
docs/
├── tareas/
│   ├── tarea-1/
│   │   ├── HernandezBermudezAndresLeonardo/
│   │   │   ├── files/
│   │   │   │   └── ...
│   │   │   ├── Makefile
│   │   │   ├── programa.c
│   │   │   └── README.md
│   │   └── TorresRodriguezJoseLuis/
│   │       └── ...
│   ├── tarea-2/
│   │   ├── HernandezBermudezAndresLeonardo/
│   │   │   ├── ...
│   │   └── TorresRodriguezJoseLuis/
│   │       └── ...
│   └── ...
└── practicas/
    ├── practica-1/
    │   └── Equipo-AAAA-BBBB-CCCC-DDDD/
    │       └── ...
    └── practica-2/
        └── ...
```

--------------------------------------------------------------------------------

| [_Merge Requests_][merge-requests]
|:--:|
| [![Pipeline Status][pipeline-status]][pipeline-list]

[pipeline-list]: https://gitlab.com/SistemasOperativos-Ciencias-UNAM/2025-1/tareas-so/-/pipelines
[pipeline-status]: https://gitlab.com/SistemasOperativos-Ciencias-UNAM/2025-1/tareas-so/badges/main/pipeline.svg
[merge-requests]: https://gitlab.com/SistemasOperativos-Ciencias-UNAM/2025-1/tareas-so/-/merge_requests

[repositorio-tareas]: https://gitlab.com/SistemasOperativos-Ciencias-UNAM/2025-1/tareas-so
[presentacion]: https://sistemasoperativos-ciencias-unam.gitlab.io/curso/
[detalles]: https://sistemasoperativos-ciencias-unam.gitlab.io/curso/
[horarios-plan-1994]: http://www.fciencias.unam.mx/docencia/horarios/20251/218/713
[horarios-plan-2013]: http://www.fciencias.unam.mx/docencia/horarios/20251/1556/713

[guia-markdown]: https://about.gitlab.com/handbook/markdown-guide/

[pagina-curso]: https://SistemasOperativos-Ciencias-UNAM.gitlab.io/
[pagina-tareas]: https://sistemasoperativos-ciencias-unam.gitlab.io/2025-1/tareas-so
[grupo-gitlab]: https://gitlab.com/SistemasOperativos-Ciencias-UNAM/
[lista]: https://tinyurl.com/ListaSO-2025-1
[pizarron]: https://tinyurl.com/SO-Ciencias-UNAM-Pizarron
[telegram]: https://t.me/sistemasoperativos_ciencias_unam
[temario]: https://www.fciencias.unam.mx/asignaturas/713.pdf

[flujo-de-trabajo]: https://sistemasoperativos-ciencias-unam.gitlab.io/workflow/

[youtube]: https://tinyurl.com/SO-Ciencias-UNAM-YouTube
[youtube-lista-semestre]: https://tinyurl.com/SO-Ciencias-UNAM-YT-2025-1
[youtube-lista-temas]: https://tinyurl.com/SO-Ciencias-UNAM-YT-temas
[youtube-lista-complementarios]: https://tinyurl.com/SO-Ciencias-UNAM-YT-temascomp

[jitsi]: https://tinyurl.com/JitsiSO-2025-1
[google-meet]: https://tinyurl.com/MeetSO-2025-1
[google-drive]: https://tinyurl.com/DriveSO-2025-1
[zoom]: https://tinyurl.com/ZoomSO-2025-1
