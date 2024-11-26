#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h> 
#include <fstream> 
#include <iostream> 
#include <string> 

#define MAX_CLIENTS 100
#define BUFFER_SIZE 512

int clients[MAX_CLIENTS];
int client_count = 0;

 std::ofstream historyFile("message_history.txt", std::ios::app); // Открываем файл в режиме добавления 
void* client_handler(void* arg) {
       
    
    int client_fd = *(int*)arg;
    

    while (1) {
        
        char buffer[BUFFER_SIZE];
        // Создание файла для записи истории сообщений 
       
        
        int bytes_received = read(client_fd, buffer, BUFFER_SIZE);
        if (bytes_received <= 0) {
            break;
        }
        buffer[bytes_received] = '\0';
        historyFile << "Client " << client_fd << ": " << buffer << std::endl; 


        buffer[bytes_received] = '\0';
        printf("Received message from client %d: %s\n", client_fd, buffer);

        std::string prefix = "Client " + std::to_string(client_fd) + ": ";
        const char* char_prefix = prefix.c_str();
        int prefix_length = strlen(char_prefix); 
        
        memmove(buffer + prefix_length, buffer, bytes_received); // Сдвигаем данные в массиве на длину префикса
        memcpy(buffer, char_prefix, prefix_length); 


        if (strcmp(buffer, "close") == 0) {
            const char* message = "end!";
            send(client_fd, message, strlen(message), 0);
            break;
        }

        // Отражаем обратно сообщение клиенту
        send(client_fd, buffer, bytes_received + prefix_length, 0);

        // Отправляем сообщение другим клиентам
        for (int i = 0; i < client_count; i++) {
            if (clients[i] != client_fd) {
                send(clients[i], buffer, bytes_received + prefix_length, 0);
            }
        }
        memset(buffer, 0, sizeof(buffer));
    }

    close(client_fd);
    return NULL;
}

int main() {
    int server_fd, new_client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    pthread_t client_thread;

    // Создание файла для записи истории сообщений 
    std::ofstream historyFile("message_history.txt", std::ios::app); // Открываем файл в режиме добавления 
    if (!historyFile) { 
        std::cerr << "Error opening history file" << std::endl; 
        return -1; 
    } 
 

    // Создаем серверный сокет
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    // Устанавливаем адрес сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(1111);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    // Привязываем серверный сокет
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Слушаем входящие соединения
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(1);
    }

    printf("Сервер слушает на порту 1111...\n");

    while (1) {
        // Принимаем новое соединение клиента
        new_client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (new_client_fd < 0) {
            perror("accept");
            continue;
        }

        printf("Новый клиент подключился: %d\n", new_client_fd);


        // Отправляем приветственное сообщение клиенту
        std::string efef;
        
       if (historyFile.is_open()) {
        std::ifstream historyReadFile("message_history.txt"); // Открываем файл для чтения
        if (historyReadFile.is_open()) {
            std::string line;
            while (std::getline(historyReadFile, line)) {
                if (line.size() == 0){
                    break;
                }
                line = line + "\n";
                send(new_client_fd, line.c_str(), line.size(), 0); // Отправляем строку клиенту
            }
            historyReadFile.close();
        } else {
            std::cerr << "Unable to open file for reading" << std::endl;
        }
       }

        const char* welcome_message = "Hello, client!\0";
        send(new_client_fd, welcome_message, strlen(welcome_message), 0);

        // Добавляем клиента в список
        clients[client_count] = new_client_fd;
        client_count++;

        // Создаем новый поток для обработки клиента
        pthread_create(&client_thread, NULL, client_handler, &new_client_fd);
    }

    return 0;
}
