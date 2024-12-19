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

#define PORT 8080
#define BUFF_SIZE 1024
#define SOCKET_P "/var/run/memcached/memcached.sock"

char primera[1000];
char metodo[8];
char endpoint[100];
char protocolo[16];
void version_request(FILE *outgoing);
void not_found(FILE *outgoing);
void not_allowed(FILE *outgoing);

void metodoGET(FILE *outgoing, char *endpoint);

//TODO: Conexion Memcached , arreglar post y delete ,dividir en funcion a get

void daemon_child_function(FILE *incoming, FILE *outgoing, char *incoming_name)
{
    fscanf(incoming, "%[^\n]", primera);
    if (primera == NULL) {
        LOG(1, ("Error%s\n", incoming_name));
        _exit(EXIT_FAILURE);
    }
    sscanf(primera, "%s %s %s", metodo, endpoint, protocolo);
    printf("%s", "-------------------------------");
    printf("%s%s", "\nMetodo: ",metodo);
    printf("%s%s", "\nEndpoint: ",endpoint);
    printf("%s%s", "\nProtocolo: ",protocolo );
    printf("%s", "\n-------------------------------\n");

    if(strcmp(metodo, "GET") == 0) {
        metodoGET(outgoing,endpoint);
    }

    LOG(9, ("Daemon child ejecutado con exito\n", incoming_name));
    _exit(EXIT_SUCCESS);

}

void version_request(FILE *outgoing) {
    // Variables para los datos
    const char *version = "0.0.1";
    const char *semestre = "2025-1";
    const char *equipo = "Equipo-ERLM-GZAL-HMJA-MVA";

    // Datos de los integrantes
    //  Diseño "modular"  por si eventualmente se necesitara agregar informacion
    const char *integrantes[][2] = {
        {"420003818", "Escobar Rosales Luis Mario"},
        {"316127927", "Garcia Zarraga Angelica Lizbeth"},
        {"315137903", "Hernandez Morales Jose Angel"},
        {"317042766", "Maldonado Vazquez Alejandro"}
    };

    // Construir el JSON en un buffer
    char contenido[1024];
    int length = snprintf(contenido, sizeof(contenido),
                          "{\n"
                          "  \"status\": {\n"
                          "    \"version\": \"%s\",\n"
                          "    \"semestre\": \"%s\",\n"
                          "    \"equipo\": \"%s\",\n"
                          "    \"integrantes\": {\n"
                          "      \"%s\": \"%s\",\n"
                          "      \"%s\": \"%s\",\n"
                          "      \"%s\": \"%s\",\n"
                          "      \"%s\": \"%s\"\n"
                          "    }\n"
                          "  }\n"
                          "}",
                          version, semestre, equipo,
                          integrantes[0][0], integrantes[0][1],
                          integrantes[1][0], integrantes[1][1],
                          integrantes[2][0], integrantes[2][1],
                          integrantes[3][0], integrantes[3][1]
                         );

    // Encabezado HTTP
    fprintf(outgoing,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json; charset=UTF-8\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Connection: close\r\n"
            "Content-Length: %d\r\n"
            "\r\n",
            length);

    // Enviar el contenido JSON
    fputs(contenido, outgoing);
}

void not_found(FILE *outgoing) {
    fprintf(outgoing, "HTTP/1.1 404 Not Found\r\n");
    fprintf(outgoing, "Content-Type: text/plain; charset=UTF-8\r\n");
    fprintf(outgoing, "\r\n");
    fprintf(outgoing, "404 Not Found\r\n");
}

void not_allowed(FILE *outgoing) {
    fprintf(outgoing, "HTTP/1.1 405 Method Not Allowed\r\n");
    fprintf(outgoing, "Content-Type: text/plain; charset=UTF-8\r\n");
    fprintf(outgoing, "\r\n");
    fprintf(outgoing, "405 Method Not Allowed\r\n");

}

void metodoGET(FILE *outgoing, char *endpoint) {

    if(strcmp(endpoint, "/") == 0) {
        FILE *json = fopen("api.json", "r");
        printf("%s", "raiz");
        if(json == NULL) {
            printf("%s", "no se encontro el archivo api.json\n");
        } else {
            char contenidos[55];
            int tamano = 0;
            char c;

            for (c = getc(outgoing); c != EOF; c = getc(outgoing))
                tamano = tamano + 1;

            fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
            fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
            fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
            fprintf(outgoing, "Connection: close\r\n");
            fprintf(outgoing, "Content-Length: %d\r\n", tamano);

            while(fgets(contenidos, sizeof(contenidos), json)) {
                fprintf(outgoing, "%s", contenidos);
            }
            fclose(json);
        }

    }
    if(strcmp(endpoint, "/version") == 0) {
        printf("%s", "version");
        FILE *json = fopen("api.json", "r");

        if(json == NULL) {
            printf("%s", "no se encontro el archivo api.json\n");
        } else {
            char contenidos[55];
            int tamano = 0;
            char c;

            for (c = getc(outgoing); c != EOF; c = getc(outgoing))
                tamano = tamano + 1;

            fprintf(outgoing, "HTTP/1.1 200 OK\r\n");
            fprintf(outgoing, "Content-Type: application/json; charset=UTF-8\r\n");
            fprintf(outgoing, "Access-Control-Allow-Origin: *\r\n");
            fprintf(outgoing, "Connection: close\r\n");
            fprintf(outgoing, "Content-Length: %d\r\n", tamano);

            char *version = "{"
                            "status: {"
                            "version: 0.0.1,"
                            "semestre: 2025-1,"
                            "equipo: Equipo-ERLM-GZAL-HMJA-MVA,"
                            "integrantes: {"
                            "420003818 - Escobar Rosales Luis Mario,"
                            "316127927 - Garcia Zarraga Angelica Lizbeth,"
                            "315137903 - Hernandez Morales Jose Angel,"
                            "317042766 - Maldonado Vazquez Alegandro"
                            "}"
                            "}"
                            "}";
            fprintf(outgoing, "%s", version);
            fclose(json);
        }
    }
    if(strcmp(endpoint, "/estado") == 0) {

        printf("%s", "estado");
        //char version_output[128] = {0};
        char *socket_path = "memcached.sock";
        int server_socket;

        server_socket = socket(AF_UNIX, SOCK_STREAM, 0);
        struct sockaddr_un server_addr;
        server_addr.sun_family = AF_UNIX;
        strcpy(server_addr.sun_path, "unix_socket");
        int slen = sizeof(server_addr);
        bind(server_socket, (struct sockaddr *) &server_addr, slen);
        connect(server_socket, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_un));

        char *versionMem;
        send(server_socket, versionMem, strlen(versionMem), 0);
        char respuesta[20] = {0};
        int bytes = recv(server_socket, respuesta, sizeof(respuesta) - 1, 0);
        respuesta[bytes] = '\0';
        fprintf(outgoing, "%s", respuesta);
    }
    return;
}

int main() {
    return 0 ;
}
