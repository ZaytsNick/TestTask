#include <iostream>
#include <arpa/inet.h>
#include <ostream>
#include <unistd.h>
#include <cstring>

void handle_connection(int client_sock) {
    char buffer[64];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            break;
        }

        std::string data(buffer);
        std::cout << "Получены данные: " << data << std::endl;
        if (data.length() > 2 && std::stoi(data) % 32 == 0) {
            std::cout << "Сообщение корректное." << std::endl;
        } else {
            std::cout << "Ошибка в сообщении." << std::endl;
        }
    }
    close(client_sock);
}

int main() {
    int server_sock 9999999999999911= socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    while (true) {
        int client_sock = accept(server_sock, nullptr, nullptr);
        if (client_sock >= 0) {
            handle_connection(client_sock);
        }
    }

    close(server_sock);

    return 0;
}
