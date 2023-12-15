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

    // while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)))
    // {
    //     buffer[bytesRead] = '\0';

    //     //Parsing login data from client
    //     json loginData = json::parse(buffer);
    //     std::string username = loginData["username"];
    //     bool successfullLogin = false;

    //     for (const auto &user : data["usersInfo"]) {
    //         if (user["username"] == username) {
    //             successfullLogin = true;
    //             break;
    //         }
    //     }

    //     std::string loginResponse;
    //     if (successfullLogin)
    //     {
    //         loginResponse = "login success";
    //         send(clientSocket, loginResponse.c_str(), loginResponse.size(), 0);
    //         break;
    //     }
    //     else {
    //         loginResponse = "login failure";
    //         send(clientSocket, loginResponse.c_str(), loginResponse.size(), 0);
    //     }
        
    // }
    

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

/*
Otrzymana wiadomość od klienta: GET /socket.io/?EIO=4&transport=polling&t=Onkj8T9 HTTP/1.1
Host: localhost:12345
User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/109.0
Accept: *//*
Accept-Language: pl,en-US;q=0.7,en;q=0.3
Accept-Encoding: gzip, deflate, br
Origin: null
Connection: keep-alive
Sec-Fetch-Dest: empty
Sec-Fetch-Mode: cors
Sec-Fetch-Site: cross-site
*/