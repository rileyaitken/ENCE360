#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "http.h"

#define BUF_SIZE 1024

Buffer* http_query(char *host, char *page, int port) {
	
	struct addrinfo* server_address = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET; // IPv4 connection
	hints.ai_socktype = SOCK_STREAM; // TCP connection
	char* port_s = malloc(20);
	sprintf(port_s, "%d", port);
	if (getaddrinfo(host, port_s, &hints, &server_address) != 0) { // Use 'hints', 'host' and 'port' to read server info into 'server_address')
		perror("Error getting the server address info.");
		return NULL;
	}
	
	free(port_s);
	
	// Create socket of compatible type to server
	int sock = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol); 
	
	// Connect to server over TCP
	if (connect(sock, server_address->ai_addr, server_address->ai_addrlen) == -1) {
		perror("Error connecting to host.");
		return NULL;
	}
	
	freeaddrinfo(server_address);
	
	Buffer* response = malloc(sizeof(Buffer));
	response->data = malloc(BUF_SIZE);
	memset(response->data, '\0', BUF_SIZE);
	response->length = 0;

	char* request = malloc(BUF_SIZE);
	snprintf(request, BUF_SIZE, "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: getter\r\n\r\n", page, host);
	
	if (write(sock, request, strlen(request)) == -1) {
		perror("Error making request to server.");
		return NULL;
	}
	
	free(request);
	
	char* buffer = malloc(BUF_SIZE);
	int num_bytes = read(sock, buffer, BUF_SIZE);
	if (num_bytes == -1) {
		perror("Error reading from server.");
		return NULL;
	}
	
	while (num_bytes > 0) {
		response->length += num_bytes;
		if (response->length > BUF_SIZE) {
			response->data = realloc(response->data, response->length);
		}
		memcpy(response->data + response->length - num_bytes, buffer, num_bytes);
		num_bytes = read(sock, buffer, BUF_SIZE);
	}
	
	free(buffer);
	close(sock);
	return response;
}

// split http content from the response string
char* http_get_content(Buffer *response) {

    char* header_end = strstr(response->data, "\r\n\r\n");

    if (header_end) {
        return header_end + 4;
    }
    else {
        return response->data;
    }
}


Buffer *http_url(const char *url) {
    char host[BUF_SIZE];
    strncpy(host, url, BUF_SIZE);

    char *page = strstr(host, "/");
    if (page) {
        page[0] = '\0';

        ++page;
        return http_query(host, page, 80);
    }
    else {

        fprintf(stderr, "could not split url into host/page %s\n", url);
        return NULL;
    }
}

