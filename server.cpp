#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main() {
    // Tworzenie gniazda serwera
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Błąd podczas tworzenia gniazda." << std::endl;
        return -1;
    }

    // Struktura opisująca adres serwera
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Port serwera
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Akceptuj połączenia od dowolnego adresu

    // Przypisanie adresu do gniazda serwera
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Błąd podczas przypisywania adresu do gniazda." << std::endl;
        close(serverSocket);
        return -1;
    }

    // Nasłuchiwanie na gnieździe serwera
    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Błąd podczas nasłuchiwania na gnieździe." << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Serwer nasłuchuje na porcie 12345..." << std::endl;

    // Akceptowanie połączeń od klientów
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket == -1) {
        std::cerr << "Błąd podczas akceptowania połączenia." << std::endl;
        close(serverSocket);
        return -1;
    }

    // Odbieranie wiadomości od klienta
    char buffer[1024];
    ssize_t bytesRead;
    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)))
    {
        //bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        buffer[bytesRead] = '\0';
        std::cout << "Otrzymana wiadomość od klienta: " << buffer << std::endl;
    }

    // Zamykanie gniazda klienta i serwera
    close(clientSocket);
    close(serverSocket);

    return 0;
}