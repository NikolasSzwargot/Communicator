#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

int main() {
    // Tworzenie gniazda klienta
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        std::cerr << "Błąd podczas tworzenia gniazda." << std::endl;
        return -1;
    }

    // Struktura opisująca adres serwera
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345); // Port serwera
    inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr); // Adres IP serwera

    // Nawiązywanie połączenia z serwerem
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Błąd podczas nawiązywania połączenia z serwerem." << std::endl;
        close(clientSocket);
        return -1;
    }

    // Wysyłanie wiadomości do serwera
    std::string message;
    while (message != "end") {
        std::cout << "Wprowadz wiadomosc: ";
        std::getline(std::cin, message);
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    // Zamykanie gniazda klienta
    close(clientSocket);

    return 0;
}