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



table_entry get_data(int sockfd, char *key) {
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











int main() {
    return 0 ;
}
