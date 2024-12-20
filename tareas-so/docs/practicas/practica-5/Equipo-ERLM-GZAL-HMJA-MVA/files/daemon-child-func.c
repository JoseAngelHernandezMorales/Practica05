/*
 * (C) Dan Shearer 2003
 *
 * This program is open source software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version. You
 * should have received a copy of the license with this program; if
 * not, go to http://www.fsf.org.
*/

/* daemon_child_func.c

   child function for daemon template. This function never returns, because it
   is what the detached daemon spins off for every connection.

*/

#include <stdio.h>
#include <stdlib.h> /* exit codes and things */
#include <unistd.h> /* _exit */
#include "log.h"
#include "global.h"
#include <sys/socket.h>
#include <string.h>
#include <sys/un.h>


#include "mjson.c"
#include "log.h"

#define PORT 8080
#define BUFF_SIZE 1024
#define SOCKET_PATH "/var/run/memcached/memcached.sock"
#define BACKLOG 10

#define MEMCACHED_VERSION_COMMAND "printf 'version\r\n' | nc -U -w 1 /var/run/memcached/memcached.sock"
#define MEMCACHED_STATS_SCRIPT "/var/www/html/memcached-stats.sh"
#define MEMCACHED_JSON_SCRIPT "/var/www/html/api.json"
#define ERROR_G "Error\r\n"



typedef struct {
    char key[11];
    int values[10];
} table_entry;


int memcache_conection() {
    int sockfd, server_len;
    struct sockaddr_un server_address;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    memset((char *) &server_address, 0, sizeof(server_address));
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SOCKET_PATH);

    server_len = strlen(server_address.sun_path) + sizeof(server_address.sun_family);

    if (connect(sockfd, (struct sockaddr *) &server_address, server_len) < 0) {
        close(sockfd);
        printf("ERROR");
        return -1;
    }

    return sockfd;
}

void handle_command(const char *command, char *result, int max_len) {
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        strncpy(result, "Error", max_len);
        return;
    }

    // Initialize the result buffer to an empty string
    //result[0] = '\0';
    memset(result, '\0', strlen(result));

    // Read each line from the command output and append it to the result
    char buffer[256];  // Temporary buffer for each line

    for (; fgets(buffer, sizeof(buffer), fp) != NULL;) {
        // Ensure we don't exceed max_len when appending
        if (strlen(result) + strlen(buffer) < max_len - 1) {
            strncat(result, buffer, max_len - strlen(result) - 1);
        } else {
            break;  // Prevent buffer overflow
        }
    }

    pclose(fp);
}

int save_data(int sockfd, table_entry *data) {
    char buffer[BUFF_SIZE];
    char values[512] = "";
    snprintf(values, 512, "%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d",
             data->values[0],
             data->values[1],
             data->values[2],
             data->values[3],
             data->values[4],
             data->values[5],
             data->values[6],
             data->values[7],
             data->values[8],
             data->values[9]);
    snprintf(buffer, BUFF_SIZE, "set %s 0 0 %lu\r\n%s\r\n", data->key, strlen(values), values);
    printf("%s", buffer);
    write(sockfd, buffer, strlen(buffer));
    memset(buffer, 0, BUFF_SIZE);
    read(sockfd, buffer, BUFF_SIZE);

    if (strncmp(buffer, "STORED", 6) != 0)
        return -1;
    return 0;

}



void parse_numbers(char *input, int *numbers) {
    int count = 0;
    // Find the first occurrence of "\r\n" using memchr
    char *start = memchr(input, '\r', strlen(input));
    if (start == NULL || *(start + 1) != '\n') return;  // Return if not found

    // Move past the first "\r\n"
    start += 2;

    // Find the second occurrence of "\r\n" using memchr
    char *end = memchr(start, '\r', strlen(start));
    if (end == NULL || *(end + 1) != '\n') return;  // Return if not found

    // Extract the substring between the two "\r\n"
    size_t length = end - start;
    char *substring = (char *)malloc(length + 1);
    if (!substring) return;  // Return if memory allocation fails

    snprintf(substring, length + 1, "%s", start);  // Safely copy the substring

    
    char *ptr = substring;
    char *next_tab;
    count = 0;

    while ((next_tab = strchr(ptr, '\t')) != NULL) {
        *next_tab = '\0';  // Replace the tab with a null terminator
        numbers[count++] = atoi(ptr);  // Convert to integer and store
        ptr = next_tab + 1;  // Move to the next token
    }
    numbers[count++] = atoi(ptr);  // Process the last token

    free(substring);  // Free memory
}



table_entry get_data(int sockfd, const char *key) {
    char buffer[BUFF_SIZE];  // Fixed-size array for buffer
    table_entry t_data;
    memset(&t_data, 0, sizeof(t_data));  

    // Format the command and send it
    snprintf(buffer, BUFF_SIZE, "get %s\r\n", key);
    write(sockfd, buffer, strlen(buffer));

    // Clear buffer and read response from socket
    memset(buffer, 0, BUFF_SIZE);
    if (read(sockfd, buffer, BUFF_SIZE) > 1) {
        strcpy(t_data.key, key);  // Copy key into consult.key
    }

    // Parse the response to fill consult.values
    parse_numbers(buffer, t_data.values);

    return t_data;
}

int get_max_val(int values[], int size) {
    int max = values[0];
    for (int i = 1; i < size; i++) {
        if (values[i] > max) max = values[i];
    }

    return max;
}

const char* get_category(int max) {
    static const char* categories[] = {
        "Sin datos",           // Index 0
        "Buena",               
        "Regular",             
        "Mala",                
        "Muy mala",            
        "Extremadamente mala"  
    };

    if (max == 0) return categories[0];
    
    int index = (max - 1) / 50 + 1; // Calculate the category index based on ranges
    if (index > 5) index = 5;       // Cap the index to the maximum category

    return categories[index];
}

// Function to generate the JSON response
void generate_json(table_entry entry, char *json_output, size_t output_size) {
    // Calculate max value
    int max = get_max_val(entry.values, 10);
    // Obtain the category
    const char *category = get_category(max);

    // Build the JSON with the obtained data
    snprintf(json_output, output_size,
             "{\n"
             "  \"dia\": {\n"
             "    \"fecha\": \"%s\",\n"
             "    \"categoria\": \"%s\",\n"
             "    \"max\": %d,\n"
             "    \"valores\": [\n"
             "      %d, %d, %d, %d, %d, %d, %d, %d, %d, %d\n"
             "    ]\n"
             "  }\n"
             "}",
             entry.key, 
             category,
              max,
             entry.values[0],
             entry.values[1], 
             entry.values[2], 
             entry.values[3], 
             entry.values[4],
             entry.values[5], 
             entry.values[6], 
             entry.values[7], 
             entry.values[8], 
             entry.values[9]);
}


char* memcached_delete(int sockfd, const char *key) {
    char buffer[BUFF_SIZE];

    // Prepare the delete command
    snprintf(buffer, BUFF_SIZE, "delete %s\r\n", key);

    // Send the delete command to memcached
    ssize_t bytes_written = write(sockfd, buffer, strlen(buffer));
    if (bytes_written <= 0) {
        return ERROR_G;
    }

    // Clear buffer and read response from memcached
    memset(buffer, '\0', BUFF_SIZE);
    ssize_t read_bytes = read(sockfd, buffer, BUFF_SIZE - 1);

    // Check the response from memcached
    if (read_bytes > 0) {
        buffer[read_bytes] = '\0';  //no es necesario
        if (strncmp(buffer, "DELETED", 7) == 0) {
            return "DELETED";
        } else if (strncmp(buffer, "NOT_FOUND", 9) == 0) {
            return "NOT_FOUND";
        } else {
            return ERROR_G;
        }
    } else {
        return ERROR_G;
    }
}

void send_error_response(FILE *outgoing, const char *message) {
    fprintf(outgoing, "HTTP/1.1 400 Bad Request\r\n");
    fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
    fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
    fprintf(outgoing, "Connection: close\r\n\r\n");
    fprintf(outgoing, "{\"error\": \"%s\"}\r\n", message);
    fflush(outgoing);
}

void handle_root_get(FILE *outgoing) {
    FILE *file = fopen(MEMCACHED_JSON_SCRIPT, "r");
    if (file == NULL) {
        send_error_response(outgoing, "Could not open JSON file");
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
    fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
    fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
    fprintf(outgoing, "Connection: close\r\n");
    fprintf(outgoing, "Content-Length: %lu\r\n\r\n", file_size);

    char buffer[BUFF_SIZE];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        fprintf(outgoing, "%s", buffer);
    }

    fflush(outgoing);
    fclose(file);
}

void handle_version_get(FILE *outgoing) {
    const char *jsonString = "{\n"
                             "\t\"status\": {\n"
                             "\t\t\"version\": \"0.0.1\",\n"
                             "\t\t\"semestre\": \"2025-1\",\n"
                             "\t\t\"equipo\": \"Equipo-ERLM-GZAL-HMJA-MVA\",\n"
                             "\t\t\"integrantes\": {\n"
                             "\t\t\t\"420003818\": \"Escobar Rosales Luis Mario\",\n"
                             "\t\t\t\"316127927\": \"Garcia Zarraga Angelica Lizbeth\",\n"
                             "\t\t\t\"315137903\": \"Hernandez Morales Jose Angel\",\n"
                             "\t\t\t\"317042766\": \"Maldonado Vazquez Alegandro\"\n"
                             "\t\t}\n"
                             "\t}\n"
                             "}";
    fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
    fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
    fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
    fprintf(outgoing, "Connection: close\r\n");
    fprintf(outgoing, "Content-Length: %lu\r\n\r\n", strlen(jsonString));
    fprintf(outgoing, "%s", jsonString);
    fflush(outgoing);
}

void handle_estado_get(FILE *outgoing, int sockfd) {
    char stats[512];
    char jsonString[BUFF_SIZE];

    handle_command(MEMCACHED_STATS_SCRIPT, stats, sizeof(stats));

    snprintf(jsonString, sizeof(jsonString),
             "{\n"
             "  \"backend\": {\n"
             "    \"socket\": \"%s\",\n"
             "    \"version\": \"%s\",\n"
             "    \"stats\": %s\n"
             "  }\n"
             "}\n",
             SOCKET_PATH, "version", stats);

    fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
    fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
    fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
    fprintf(outgoing, "Connection: close\r\n");
    fprintf(outgoing, "Content-Length: %lu\r\n\r\n", strlen(jsonString));
    fprintf(outgoing, "%s", jsonString);
    fflush(outgoing);
}


void handle_dia_post(FILE *outgoing, int sockfd, const char *key, char *payload) {
    table_entry entry;
    // Aquí deberías procesar el payload, por ejemplo, parsearlo en un formato adecuado.
    // En este caso, simplemente podemos asumir que el payload contiene los datos en formato JSON.

    // Suponiendo que payload tiene la clave y los valores, asignamos estos a entry:
    sscanf(payload, "{\"key\": \"%s\", \"values\": [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]}", 
           entry.key, 
           &entry.values[0], &entry.values[1], &entry.values[2], &entry.values[3], 
           &entry.values[4], &entry.values[5], &entry.values[6], &entry.values[7], 
           &entry.values[8], &entry.values[9]);
    
    // Aquí puedes agregar la lógica para guardar los datos en Memcached. Como ejemplo, usaremos save_data.
    int save_result = save_data(sockfd, &entry);
    
    if (save_result == 0) {
        fprintf(outgoing, "HTTP/1.1 201 Created\r\n");
        fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
        fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
        fprintf(outgoing, "Connection: close\r\n");
        fprintf(outgoing, "{\"message\": \"Data added successfully\"}\r\n");
    } else {
        send_error_response(outgoing, "Failed to add data");
    }
}
void handle_dia_put(FILE *outgoing, int sockfd, const char *key, char *payload) {
    table_entry entry;
    sscanf(payload, "{\"key\": \"%s\", \"values\": [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]}", 
           entry.key, 
           &entry.values[0], &entry.values[1], &entry.values[2], &entry.values[3], 
           &entry.values[4], &entry.values[5], &entry.values[6], &entry.values[7], 
           &entry.values[8], &entry.values[9]);
    
    // Usamos el comando "set" para modificar los datos en Memcached.
    int save_result = save_data(sockfd, &entry);
    
    if (save_result == 0) {
        fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
        fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
        fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
        fprintf(outgoing, "Connection: close\r\n");
        fprintf(outgoing, "{\"message\": \"Data modified successfully\"}\r\n");
    } else {
        send_error_response(outgoing, "Failed to modify data");
    }
}
void handle_dia_delete(FILE *outgoing, int sockfd, const char *key) {
    char *result = memcached_delete(sockfd, key);

    if (strcmp(result, "DELETED") == 0) {
        fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
        fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
        fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
        fprintf(outgoing, "Connection: close\r\n");
        fprintf(outgoing, "{\"message\": \"Data deleted successfully\"}\r\n");
    } else {
        send_error_response(outgoing, "Failed to delete data");
    }
}

void handle_dia_get(FILE *outgoing, int sockfd, const char *key) {
    // Obtener datos de Memcached usando el comando GET
    table_entry entry = get_data(sockfd, key);

    // Crear el JSON con los datos obtenidos
    char json_output[BUFF_SIZE];
    generate_json(entry, json_output, sizeof(json_output));

    // Enviar la respuesta HTTP con el JSON generado
    fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
    fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
    fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
    fprintf(outgoing, "Connection: close\r\n");
    fprintf(outgoing, "Content-Length: %lu\r\n\r\n", strlen(json_output));
    fprintf(outgoing, "%s", json_output);
    fflush(outgoing);
}

void handle_request(const char *endpoint, const char *method, int sockfd, FILE *outgoing, char *payload) {
    if (strcmp(endpoint, "/") == 0 && strcmp(method, "GET") == 0) {
        handle_root_get(outgoing);
    } else if (strcmp(endpoint, "/version") == 0 && strcmp(method, "GET") == 0) {
        handle_version_get(outgoing);
    } else if (strcmp(endpoint, "/estado") == 0 && strcmp(method, "GET") == 0) {
        handle_estado_get(outgoing, sockfd);
    } else if (strncmp(endpoint, "/dia", 4) == 0) {
        const char *key = endpoint + 5;  // Suponiendo que el formato es /dia/{key}

        if (strcmp(method, "GET") == 0) {
            handle_dia_get(outgoing, sockfd, key);  // Ver
        } else if (strcmp(method, "POST") == 0) {
            handle_dia_post(outgoing, sockfd, key, payload);  // Agregar
        } else if (strcmp(method, "PUT") == 0) {
            handle_dia_put(outgoing, sockfd, key, payload);  // Modificar
        } else if (strcmp(method, "DELETE") == 0) {
            handle_dia_delete(outgoing, sockfd, key);  // Borrar
        } else {
            send_error_response(outgoing, "Unknown method for /dia");
        }
    } else {
        send_error_response(outgoing, "Unknown endpoint or HTTP method");
    }
}

void cleanup_resources(FILE *incoming, FILE *outgoing, char *data) {
    if (data) free(data);
    if (outgoing) fclose(outgoing);
    if (incoming) fclose(incoming);
}

void daemon_child_function(FILE *incoming, FILE *outgoing, char *incoming_name) {
    int ignore;
    char message[BUFF_SIZE] = {0};
    char method[8] = {0};
    char endpoint[128] = {0};
    int content_length = 0;  // Stores the Content-Length value
    char line[BUFF_SIZE] = {0};
    char *data = NULL;

    // Connect to Memcached
    int sockfd = memcache_conection();
    if (sockfd == -1) {
        fprintf(outgoing, "Content-Type: text/plain\r\nError: Could not connect to Memcached\n");
        fflush(outgoing);
        fclose(outgoing);
        fclose(incoming);
        _exit(EXIT_FAILURE);
    }

    LOG(1, ("Receiving message\n"));

    // Read headers
    while (fgets(line, sizeof(line), incoming) != NULL) {
        // End of headers
        if (strcmp(line, "\r\n") == 0) {
            strcat(message, line);
            break;
        }

        // Check for Content-Length header
        if (strncmp(line, "Content-Length:", 15) == 0) {
            content_length = atoi(line + 15);
        }

        strncat(message, line, sizeof(message) - strlen(message) - 1);
    }

    // Read body if Content-Length > 0
    if (content_length > 0) {
        size_t bytes_to_read = content_length;
        while (bytes_to_read > 0) {
            size_t read_now = fread(line, 1, (bytes_to_read > sizeof(line) ? sizeof(line) : bytes_to_read), incoming);
            if (read_now <= 0) break;

            strncat(message, line, read_now);
            bytes_to_read -= read_now;
        }
    }

    // Ensure the message is null-terminated
    message[BUFF_SIZE - 1] = '\0';

    // Parse method and endpoint
    sscanf(message, "%7s %127s", method, endpoint);

    // Extract body
    char *body_start = strstr(message, "\r\n\r\n");
    if (body_start != NULL) {
        body_start += 4;
        if (content_length > 0) {
            data = strndup(body_start, content_length);
        }
    }

    LOG(1, ("Handling request\n"));
    handle_request(endpoint, method, sockfd, outgoing, data);

    cleanup_resources(incoming, outgoing, data);
    _exit(EXIT_SUCCESS);
}