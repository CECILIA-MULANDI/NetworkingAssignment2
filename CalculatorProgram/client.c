#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8888
#define BUFFER_SIZE 1024

int main(int argc, char const *argv[])
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char operation;
    int num1, num2;

    // Create socket file descriptor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    // Read user input and send request to server
    while (1)
    {
        printf("Enter operation (+, -, *, /) and two numbers: ");
        scanf("%d%c%d", &num1, &operation, &num2);
        getchar(); // consume newline character

        // Send request to server
        sprintf(buffer, "%c %d %d", operation, num1, num2);
        send(sock, buffer, strlen(buffer), 0);

        // Receive response from server
        memset(buffer, 0, BUFFER_SIZE);
        valread = read(sock, buffer, BUFFER_SIZE);

        if (valread == 0)
        {
            printf("Server disconnected.\n");
            break;
        }
        else if (valread < 0)
        {
            perror("read");
            break;
        }
        else
        {
            printf("Result: %s\n", buffer);
        }
    }

    close(sock);

    return 0;
}
