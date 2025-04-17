// main.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#define BUFFER_SIZE 4096

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

double now_in_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)(ts.tv_sec * 1000.0 + ts.tv_nsec / 1.0e6);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *hostname = argv[1];
    const int port = 80;

    // Resolve hostname
    struct hostent *server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr, "Error: No such host: %s\n", hostname);
        return EXIT_FAILURE;
    }

    // Create socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error_exit("socket");

    // Set up the server address struct
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    // Start timer
    double start_time = now_in_ms();

    // Connect
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error_exit("connect");

    // Build HTTP GET request
    char request[1024];
    snprintf(request, sizeof(request),
             "GET / HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n"
             "\r\n", hostname);

    // Send request
    ssize_t sent = send(sockfd, request, strlen(request), 0);
    if (sent < 0) error_exit("send");

    // Receive response
    char buffer[BUFFER_SIZE];
    ssize_t received;
    while ((received = recv(sockfd, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[received] = '\0';
        printf("%s", buffer);
    }

    // End timer
    double end_time = now_in_ms();
    printf("\n\nElapsed Time: %.2f ms\n", end_time - start_time);

    // Cleanup
    close(sockfd);
    return 0;
}
