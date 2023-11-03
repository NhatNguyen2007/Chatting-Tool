#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <csignal>
#include <atomic>
#include <vector>
#include <thread>
#include <fcntl.h>
#include <algorithm>
#include "Client.h"

#define PORT 2000

using namespace std;

atomic<bool> ctrlCPressed(false);

vector<int> connectedClientSockets;
vector<Client> connectedClients;

void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        ctrlCPressed.store(true);
    }
}

void setSocketNonBlocking(int& client_fd){
    int flag = fcntl(client_fd, F_GETFL, 0);
    if(flag == -1)
        return;
    fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);
}

void removeClient(int clientSocket)
{
    auto it = find(connectedClientSockets.begin(), connectedClientSockets.end(), clientSocket);
    if(it != connectedClientSockets.end()){
        connectedClientSockets.erase(it);
        cout << "Remove client successfully\n";
    }
    else{
        cout << "Client not found\n";
    }

    close(clientSocket);
}

void broadcastMessage(int clientSocket, char* message)
{
    for (auto client = connectedClientSockets.begin(); client != connectedClientSockets.end(); client++)
    {
        if (*client == clientSocket)
        {
            continue;
        }

        send(*client, message, strlen(message), 0);
    }
}

// void clientHandler(int clientSocket)
// {
//     char buffer[2048];
//     ssize_t bytesRead;
//     while (true)
//     {
//         memset(buffer, 0, sizeof(buffer));
//         bytesRead = read(clientSocket, buffer, sizeof(buffer) - 1);
        
//         // if (bytesRead <= 0)
//         // {
//         //     cout << "A client disconnect\n";
//         //     removeClient(clientSocket);
//         //     break;
//         // }
//         // else 
//         if (buffer[0] != '\0'){
//             cout << buffer << endl;
//             broadcastMessage(clientSocket, buffer);
//         }

//         this_thread::sleep_for(chrono::milliseconds(100));
//     }
// }

void clientHandler(Client client)
{
    char message[2048];
    char buffer[2048];
    ssize_t bytesRead;
    string pidStr;
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        bytesRead = read(client.getClientSocket(), buffer, sizeof(buffer) - 1);

        strcpy(message, client.getName());
        strcat(message, "-");
        pidStr = to_string(client.getPID());
        strcat(message, pidStr.c_str());
        strcat(message, ": ");
        strcat(message, buffer);
        
        if (buffer[0] != '\0'){
            cout << message << endl;
            broadcastMessage(client.getClientSocket(), message);
        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main()
{
    signal(SIGINT, signalHandler);

    vector<thread> clientThreads;

    int server_fd, newSocket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int option = 1;

    ssize_t bytesRead;
    char buffer[2048] = {0};
    int pidInt;
    pid_t pid;
    int roomId;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "socket failed\n";
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option)))
    {
        cerr << "setsockopt failed\n";
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        cerr << "bind failed\n";
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        cerr << "listen failed\n";
        exit(EXIT_FAILURE);
    }

    // while(!ctrlCPressed.load()){
    //     memset(buffer, 0, sizeof(buffer));
    //     bytesRead = read(newSocket, buffer, 2048 - 1);
    //     if(buffer[0] != '\0')
    //         cout << "Client: " << buffer << endl;
    //     strcpy(message, header);
    //     strcat(message, buffer);
    //     send(newSocket, message, strlen(message), 0);
    // }

    while (true)
    {
        if ((newSocket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            cerr << "accept failed\n";
            continue;
        }

        memset(buffer, 0, sizeof(buffer));
        bytesRead = read(newSocket, buffer, 2048 - 1);
        pidInt = atoi(buffer);
        pid = static_cast<pid_t>(pidInt);

        memset(buffer, 0, sizeof(buffer));
        bytesRead = read(newSocket, buffer, 2048 - 1);

        Client newClient{newSocket, pid, buffer};

        bytesRead = read(newSocket, &roomId, sizeof(roomId));
        roomId = ntohl(roomId);

        cout << "Client name: " << newClient.getName() << endl;
        cout << "Client pid: " << newClient.getPID() << endl;
        cout << "Client room id: " << roomId << endl;

        setSocketNonBlocking(newSocket);

        connectedClientSockets.push_back(newSocket);
        connectedClients.push_back(newClient);

        thread clientThread(clientHandler, newClient);
        clientThreads.push_back(move(clientThread));

        cout << "Client thread is created\n";
    }

    for (auto &thread : clientThreads)
    {
        thread.join();
    }

    close(server_fd);

    return 0;
}