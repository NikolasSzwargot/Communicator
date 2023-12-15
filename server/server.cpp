#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <nlohmann/json.hpp>
#include <fstream>
using json = nlohmann::json;

void *handleClient(void *arg) {
    int clientSocket = *((int *)arg);
    char buffer[1024];
    ssize_t bytesRead;
    std::ifstream file("usersData/usersInfo.json");
    json data = json::parse(file);

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesRead] = '\0';
        std::cout << "Otrzymana wiadomość od klienta: " << buffer << std::endl;
    }

    if (bytesRead == 0) {
        std::cout << "Klient się rozłączył." << std::endl;
    } else if (bytesRead == -1) {
        std::cerr << "Błąd podczas odbierania danych." << std::endl;
    }

    close(clientSocket);
    pthread_exit(NULL);
}

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Błąd podczas tworzenia gniazda." << std::endl;
        return -1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0) {
        std::cerr << "Nieprawidlowy adres IP!" << std::endl;
        close(serverSocket);
        return -1;

    }

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Błąd podczas przypisywania adresu do gniazda." << std::endl;
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Błąd podczas nasłuchiwania na gnieździe." << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Serwer nasłuchuje na porcie 12345..." << std::endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Błąd podczas akceptowania połączenia." << std::endl;
            close(serverSocket);
            return -1;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handleClient, (void *)&clientSocket) != 0) {
            std::cerr << "Błąd podczas tworzenia wątku." << std::endl;
            close(clientSocket);
        }
    }

    close(serverSocket);
    return 0;
}