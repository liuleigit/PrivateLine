#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <iostream>
using namespace std;

#define SERVER_PORT    8888
#define LISTEN_QUEUE   10 
#define BUFFER_SIZE    2048
 
int main(int argc, char* argv[])
{
    struct sockaddr_in svrAddr;
    memset(&svrAddr, 0, sizeof(svrAddr));
    svrAddr.sin_family = AF_INET;
    svrAddr.sin_addr.s_addr = htons(INADDR_ANY);
    svrAddr.sin_port = htons(SERVER_PORT);
 
    int svrSock = socket(PF_INET, SOCK_STREAM, 0);
    if (svrSock < 0)
    {
        cout << "Create socket error!\n";
        return -1;
    }

    int opt =1;
    setsockopt(svrSock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(svrSock, (struct sockaddr*)&svrAddr, sizeof(svrAddr)))
    {
        cout << "Bind server error!\n";
        return -1;
    }

    if (listen(svrSock, LISTEN_QUEUE))
    {
        cout << "Server listen error!\n"; 
        return -1;
    }

    int errNo = 0;
    int maxFd = 0;
    fd_set readFdset;
    struct timeval timeOut;
    struct sockaddr_in clientAddr;
    while (true)
    {
        socklen_t length = sizeof(clientAddr);
        timeOut.tv_sec = 3;
        timeOut.tv_usec = 0;
        FD_ZERO(&readFdset);
        FD_SET(svrSock, &readFdset);
        maxFd = svrSock + 1;
        switch(select(maxFd, &readFdset, NULL, NULL, &timeOut))
        {
        case -1:
            // error occurred
            continue;
        case 0:
            // time out, nothing to do
            continue;
        default:
            // someone is connected
            int clientSock = -1;
            try
            {
                if (!FD_ISSET(svrSock, &readFdset))
                    continue;

                int clientSock = accept(svrSock, (struct sockaddr*)&clientAddr, &length);
                if (clientSock < 0)
                {
                    cout << "Server accept error!\n";
                    throw errNo;
                }

                char buffer[BUFFER_SIZE];
                memset(buffer, 0, BUFFER_SIZE);
                length = recv(clientSock, buffer, BUFFER_SIZE, 0);
                if (length == 0)
                {
                    cout << "Server recieves nothing!\n";
                    throw errNo;
                }

                cout << length << endl;
                close(clientSock);
            }
            catch (int& errNo)
            {
                close(clientSock);
                continue;
            }
        }
    }

    close(svrSock);
    return 0;
}
