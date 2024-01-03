#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
using json = nlohmann::json;

// TODO: Implement better sending response function. This one SUCKS!
//  const std::string JSONTYPE = "application/json";
//  const std::string PLAINTYPE = "text/plain";

const std::string USERMESSAGES = "usersData/messages.json";
json userMessages;

void handleOptions(int clientSocket)
{
    const char *response = "HTTP/1.1 200 OK\n"
                           "Access-Control-Allow-Origin: *\n"
                           "Access-Control-Allow-Methods: POST, GET, OPTIONS\n"
                           "Access-Control-Allow-Headers: Content-Type\n"
                           "Content-Length: 0\n\n";

    send(clientSocket, response, strlen(response), 0);
}

void sendResponse(int clientSocket, const std::string &response)
{
    std::string corsHeader = "HTTP/1.1 200 OK\n"
                             "Access-Control-Allow-Origin: *\n"
                             "Content-Length: " +
                             std::to_string(response.size()) + "\n"
                                                               "Content-Type: text/plain\n\n";

    std::string fullResponse = corsHeader + response;
    send(clientSocket, fullResponse.c_str(), fullResponse.size(), 0);
}

void sendFriendsList(int clientSocket, const std::string &friendsList)
{
    std::string corsHeader = "HTTP/1.1 200 OK\n"
                             "Access-Control-Allow-Origin: *\n"
                             "Content-Length: " +
                             std::to_string(friendsList.size()) + "\n"
                                                                  "Content-Type: application/json\n\n";

    std::string fullResponse = corsHeader + friendsList;
    send(clientSocket, fullResponse.c_str(), fullResponse.size(), 0);
}

json parseData(char buffer[1024])
{
    std::string str(buffer);
    std::size_t found = str.find("{");

    if (found != std::string::npos)
    {
        std::string jsonData = str.substr(found);

        try
        {
            json receivedData = json::parse(jsonData);
            return receivedData;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error parsing JSON: " << e.what() << std::endl;
        }
    }

    return nullptr;
}

void extractParams(const std::string& url, std::string& sender, std::string& recipient) {
    std::istringstream iss(url);
    std::string token;

    while (std::getline(iss, token, ' ')) {
        size_t paramPos = token.find('?');
        if (paramPos != std::string::npos) {
            std::istringstream params(token.substr(paramPos + 1));
            std::string param;

            while (std::getline(params, param, '&')) {
                size_t equalPos = param.find('=');
                if (equalPos != std::string::npos) {
                    std::string key = param.substr(0, equalPos);
                    std::string value = param.substr(equalPos + 1);

                    if (key == "sender") {
                        sender = value;
                    } else if (key == "recipient") {
                        recipient = value;
                    }
                }
            }
        }
    }
}

bool handleLogin(json loginData, json serverUsers, std::string &friendsList)
{
    std::string username = loginData["username"];
    std::string password = loginData["password"];
    bool successfullLogin = false;

    for (const auto &user : serverUsers["usersInfo"])
    {
        if (user["username"] == username && user["password"] == password)
        {
            successfullLogin = true;
            friendsList = user["friends"].dump();
            break;
        }
    }

    return successfullLogin;
}

void addMessageToJson(char buffer[1024], const std::string filename)
{
    json message = parseData(buffer);

    std::ifstream inputFile(filename);
    json userMessages;
    if (inputFile.is_open())
    {
        inputFile >> userMessages;
    }
    else
    {
        userMessages["userMessages"] = json::array();
    }

    userMessages["userMessages"].push_back(message);
    inputFile.close();

    std::ofstream outputFile(filename, std::ios::out);
    if (outputFile.is_open())
    {
        outputFile << std::setw(4) << userMessages << std::endl;
        outputFile.close();
        std::cout << "Dodano wiadomosc!\n";
    }
    else
    {
        std::cout << "Nie mozna otworzyc pliku!\n";
    }
}

void sendChatHistory(int clientSocket, const std::string &sender, const std::string &recipient, const std::string &filename)
{
    std::ifstream inputFile(filename);
    json userMessages;
    if (inputFile.is_open())
    {
        inputFile >> userMessages;
        inputFile.close();

        // Filtruj wiadomości dla danego czatu (dla określonego nadawcy i odbiorcy)
        json chatHistory = json::array();
        for (const auto &message : userMessages["userMessages"])
        {
            if (message["sender"] == sender && message["recipient"] == recipient)
            {
                chatHistory.push_back(message);
            }
            else if (message["sender"] == recipient && message["recipient"] == sender)
            {
                chatHistory.push_back(message);
            }
        }

        // Konwertuj historię czatu na string JSON i wyślij jako odpowiedź
        std::string response = chatHistory.dump();
        sendFriendsList(clientSocket, response); // Zakładając, że funkcja sendFriendsList wysyła odpowiedź na wiadomość.
    }
    else
    {
        // Obsłuż przypadki błędów odczytu pliku
        std::string errorMessage = "Error reading chat history file!";
        sendResponse(clientSocket, errorMessage); // Zakładając, że funkcja sendResponse wysyła odpowiedź na wiadomość.
    }
}

void *handleClient(void *arg)
{
    int clientSocket = *((int *)arg);
    char buffer[1024];
    ssize_t bytesRead;
    std::ifstream file("usersData/usersInfo.json");
    json data = json::parse(file);
    std::string userFriends;
    std::cout << "Klient polaczony i gotowy do pracy!\n";

    bool loggedIn = false;

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[bytesRead] = '\0';
        json loginData = parseData(buffer);
        std::string loginResponse;

        if (strncmp(buffer, "OPTIONS", 7) == 0)
        {
            handleOptions(clientSocket);
            std::cout << "Opcje klienta ustawione.\n";
        }
        else if (!loggedIn && handleLogin(loginData, data, userFriends))
        {
            loginResponse = "login success";
            std::cout << "Proba logowania udana!\n";
            std::cout << "Znajomi:\n"
                      << userFriends << std::endl;
            sendResponse(clientSocket, loginResponse);
            loggedIn = true;
        }
        else if (loggedIn && strncmp(buffer, "GET /friends", 12) == 0)
        {
            std::cout << "Wysylanie listy znajomych\n";
            sendFriendsList(clientSocket, userFriends);
        }
        else if (loggedIn && strncmp(buffer, "POST /send-message", 18) == 0)
        {
            handleOptions(clientSocket);
            addMessageToJson(buffer, USERMESSAGES);
        }
        else if (loggedIn && strncmp(buffer, "GET /chat-history", 17) == 0)
        {
            std::string sender, receipent;
            extractParams(buffer, sender, receipent);
            sendChatHistory(clientSocket, sender, receipent, USERMESSAGES);
        }
        else if (loggedIn)
        {
            std::cout << "Message from loggedIn user: " << buffer << std::endl;
            // Obsługa komunikacji zalogowanego użytkownika
        }
        else
        {
            loginResponse = "login failure";
            std::cout << "Proba logowania odrzucona!\n";
            sendResponse(clientSocket, loginResponse);
        }
    }
    // GET /chat-history

    if (bytesRead == 0)
    {
        std::cout << "Klient się rozłączył." << std::endl;
    }
    else if (bytesRead == -1)
    {
        std::cerr << "Błąd podczas odbierania danych." << std::endl;
    }

    close(clientSocket);
    pthread_exit(NULL);
}

int main()
{

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Błąd podczas tworzenia gniazda." << std::endl;
        return -1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(12345);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddress.sin_addr) <= 0)
    {
        std::cerr << "Nieprawidlowy adres IP!" << std::endl;
        close(serverSocket);
        return -1;
    }

    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        std::cerr << "Błąd podczas przypisywania adresu do gniazda." << std::endl;
        close(serverSocket);
        return -1;
    }

    if (listen(serverSocket, 5) == -1)
    {
        std::cerr << "Błąd podczas nasłuchiwania na gnieździe." << std::endl;
        close(serverSocket);
        return -1;
    }

    std::cout << "Serwer nasłuchuje na porcie 12345..." << std::endl;

    while (true)
    {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1)
        {
            std::cerr << "Błąd podczas akceptowania połączenia." << std::endl;
            close(serverSocket);
            return -1;
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handleClient, (void *)&clientSocket) != 0)
        {
            std::cerr << "Błąd podczas tworzenia wątku." << std::endl;
            close(clientSocket);
        }
    }

    close(serverSocket);
    return 0;
}