#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cstdlib>
#include <cstdio>
#include <error.h>
#include <iostream>

const int ONE = 1;

int main(int argc, char** argv){
    sockaddr_in localAdress {
        .sin_family = AF_INET,
        .sin_port = htons(atoi(argv[1])),
        .sin_addr = {htonl(INADDR_ANY)}
    };

    int servSock = socket(PF_INET, SOCK_STREAM, 0);
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &ONE, sizeof(ONE));

    if (bind(servSock, (sockaddr *) &localAdress, sizeof(localAdress))){
        error(1, errno, "Could not bind!");
    }

    std::cout << "Listening!\n";
    listen(servSock, 1);

    int ackSock = accept(servSock, NULL, NULL);
    std::cout << "Connection accepted\n";
    char data[255]{};

    int len = read(ackSock, data, sizeof(data) - 1);
    std::cout << "Received: " << data << std::endl;

    close(servSock);

    return 0;
}