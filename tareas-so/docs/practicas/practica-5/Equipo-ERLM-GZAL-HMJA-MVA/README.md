# **Práctica 5**

## *Equipo ERLM-GZAL-HMJA-MVA*

* Escobar Rosales Luis Mario 420003818 @luismarioescobar
* Garcia Zarraga  Angelica Lizbeth 316127927 @Angy_14
* Hernández Morales José Angel 315137903 @JoseAngelHernandezMorales
* Maldonado Vázquez Alejandro 317042766 @amaldov

## Objetivo

Este proyecto tiene como objetivo familiarizarse con el uso de sockets para establecer comunicación entre dos equipos y desarrollar un servidor web orientado a procesos o hilos para transmitir información a través del protocolo HTTP. Además, se busca implementar un servidor web que conteste las peticiones de un API REST recibiendo y regresando datos en formato JSON.

## Contenido del Proyecto

### Código fuente:

- `daemon.c`: Código fuente principal del Daemon.
- `log.c`: Manejo de registros/logs.
- `util.c`: Utilidades auxiliares para funciones diversas.
- `lockfile.c`: Manejo de archivos de bloqueo.
- `socket.c`: Implementación de conexiones de socket.
- `confdata.c`: Manejo de configuración de datos.
- `daemon-child-func.c`: Funciones relacionadas con procesos hijos del Daemon.

### Scripts y configuración:

- `pruebas.sh`: Script para realizar pruebas automatizadas del daemon y su API.

- `memcached, memcached.conf, ports.conf, make.conf`: Archivos de configuración para memcached y el servidor web.

- `api.json, memcached-stats.sh`: Archivos necesarios para la API y estadísticas.

## Requisitos

- **Sistema operativo**: Linux
- **Dependencias**:
  - Memcached
  - Apache2
  - GCC
  - Utilidades básicas de Linux como `bash` y `curl`

### Makefile

Define las tareas para la configuración del entorno, compilación, despliegue y pruebas.

## Estructura del Proyecto

```text
.
├── daemon.c
├── log.c
├── util.c
├── lockfile.c
├── socket.c
├── confdata.c
├── daemon-child-func.c
├── memcached
├── memcached.conf
├── ports.conf
├── make.conf
├── api.json
├── memcached-stats.sh
├── pruebas.sh
├── Makefile

```

## Servidor API REST

### Endpoints:

- `/`: Regresa la especificación del API en formato JSON.
- `/version`: Permite ver la versión del API.
- `/estado`: Permite ver el estado del backend memcached.
- `/dia`: Permite ver, agregar, modificar o borrar la información correspondiente a un día.

### Métodos HTTP:

- `GET`: Pide un recurso.
- `POST`: Envía la información para crear un recurso nuevo.
- `DELETE`: Borra un recurso.

### CORS:

Se incluye la cabecera Access-Control-Allow-Origin: * para permitir peticiones AJAX desde cualquier dominio.


### Memcached:

Implementación de conexión a través de sockets TCP/UNIX para la interacción con servidores memcached.


### Procesamiento JSON:

Se procesan las solicitudes y respuestas en formato JSON.

## Instalación y Configuración

## Instalación y Configuración

### 1. Clonar el Repositorio

Clona este proyecto en tu máquina local:
```bash
git clone <https://github.com/JoseAngelHernandezMorales/Practica05.git>
cd <Practica05>
```

### 2. Preparar el Entorno

Ejecuta el siguiente comando para instalar las dependencias y configurar los servicios necesarios:
```bash
make setup
```

Esto:
- Instalará `memcached` y `apache2`.
- Configurará archivos de configuración en los directorios correspondientes.
- Iniciará el servicio Memcached.

### 3. Compilar el Proyecto

Para compilar el proyecto, ejecuta:
```bash
make build
```
Esto generará el ejecutable `daemon` en el directorio raíz.


## *Ejecución de pruebas:*

Compilar el programa

```bash
make
```

Salida del comando make :

```text
Preparando entorno...
Instalando dependencias necesarias...
sudo apt-get update && sudo apt-get install -y memcached apache2
Hit:1 http://security.debian.org/debian-security bookworm-security InRelease
Hit:2 http://deb.debian.org/debian bookworm InRelease
Hit:3 http://deb.debian.org/debian bookworm-updates InRelease
Reading package lists... Done
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
memcached is already the newest version (1.6.18-1).
apache2 is already the newest version (2.4.62-1~deb12u2).
The following packages were automatically installed and are no longer required:
  libdbus-glib-1-2 linux-image-6.1.0-22-amd64 linux-image-6.1.0-23-amd64
Use 'sudo apt autoremove' to remove them.
0 upgraded, 0 newly installed, 0 to remove and 108 not upgraded.
sudo install -v -o root -g root -m 0644 memcached /etc/default/memcached
removed '/etc/default/memcached'
'memcached' -> '/etc/default/memcached'
sudo install -v -o root -g root -m 0644 memcached.conf /etc/memcached.conf
removed '/etc/memcached.conf'
'memcached.conf' -> '/etc/memcached.conf'
sudo install -v -o root -g root -m 0644 ports.conf /etc/apache2/ports.conf
removed '/etc/apache2/ports.conf'
'ports.conf' -> '/etc/apache2/ports.conf'
sudo install -v -o root -g root -m 0644 make.conf /etc/apache2/sites-available/make.conf
removed '/etc/apache2/sites-available/make.conf'
'make.conf' -> '/etc/apache2/sites-available/make.conf'
sudo mkdir -p /var/run/memcached && sudo chmod 777 /var/run/memcached
sudo service memcached start
Verificando estado de memcached...
ps aux | grep memcached || echo "Error: memcached no está ejecutándose."
memcache     575  0.0  0.1 412404  4888 ?        Ssl  Dec19   0:02 /usr/bin/memcached -m 64 -s /var/run/memcached/memcached.sock -a 0777 -u memcache -P /var/run/memcached/memcached.pid
algarci+   14409  0.0  0.0   6932  3260 pts/0    S+   00:38   0:00 /bin/bash -c ps aux | grep memcached || echo "Error: memcached no está ejecutándose."
algarci+   14411  0.0  0.0   6332  2060 pts/0    S+   00:38   0:00 grep memcached
Compilando daemon...
gcc -g -fcommon -o daemon daemon.c log.c util.c lockfile.c socket.c confdata.c daemon-child-func.c
Configurando permisos para daemon...
chmod +x daemon
Moviendo archivos de la API...
Configurando permisos para pruebas.sh...
chmod +x pruebas.sh
Ejecutando pruebas...
if [ -f ./pruebas.sh ]; then \
	sudo ./pruebas.sh; \
else \
	echo "Error: El archivo pruebas.sh no existe."; \
fi
Iniciando el daemon
PRUEBA /
*   Trying 127.0.0.1:80...
* Connected to localhost (127.0.0.1) port 80 (#0)
> GET / HTTP/1.1
> Host: localhost
> User-Agent: curl/7.88.1
> Accept: /
> 
< HTTP/1.1 200 OK
< Content-Type: application/json; charset=UTF-8
< Access-Control-Allow-Origin: *
< Connection: close
< Content-Length: 8772
< 
{
  "components": {
    "schemas": {
      "DiaIn": {
        "properties": {
          "fecha": {
            "description": "Llave: YYYY-MM-DD",
            "maxLength": 10,
            "minLength": 0,
            "title": "Fecha",
            "type": "string"
          },
          "valores": {
            "items": {
              "type": "integer"
            },
            "type": "array"
          }
        },
        "required": [
          "fecha"
        ],
        "type": "object"
      },
      "DiaOut": {
        "properties": {
          "fecha": {
            "description": "The ID of the pet.",
            "title": "Pet ID",
            "type": "string"
          },
          "maximo": {
            "type": "integer"
          },
          "valores": {
            "items": {
              "type": "integer"
            },
            "type": "array"
          }
        },
        "type": "object"
      },
      "HTTPError": {
        "properties": {
          "detail": {
            "type": "object"
          },
          "message": {
            "type": "string"
          }
        },
        "type": "object"
      },
      "ValidationError": {
        "properties": {
          "detail": {
            "properties": {
              "<location>": {
                "properties": {
                  "<field_name>": {
                    "items": {
                      "type": "string"
                    },
                    "type": "array"
                  }
                },
                "type": "object"
              }
            },
            "type": "object"
          },
          "message": {
            "type": "string"
          }
        },
        "type": "object"
      }
    }
  },
  "externalDocs": {
    "description": "Find more info here",
    "url": "http://0.0.0.0:9999/docs"
  },
  "info": {
    "contact": {
      "email": "support@example.com",
      "name": "API Support",
      "url": "http://www.example.com/support"
    },
    "description": "...",
    "license": {
      "name": "Apache 2.0",
      "url": "http://www.apache.org/licenses/LICENSE-2.0.html"
    },
    "termsOfService": "http://example.com",
    "title": "APIFlask",
    "version": "0.1.0"
  },
  "openapi": "3.0.3",
  "paths": {
    "/": {
      "get": {
        "parameters": [],
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          }
        },
        "summary": "Get Raiz",
        "tags": [
          "_"
        ]
      }
    },
    "/dia": {
      "patch": {
        "parameters": [],
        "requestBody": {
          "content": {
            "application/json": {
              "schema": {
                "$ref": "#/components/schemas/DiaIn"
              }
            }
          }
        },
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          },
          "422": {
            "content": {
              "application/json": {
                "schema": {
                  "$ref": "#/components/schemas/ValidationError"
                }
              }
            },
            "description": "Validation error"
          }
        },
        "summary": "Create Dia",
        "tags": [
          "Dia"
        ]
      },
      "post": {
        "parameters": [],
        "requestBody": {
          "content": {
            "application/json": {
              "schema": {
                "$ref": "#/components/schemas/DiaIn"
              }
            }
          }
        },
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          },
          "422": {
            "content": {
              "application/json": {
                "schema": {
                  "$ref": "#/components/schemas/ValidationError"
                }
              }
            },
            "description": "Validation error"
          }
        },
        "summary": "Create Dia",
        "tags": [
          "Dia"
        ]
      },
      "put": {
        "parameters": [],
        "requestBody": {
          "content": {
            "application/json": {
              "schema": {
                "$ref": "#/components/schemas/DiaIn"
              }
            }
          }
        },
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          },
          "422": {
            "content": {
              "application/json": {
                "schema": {
                  "$ref": "#/components/schemas/ValidationError"
                }
              }
            },
            "description": "Validation error"
          }
        },
        "summary": "Create Dia",
        "tags": [
          "Dia"
        ]
      }
    },
    "/dia/{dia_id}": {
      "delete": {
        "parameters": [
          {
            "in": "path",
            "name": "dia_id",
            "required": true,
            "schema": {
              "type": "string"
            }
          }
        ],
        "responses": {
          "204": {
            "description": "Empty"
          },
          "404": {
            "content": {
              "application/json": {
                "schema": {
                  "$ref": "#/components/schemas/HTTPError"
                }
              }
            },
            "description": "Not found"
          }
        },
        "summary": "Delete Dia",
        "tags": [
          "Dia"
        ]
      },
      "get": {
        "parameters": [
          {
            "in": "path",
            "name": "dia_id",
            "required": true,
            "schema": {
              "type": "string"
            }
          }
        ],
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {
                  "$ref": "#/components/schemas/DiaOut"
                }
              }
            },
            "description": "Output data of a dia"
          },
          "404": {
            "content": {
              "application/json": {
                "schema": {
                  "$ref": "#/components/schemas/HTTPError"
                }
              }
            },
            "description": "Not found"
          }
        },
        "summary": "Get Dia",
        "tags": [
          "Dia"
        ]
      }
    },
    "/estado": {
      "get": {
        "parameters": [],
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          }
        },
        "summary": "Get Estado",
        "tags": [
          "Estado"
        ]
      }
    },
    "/tabla": {
      "delete": {
        "parameters": [],
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          }
        },
        "summary": "Delete Tabla",
        "tags": [
          "Tabla"
        ]
      },
      "get": {
        "parameters": [],
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          }
        },
        "summary": "Get Tabla",
        "tags": [
          "Tabla"
        ]
      }
    },
    "/version": {
      "get": {
        "parameters": [],
        "responses": {
          "200": {
            "content": {
              "application/json": {
                "schema": {}
              }
            },
            "description": "Successful response"
          }
        },
        "summary": "Get Version",
        "tags": [
          "Version"
        ]
      }
    }
  },
  "servers": [
    {
      "name": "Production Server",
      "url": "http://clase.tonejito.info:9999"
    },
    {
      "name": "Testing Server",
      "url": "http://localhost:9999"
    },
    {
      "name": "Development Server",
      "url": "http://127.0.0.1:9999"
    }
  ],
  "tags": [
    {
      "description": "The description of the *Dia* tag.",
      "name": "Dia"
    }
  ]
}
* Closing connection 0



PRUEBA /version
*   Trying 127.0.0.1:80...
* Connected to localhost (127.0.0.1) port 80 (#0)
> GET /version HTTP/1.1
> Host: localhost
> User-Agent: curl/7.88.1
> Accept: /
> 
< HTTP/1.1 200 OK
< Content-Type: application/json; charset=UTF-8
< Access-Control-Allow-Origin: *
< Connection: close
< Content-Length: 320
< 
{
	"status": {
		"version": "0.0.1",
		"semestre": "2025-1",
		"equipo": "Equipo-ERLM-GZAL-HMJA-MVA",
		"integrantes": {
			"420003818": "Escobar Rosales Luis Mario",
			"316127927": "Garcia Zarraga Angelica Lizbeth",
			"315137903": "Hernandez Morales Jose Angel",
			"317042766": "Maldonado Vazquez Alegandro"
		}
	}
* Closing connection 0
}


PRUEBA /estado
*   Trying 127.0.0.1:80...
* Connected to localhost (127.0.0.1) port 80 (#0)
> GET /estado HTTP/1.1
> Host: localhost
> User-Agent: curl/7.88.1
> Accept: /
> 
< HTTP/1.1 200 OK
< Content-Type: application/json; charset=UTF-8
< Access-Control-Allow-Origin: *
< Connection: close
< Content-Length: 371
< 
{
  "backend": {
    "socket": "/var/run/memcached/memcached.sock",
    "version": "version",
    "stats": {
  "pid": "575",
  "uptime": "9071",
  "time": "1734676705",
  "version": "1.6.18",
  "libevent": "2.1.12-stable",
  "pointer_size": "64",
  "rusage_user": "1.447518",
  "rusage_system": "1.225635",
  "max_connections": "1024",
  "curr_connections": "2"
}

  }
}
* Closing connection 0



PRUEBA /dia
* Could not resolve host: POST
* Closing connection 0
curl: (6) Could not resolve host: POST
*   Trying 127.0.0.1:80...
* Connected to localhost (127.0.0.1) port 80 (#1)
> POST /dia HTTP/1.1
> Host: localhost
> User-Agent: curl/7.88.1
> Accept: /
> Content-Type: application/json
> Content-Length: 92
> 
< HTTP/1.1 201 Created
< Content-Type: application/json; charset=UTF-8
< Access-Control-Allow-Origin: *
< Connection: close
< {"message": "Data added successfully"}
* Closing connection 1
Matando al daemon con id: 14448
```

Limpiar los archivos generados:

```bash
make clean
```

Salida del comando make clean:
```text
Limpiando archivos generados...
rm -fv daemon *~ logfile
removed 'daemon'
removed 'logfile'
```
