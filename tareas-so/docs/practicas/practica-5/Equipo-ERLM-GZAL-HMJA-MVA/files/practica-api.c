#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <microjson.h>  // Biblioteca para procesamiento JSON
#include <memcached.h>  // Biblioteca para memcached
#include <syslog.h>      // Biblioteca para manejo de logs

#define PORT 80  // Establecer puerto 80 como predeterminado
#define BUFF_SIZE 1024
#define SOCKET_P "/run/memcached.sock"
// Definir la ruta del archivo de log
#define LOG_FILE "/var/log/practica-api.log"

void version_request(FILE *outgoing);
void not_found(FILE *outgoing);
void not_allowed(FILE *outgoing);
void handle_get(FILE *outgoing, const char *endpoint);
void handle_post(FILE *outgoing, const char *endpoint, const char *json_data);
void handle_put(FILE *outgoing, const char *endpoint, const char *json_data);
void handle_patch(FILE *outgoing, const char *endpoint, const char *json_data);
void handle_delete(FILE *outgoing, const char *endpoint);
void* handle_client(void* client_socket_ptr);
const char* get_category(int value);

void send_json_response(FILE *outgoing, int status, const char *message) {
    fprintf(outgoing, 
        "HTTP/1.1 %d OK\r\n"
        "Content-Type: application/json; charset=UTF-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n"
        "Content-Length: %zu\r\n\r\n%s",
        status, strlen(message), message
    );
}

const char* get_category(int value) {
    if (value == 0) return "Sin datos";
    if (value <= 50) return "Buena";
    if (value <= 100) return "Regular";
    if (value <= 150) return "Mala";
    if (value <= 200) return "Muy mala";
    return "Extremadamente mala";
}

void version_request(FILE *outgoing) {
    const char *json_response = 
        "{\n"
        "  \"status\": {\n"
        "    \"version\": \"0.0.1\",\n"
        "    \"semestre\": \"2025-1\",\n"
        "    \"equipo\": \"Equipo-ERLM-GZAL-HMJA-MVA\",\n"
        "    \"integrantes\": {\n"
        "      \"420003818\": \"Escobar Rosales Luis Mario\",\n"
        "      \"316127927\": \"Garcia Zarraga Angelica Lizbeth\",\n"
        "      \"315137903\": \"Hernandez Morales Jose Angel\",\n"
        "      \"317042766\": \"Maldonado Vazquez Alejandro\"\n"
        "    }\n"
        "  }\n"
        "}";

    fprintf(outgoing, 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json; charset=UTF-8\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Connection: close\r\n"
            "Content-Length: %zu\r\n\r\n%s",
            strlen(json_response), json_response);
}

void not_found(FILE *outgoing) {
    fprintf(outgoing, "HTTP/1.1 404 Not Found\r\n\r\n404 Not Found\n");
}

void not_allowed(FILE *outgoing) {
    fprintf(outgoing, "HTTP/1.1 405 Method Not Allowed\r\n\r\n405 Method Not Allowed\n");
}

void handle_get(FILE *outgoing, const char *endpoint) {
    if (strcmp(endpoint, "/") == 0 || strcmp(endpoint, "/version") == 0) {
        version_request(outgoing);
    } else if (strcmp(endpoint, "/estado") == 0) {
        int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("Error creando socket");
            not_found(outgoing);
            return;
        }

        struct sockaddr_un server_addr = {0};
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SOCKET_P);

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error conectando al socket");
            not_found(outgoing);
            close(server_socket);
            return;
        }

        const char *cmd = "stats\n";
        send(server_socket, cmd, strlen(cmd), 0);
        char buffer[BUFF_SIZE];
        int bytes = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            fprintf(outgoing, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n%s", buffer);
        } else {
            not_found(outgoing);
        }

        close(server_socket);
    } else if (strcmp(endpoint, "/tabla") == 0) {
        int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("Error creando socket");
            not_found(outgoing);
            return;
        }

        struct sockaddr_un server_addr = {0};
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SOCKET_P);

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error conectando al socket");
            not_found(outgoing);
            close(server_socket);
            return;
        }

        const char *cmd = "get *\n";
        send(server_socket, cmd, strlen(cmd), 0);
        char buffer[BUFF_SIZE];
        int bytes = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            fprintf(outgoing, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: %d\r\n\r\n%s", bytes, buffer);
        } else {
            not_found(outgoing);
        }

        close(server_socket);
    } else {
        // Lógica para servir archivos estáticos
        char full_path[512];
        snprintf(full_path, sizeof(full_path), "%s%s", ROOT_DIR, endpoint);

        struct stat file_stat;
        if (stat(full_path, &file_stat) == -1) {
            not_found(outgoing);
            return;
        }

        FILE *file = fopen(full_path, "rb");
        if (file == NULL) {
            not_found(outgoing);
            return;
        }

        // Obtener el tipo MIME
        const char *mime_type = get_mime_type(endpoint);
        
        // Enviar el encabezado HTTP con el tipo MIME correcto
        char header[1024];
        snprintf(header, sizeof(header), "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", mime_type);
        fprintf(outgoing, "%s", header);

        // Leer el archivo y enviarlo al cliente
        char file_buffer[1024];
        size_t bytes_read;
        while ((bytes_read = fread(file_buffer, 1, sizeof(file_buffer), file)) > 0) {
            fwrite(file_buffer, 1, bytes_read, outgoing);
        }

        fclose(file);
    }
}

void handle_post(FILE *outgoing, const char *endpoint, const char *json_data) {
    if (strcmp(endpoint, "/dia") == 0) {
        json_value_t *root = json_parse(json_data, strlen(json_data));
        
        if (!root) {
            fprintf(outgoing, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        json_value_t *fecha_value = json_object_get(root, "dia.fecha");
        json_value_t *valores_value = json_object_get(root, "dia.valores");

        if (!fecha_value || !json_is_array(valores_value)) {
            json_free(root);
            fprintf(outgoing, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        const char *fecha = json_string_value(fecha_value);
        size_t valores_count = json_array_size(valores_value);
        char values_str[BUFF_SIZE] = "";

        for (size_t i = 0; i < valores_count; i++) {
            json_value_t *value = json_array_get(valores_value, i);
            if (i > 0) strcat(values_str, "\t");
            strcat(values_str, json_integer_value(value));
        }

        int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("Error creando socket");
            json_free(root);
            not_found(outgoing);
            return;
        }

        struct sockaddr_un server_addr = {0};
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SOCKET_P);

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error conectando al socket");
            json_free(root);
            not_found(outgoing);
            close(server_socket);
            return;
        }

        char cmd[BUFF_SIZE];
        snprintf(cmd, sizeof(cmd), "set %s 0 0 %lu\r\n%s\r\n", fecha, strlen(values_str), values_str);
        send(server_socket, cmd, strlen(cmd), 0);

        char buffer[BUFF_SIZE];
        int bytes = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            fprintf(outgoing, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
        } else {
            json_free(root);
            not_found(outgoing);
        }

        close(server_socket);
        json_free(root);
    } else {
        not_found(outgoing);
    }
}

void handle_put(FILE *outgoing, const char *endpoint, const char *json_data) {
    if (strcmp(endpoint, "/dia") == 0) {
        json_value_t *root = json_parse(json_data, strlen(json_data));

        if (!root) {
            fprintf(outgoing, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        json_value_t *fecha_value = json_object_get(root, "dia.fecha");
        json_value_t *valores_value = json_object_get(root, "dia.valores");

        if (!fecha_value || !json_is_array(valores_value)) {
            json_free(root);
            fprintf(outgoing, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        const char *fecha = json_string_value(fecha_value);
        size_t valores_count = json_array_size(valores_value);
        char values_str[BUFF_SIZE] = "";

        for (size_t i = 0; i < valores_count; i++) {
            json_value_t *value = json_array_get(valores_value, i);
            if (i > 0) strcat(values_str, "\t");
            strcat(values_str, json_integer_value(value));
        }

        int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("Error creando socket");
            json_free(root);
            not_found(outgoing);
            return;
        }

        struct sockaddr_un server_addr = {0};
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SOCKET_P);

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error conectando al socket");
            json_free(root);
            not_found(outgoing);
            close(server_socket);
            return;
        }

        char cmd[BUFF_SIZE];
        snprintf(cmd, sizeof(cmd), "set %s 0 0 %lu\r\n%s\r\n", fecha, strlen(values_str), values_str);
        send(server_socket, cmd, strlen(cmd), 0);

        char buffer[BUFF_SIZE];
        int bytes = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            fprintf(outgoing, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
        } else {
            json_free(root);
            not_found(outgoing);
        }

        close(server_socket);
        json_free(root);
    } else {
        not_found(outgoing);
    }
}

void handle_patch(FILE *outgoing, const char *endpoint, const char *json_data) {
    if (strcmp(endpoint, "/dia") == 0) {
        json_value_t *root = json_parse(json_data, strlen(json_data));

        if (!root) {
            fprintf(outgoing, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        json_value_t *fecha_value = json_object_get(root, "dia.fecha");
        json_value_t *valores_value = json_object_get(root, "dia.valores");

        if (!fecha_value || !json_is_array(valores_value)) {
            json_free(root);
            fprintf(outgoing, "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
            return;
        }

        const char *fecha = json_string_value(fecha_value);
        size_t valores_count = json_array_size(valores_value);
        char values_str[BUFF_SIZE] = "";

        for (size_t i = 0; i < valores_count; i++) {
            json_value_t *value = json_array_get(valores_value, i);
            if (i > 0) strcat(values_str, "\t");
            strcat(values_str, json_integer_value(value));
        }

        int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("Error creando socket");
            json_free(root);
            not_found(outgoing);
            return;
        }

        struct sockaddr_un server_addr = {0};
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SOCKET_P);

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error conectando al socket");
            json_free(root);
            not_found(outgoing);
            close(server_socket);
            return;
        }

        char cmd[BUFF_SIZE];
        snprintf(cmd, sizeof(cmd), "set %s 0 0 %lu\r\n%s\r\n", fecha, strlen(values_str), values_str);
        send(server_socket, cmd, strlen(cmd), 0);

        char buffer[BUFF_SIZE];
        int bytes = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            fprintf(outgoing, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
        } else {
            json_free(root);
            not_found(outgoing);
        }

        close(server_socket);
        json_free(root);
    } else {
        not_found(outgoing);
    }
}

void handle_delete(FILE *outgoing, const char *endpoint) {
    if (strcmp(endpoint, "/dia") == 0) {
        int server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        if (server_socket < 0) {
            perror("Error creando socket");
            not_found(outgoing);
            return;
        }

        struct sockaddr_un server_addr = {0};
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, SOCKET_P);

        if (connect(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Error conectando al socket");
            not_found(outgoing);
            close(server_socket);
            return;
        }

        const char *cmd = "flush_all\r\n";
        send(server_socket, cmd, strlen(cmd), 0);

        char buffer[BUFF_SIZE];
        int bytes = recv(server_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            fprintf(outgoing, "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nAccess-Control-Allow-Origin: *\r\nConnection: close\r\nContent-Length: 0\r\n\r\n");
        } else {
            not_found(outgoing);
        }

        close(server_socket);
    } else {
        not_found(outgoing);
    }
}

void* handle_client(void* client_socket_ptr) {
    int client_socket = ((int)client_socket_ptr);
    free(client_socket_ptr);
    FILE *client_stream = fdopen(client_socket, "r+");
    char buffer[BUFF_SIZE];
    char *method, *endpoint, *http_version, *json_data;

    fgets(buffer, sizeof(buffer), client_stream);
    method = strtok(buffer, " ");
    endpoint = strtok(NULL, " ");
    http_version = strtok(NULL, "\r\n");

    // Procesar datos JSON si es necesario
    if (strcmp(method, "POST") == 0 || strcmp(method, "PUT") == 0 || strcmp(method, "PATCH") == 0) {
        fgets(buffer, sizeof(buffer), client_stream); // Leer encabezado Content-Length
        int content_length = atoi(strtok(NULL, " "));
        json_data = (char *)malloc(content_length + 1);
        fread(json_data, 1, content_length, client_stream);
    } else {
        json_data = NULL;
    }

    if (strcmp(method, "GET") == 0) {
        handle_get(client_stream, endpoint);
    } else if (strcmp(method, "POST") == 0) {
        handle_post(client_stream, endpoint, json_data);
    } else if (strcmp(method, "PUT") == 0) {
        handle_put(client_stream, endpoint, json_data);
    } else if (strcmp(method, "PATCH") == 0) {
        handle_patch(client_stream, endpoint, json_data);
    } else if (strcmp(method, "DELETE") == 0) {
        handle_delete(client_stream, endpoint);
    } else {
        not_allowed(client_stream);
    }

    if (json_data) free(json_data);
    fclose(client_stream);
    return NULL;
}

int main() {
    // Abrir archivo de log
    FILE *log_file = fopen(LOG_FILE, "a");
    if (!log_file) {
        perror("Error abriendo el archivo de log");
        exit(EXIT_FAILURE);
    }
    fprintf(log_file, "Servidor iniciado en puerto %d\n", PORT);

    // Crear el socket del servidor
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Error creando socket");
        fclose(log_file);  // Cerrar el archivo de log antes de salir
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Vincular el socket al puerto
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al bindear el socket");
        fclose(log_file);  // Cerrar el archivo de log antes de salir
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    listen(server_socket, 5);
    fprintf(log_file, "Servidor escuchando en puerto %d\n", PORT);

    // Aceptar conexiones y crear un hilo para cada cliente
    while (1) {
        int *client_socket_ptr = malloc(sizeof(int));
        *client_socket_ptr = accept(server_socket, NULL, NULL);
        if (*client_socket_ptr < 0) {
            perror("Error aceptando conexión");
            free(client_socket_ptr);
            continue;
        }

        pthread_t thread;
        pthread_create(&thread, NULL, handle_client, client_socket_ptr);
        pthread_detach(thread);  // Desprender el hilo para que se libere automáticamente
    }

    // Cerrar el servidor y el archivo de log
    fclose(log_file);  // Cerrar el archivo de log antes de salir
    close(server_socket);
    return 0;
}
