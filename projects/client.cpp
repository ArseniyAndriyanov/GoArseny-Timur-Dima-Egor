#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 512

int sock;
char close_msg[] = "end!";

void* client_handler(void* arg) {
    
   
    while (1) {
        int msg_size;
        char buffer[BUFFER_SIZE];
        read(sock, buffer, sizeof(buffer));

        if (msg_size < BUFFER_SIZE) {
            buffer[msg_size] = '\0';
        }
        printf("%s\n", buffer);

        if (strcmp(buffer, close_msg) == 0) {
            break;
        }
        memset(buffer, 0, sizeof(buffer));
    }

    return NULL;
}

int main() {
    struct sockaddr_in server_addr;
    pthread_t client_thread;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1111);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    printf("Connected!\n");

    // Create thread for client handling
    pthread_create(&client_thread, NULL, client_handler, NULL);

    
    while (1) {
        char msg[BUFFER_SIZE];
        fgets(msg, BUFFER_SIZE, stdin);
        int msg_size = strlen(msg)-1;

        send(sock, msg, msg_size, 0);
        usleep(10000);
    }

    return 0;
}
