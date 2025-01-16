#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define MAX_HTTP_REQUEST 2048

void handleError(char *errorMsg);
void calculateRTT(int socket, struct sockaddr_in *serverAddr, const char *httpRequest, size_t requestLen);

int main(int argc, char *argv[]) {
    int clientSocket;
    struct sockaddr_in serverAddr;
    unsigned short serverPort;
    char *serverHost;
    char *resourcePath;
    char httpRequest[MAX_HTTP_REQUEST];
    char serverResponse[BUFFER_SIZE];
    int bytesReceived, totalBytesReceived;

    if (argc < 3 || argc > 4) {
        fprintf(stderr, "Usage: %s [-p] <Server Host> <Port Number>\n", argv[0]);
        exit(1);
    }

    int measureRTTFlag = 0;

    if (argc == 4 && strcmp(argv[1], "-p") == 0) {
        measureRTTFlag = 1;
        serverHost = argv[2];
        serverPort = atoi(argv[3]);
    } else if (argc == 3) {
        serverHost = argv[1];
        serverPort = atoi(argv[2]);
    } else {
        fprintf(stderr, "Invalid arguments\n");
        exit(1);
    }

    char *hostName = strtok(serverHost, "/");
    char *resource = strtok(NULL, "");

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        handleError("socket() failed");

    struct addrinfo hints, *addrInfo;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int addrStatus = getaddrinfo(hostName, NULL, &hints, &addrInfo);
    if (addrStatus != 0) {
        fprintf(stderr, "getaddrinfo() failed: %s\n", gai_strerror(addrStatus));
        exit(1);
    }

    memcpy(&serverAddr, addrInfo->ai_addr, sizeof(serverAddr));
    serverAddr.sin_port = htons(serverPort);

    freeaddrinfo(addrInfo);

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        handleError("connect() failed");

    snprintf(httpRequest, MAX_HTTP_REQUEST,
             "GET /%s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Connection: close\r\n\r\n", resource ? resource : "", hostName);

    if (measureRTTFlag) {
        calculateRTT(clientSocket, &serverAddr, httpRequest, strlen(httpRequest));
    } else {
        if (send(clientSocket, httpRequest, strlen(httpRequest), 0) != strlen(httpRequest))
            handleError("send() sent an unexpected number of bytes");

        totalBytesReceived = 0;
        while (totalBytesReceived < BUFFER_SIZE) {
            bytesReceived = recv(clientSocket, serverResponse + totalBytesReceived, BUFFER_SIZE - totalBytesReceived, 0);
            if (bytesReceived <= 0)
                handleError("recv() failed or connection closed early");

            totalBytesReceived += bytesReceived;
        }

        printf("Server Response:\n%s\n", serverResponse);
    }

    close(clientSocket);
    return 0;
}

void calculateRTT(int socket, struct sockaddr_in *serverAddr, const char *httpRequest, size_t requestLen) {
    struct timespec startTime, endTime;
    long long roundTripTime;

    clock_gettime(CLOCK_MONOTONIC, &startTime);

    if (send(socket, httpRequest, requestLen, 0) != requestLen)
        handleError("send() sent an unexpected number of bytes");

    char serverResponse[BUFFER_SIZE];
    int bytesReceived = recv(socket, serverResponse, BUFFER_SIZE, 0);
    if (bytesReceived <= 0)
        handleError("recv() failed or connection closed prematurely");

    clock_gettime(CLOCK_MONOTONIC, &endTime);

    roundTripTime = (endTime.tv_sec - startTime.tv_sec) * 1000L + (endTime.tv_nsec - startTime.tv_nsec) / 1000000L;

    printf("RTT: %lld ms\n", roundTripTime);
    printf("Server Response:\n%s\n", serverResponse);
}

void handleError(char *errorMsg) {
    perror(errorMsg);
    exit(1);
}
