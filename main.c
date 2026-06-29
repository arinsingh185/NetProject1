#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>

struct sockaddr;
struct sockaddr_in serveaddr;
int server_socket, client_socket;
socklen_t serve_addrlen = sizeof(serveaddr);
char server_msg[] = "Client connections successful";
char default_msg[] = "Nice msg";
void create_server_socket();
void bind_server_socket();
void listen_server_socket();
void accept_connection();
ssize_t client_request();
void server_response();

//contains the creation of a server socket and accepts console commands for server port number and also the main server loop
int main(int argc, char *argv[]) {
    create_server_socket();
    if (argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
    }

    int port = atoi(argv[1]);
        if (port < 0 || port > 65535) {
            printf("Invalid port number\n");
            return 1;
        }
    serveaddr.sin_family = AF_INET;
    serveaddr.sin_port = htonl(port);
    serveaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    //main server loop
    while (1) {
        accept_connection();
        if (client_request() > 0) {
            server_response();
        }
        else if (client_request() == 0) {
            break;
        }
        else {
            close(client_socket);
            exit(EXIT_FAILURE);
        }
    }
    close(server_socket);
    exit(EXIT_SUCCESS);
    return 0;
}

//creation of server socket
void create_server_socket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
}
//binding of server socket with error detection
void bind_server_socket() {
    if (bind(server_socket, (struct sockaddr *)&serveaddr, sizeof(serveaddr)) != 0) {
        printf("socket failed to bind\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    else {
        printf("Socket successfully binded");
    }
}

//sets the socket to listen in on any upcoming connections will throw error if socket fails to do so
void listen_server_socket() {
    if (listen(server_socket, 1) < 1) {
        printf("listen failed\n");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
}
//accepts client connection will throw error if the socket fails to accept
void accept_connection() {
    if (client_socket = accept(server_socket, (struct sockaddr *)&serveaddr, &serve_addrlen) < 0) {
        printf("Socket failed to accept client connection");
        close(client_socket);
        exit(EXIT_FAILURE);
    }
    else {
        send(client_socket,server_msg,strlen(server_msg),0);
    }
}

//read what the client sends and respond with a default msg
ssize_t client_request() {
        return recv(client_socket,server_msg,sizeof(server_msg),0);
}

void server_response() {
    send(client_socket, default_msg, strlen(default_msg), 0);
}