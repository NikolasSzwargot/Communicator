#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cstdio>
#include <error.h>
#include <iostream>

int main(int argc, char** argv){
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (struct sockaddr *)& serverAddress, sizeof(serverAddress)))
    {
        error(1, errno, "Failed to connect");
    }

    char data[] = "Message to server";
    std::cout << "Message to be sent: " << data << std::endl;
    write(sock, data, sizeof(data));

    close(sock);
    return 0;
    
}