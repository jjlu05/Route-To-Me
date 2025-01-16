#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BACKLOG 5
#define CHUNK_SIZE 1024

void handleError(char *errorMsg);
void processClientRequest(int clientSocket);

int main(int argc, char *argv[]) {
    int serverSocket;
    int clientSocket;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    unsigned short serverPort;
    unsigned int clientAddrLen;

    if(argc!=2) {
        fprintf(stderr, "Usage: %s <Port>\n", argv[0]);
        exit(1);
    }

    serverPort = atoi(argv[1]);

    if((serverSocket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <0)
        handleError("socket() failed");

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(serverPort);

    if(bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        handleError("bind() failed");

    if(listen(serverSocket, MAX_BACKLOG) < 0)
        handleError("listen() failed");

    for(;;) {
        clientAddrLen = sizeof(clientAddr);
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0)
            handleError("accept() failed");

        printf("Connected to client: %s\n", inet_ntoa(clientAddr.sin_addr));
        processClientRequest(clientSocket);
    }

    return 0;
}

void handleError(char *errorMsg) {
    printf("Error: %s\n", errorMsg);
    exit(1);
}

void processClientRequest(int clientSocket) {
    char recvBuffer[CHUNK_SIZE];
    int receivedBytes = recv(clientSocket, recvBuffer, CHUNK_SIZE - 1, 0);

    if (receivedBytes <= 0) {
        handleError("recv() failed or client disconnected");
        return;
    }

    recvBuffer[receivedBytes] = '\0';
    printf("Request received:\n%s\n", recvBuffer);

    char httpMethod[16], filePath[256], protocol[16];
    sscanf(recvBuffer, "%s %s %s", httpMethod, filePath, protocol);

    if (strcmp(httpMethod, "GET") != 0) {
        const char *methodNotAllowed = "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
        send(clientSocket, methodNotAllowed, strlen(methodNotAllowed), 0);
        close(clientSocket);
        return;
    }

    if (filePath[0] == '/') memmove(filePath, filePath + 1, strlen(filePath));

    if (strlen(filePath) == 0) {
        strcpy(filePath, "index.html");
    }

    FILE *requestedFile = fopen(filePath, "r");
    if (!requestedFile) {
        const char *notFoundResponse = "HTTP/1.1 404 Not Found\r\n"
                                       "Content-Type: text/html\r\n\r\n"
                                       "<html><body><h1>404 Not Found</h1></body></html>";
        send(clientSocket, notFoundResponse, strlen(notFoundResponse), 0);
    } else {
        const char *okHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
        send(clientSocket, okHeader, strlen(okHeader), 0);

        char fileChunk[CHUNK_SIZE];
        size_t bytesRead;
        while ((bytesRead = fread(fileChunk, 1, CHUNK_SIZE, requestedFile)) > 0) {
            send(clientSocket, fileChunk, bytesRead, 0);
        }
        fclose(requestedFile);
    }

    close(clientSocket);
}
