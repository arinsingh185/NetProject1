#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>



static int create_server_socket(int port);
static int accept_connection(int server_socket, struct sockaddr_in *client_addr);
static int receive_client_msg(int client_socket, char* buffer, size_t buffer_size);
static int send_server_response(int client_socket, const char* response);
static void clean_client(int server_socket, int client_socket);
static void usage(const char *program);

//contains the creation of a server socket and accepts console commands for server port number and also the main server loop
int main(int argc, char *argv[]) {

    if (argc != 2) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    char *ptr;
    long port_long = strtol(argv[1], &ptr, 10);
    if (*ptr != '\0' || port_long < 0 || port_long > 65535) {
        fprintf(stderr, "Usage: %s [port]\n", argv[1]);
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    int port = (int)port_long;
    int server_socket = create_server_socket(port);

    if (server_socket < 0) {
        return EXIT_FAILURE;
    }

    struct sockaddr_in client_addr;
    int client_socket = accept_connection(server_socket, &client_addr);
    if (client_socket < 0) {
        return EXIT_FAILURE;
    }

    char recv_buffer[1024];
    if (receive_client_msg(client_socket, recv_buffer, sizeof(recv_buffer)) < 0) {
        clean_client(server_socket, client_socket);
        return EXIT_FAILURE;
    }

    if (send_server_response(server_socket, recv_buffer) < 0) {
        clean_client(server_socket, client_socket);
        return EXIT_FAILURE;
    }

    clean_client(server_socket, client_socket);
    return EXIT_SUCCESS;
}

static void usage(const char* program) {
    fprintf(stderr, "Usage: %s [port]\n", program);
}

//creation of server socket
static int create_server_socket(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) != 0){
        perror("bind failed");
        close(server_socket);
        return -1;
    }
    else {
        printf("Socket successfully binded");
    }

    if (listen(server_socket, 1) < 1) {
        perror("Listen failed");
        close(server_socket);
       return -1;
    }

    printf("Server listening on port %d\n", port);
    return server_socket;
}

//accepts client connection will throw error if the socket fails to accept
static int accept_connection(int server_socket, struct sockaddr_in *client_addr) {
    socklen_t addr_len = sizeof(*client_addr);

    int client_socket = accept(server_socket, (struct sockaddr *)client_addr, &addr_len);
    if (client_socket < 0) {
        perror("Accept failed");
        return -1;
    }

    char client_msg[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr->sin_addr, client_msg, sizeof(client_msg));
    printf("Connection from %s\n", client_msg, ntohs(client_addr->sin_port));
}

//read what the client sends and respond with a default msg
static int receive_client_msg(int client_socket, char* buffer, size_t buffer_size) {
    ssize_t bytes = recv(client_socket, buffer, buffer_size - 1, 0);

    if (bytes < 0) {
        perror("recv failed");
        return -1;
    }

    if (bytes == 0) {
        fprintf(stderr, "Connection closed");
        return -1;
    }

    buffer[bytes] = '\0';
    printf("Bytes recieved from cient: %zd\n", bytes);
    printf("Message: %s\n", buffer);

    return 0;
}

static int send_server_response(int client_socket, const char* response) {
    size_t response_length = strlen(response);
    ssize_t bytes = send(client_socket, response, response_length, 0);

    if (bytes < 0) {
        perror("send failed");
        return -1;
    }

    if ((size_t)bytes != response_length) {
        fprintf(stderr, " only sent %zd of %zu bytes of the response\n", bytes, response_length);
    }

    printf("Sent response: %s\n", response);
    return 0;
}

static void clean_client(int server_socket, int client_socket) {
    if (client_socket >= 0) {
        if (close(client_socket) < 0) {
            perror("close failed");
        }
    }
    if (server_socket >= 0) {
        if (close(server_socket) < 0) {
            perror("close failed");
        }
    }
}