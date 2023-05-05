#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 1024

void *handle_client(void *arg)
{
    int new_socket = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};

    // Receive operation and operands from client
    recv(new_socket, buffer, BUFFER_SIZE, 0);
    int op1, op2, result;
    char op;
    sscanf(buffer, "%d %c %d", &op1, &op, &op2);

    // Perform calculation
    switch (op)
    {
    case '+':
        result = op1 + op2;
        break;
    case '-':
        result = op1 - op2;
        break;
    case '*':
        result = op1 * op2;
        break;
    case '/':
        if (op2 == 0)
        {
            result = -1; // Indicates a division by zero error
        }
        else
        {
            result = op1 / op2;
        }
        break;
    default:
        result = -2; // Indicates an invalid operator error
    }

    // Send result to client
    send(new_socket, &result, sizeof(result), 0);

    // Close connection with client
    close(new_socket);

    pthread_exit(NULL);
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    // Wait for incoming connections and process data concurrently
    printf("Server is running and waiting for connections...\n");
    while (1)
    {
        int new_socket;
        struct sockaddr_in client_address;
        int client_addrlen = sizeof(client_address);

        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t *)&client_addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Print incoming connection details
        char client_address_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_address.sin_addr), client_address_str, INET_ADDRSTRLEN);
        printf("Accepted connection from %s:%d\n", client_address_str, ntohs(client_address.sin_port));

        // Create a child process to handle the client
        pid_t pid;
        if ((pid = fork()) == 0)
        {
            // Close the original socket file descriptor in the child process
            close(server_fd);

            // Process client requests
            while (1)
            {
                char buffer[BUFFER_SIZE] = {0};
                char response[BUFFER_SIZE] = {0};
                int valread = read(new_socket, buffer, BUFFER_SIZE);

                if (valread == 0)
                {
                    // Client disconnected
                    printf("Client disconnected.\n");
                    break;
                }
                else if (valread < 0)
                {
                    // Error reading from client
                    perror("read");
                    break;
                }
                else
                {
                    // Parse request from client
                    char operation = buffer[0];
                    int num1, num2;
                    sscanf(buffer + 1, "%d %d", &num1, &num2);

                    // Calculate result
                    int result;
                    switch (operation)
                    {
                    case '+':
                        result = num1 + num2;
                        break;
                    case '-':
                        result = num1 - num2;
                        break;
                    case '*':
                        result = num1 * num2;
                        break;
                    case '/':
                        result = num1 / num2;
                        break;
                    default:
                        // Invalid operation
                        strcpy(response, "Invalid operation.\n");
                        send(new_socket, response, strlen(response), 0);
                        continue; // Go back to listening for requests
                    }

                    // Send response to client
                    sprintf(response, "Result: %d\n", result);
                    send(new_socket, response, strlen(response), 0);
                }
            }

            // Close the socket file descriptor in the child process
            close(new_socket);

            exit(0);
        }

        // Close the client socket in the parent process
        close(new_socket);
    }

    return 0;
}
