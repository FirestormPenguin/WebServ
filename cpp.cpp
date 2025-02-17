#include <iostream>
#include <vector>
#include <poll.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include <stdio.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket failed");
        return 1;
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        return 1;
    }

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen failed");
        return 1;
    }

    std::vector<struct pollfd> fds;
    struct pollfd server_pollfd;
    server_pollfd.fd = server_fd;
    server_pollfd.events = POLLIN;
    fds.push_back(server_pollfd);

    while (true) {
        int ret = poll(&fds[0], fds.size(), -1);
        if (ret < 0) {
            perror("poll failed");
            break;
        }

        for (size_t i = 0; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                if (fds[i].fd == server_fd) { // Nuova connessione
                    sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
                    if (client_fd < 0) {
                        perror("accept failed");
                        continue;
                    }
                    std::cout << "Nuovo client connesso!" << std::endl;
                    struct pollfd client_pollfd;
                    client_pollfd.fd = client_fd;
                    client_pollfd.events = POLLIN;
                    fds.push_back(client_pollfd);
                } else { // Il client ha inviato dati
                    char buffer[BUFFER_SIZE];
                    memset(buffer, 0, BUFFER_SIZE);
                    int bytes_read = recv(fds[i].fd, buffer, BUFFER_SIZE, 0);
                    if (bytes_read <= 0) { // Disconnessione
                        std::cout << "Client disconnesso" << std::endl;
                        close(fds[i].fd);
                        fds.erase(fds.begin() + i);
                        i--; // Evitiamo di saltare un elemento
                    } else {
                        std::cout << "Richiesta ricevuta:\n" << buffer << std::endl;
                        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
                        send(fds[i].fd, response.c_str(), response.size(), 0);
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
