#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <csignal>
#include <atomic>
#include <thread>
#include <fcntl.h>

#define PORT 2000

using namespace std;

atomic<bool> ctrlCPressed(false);
char clientName[100];

void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        ctrlCPressed.store(true);
    }
}

void setSocketNonBlocking(int &client_fd)
{
    int flag = fcntl(client_fd, F_GETFL, 0);
    if (flag == -1)
        return;
    fcntl(client_fd, F_SETFL, flag | O_NONBLOCK);
}

void readMessage(int &client_fd)
{
    char buffer[2048] = {0};
    ssize_t valRead;
    while (true)
    {
        memset(buffer, 0, sizeof(buffer));
        valRead = read(client_fd, buffer, 2048 - 1);
        if (buffer[0] != '\0')
        {
            cout << endl << buffer << endl;
            cout << "You: " << flush;
        }
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

void sendMessage(int &client_fd)
{
    char message[2048];
    
    while (true)
    {
        cout << "You: ";
        cin.getline(message, sizeof(message));
        if (message[0] == '\0')
        {
            continue;
        }

        send(client_fd, message, strlen(message), 0);
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main()
{
    signal(SIGINT, signalHandler);

    pid_t pid = getpid();
    cout << pid << endl;
    string pidToSend = to_string(pid);

    int client_fd, status;
    struct sockaddr_in server_address;
    int roomId, roomIdToSend;

    const char *welcome = "Welcome to chatting system!\n";

    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "socket creation failed\n";
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "10.0.2.15", &server_address.sin_addr) <= 0)
    {
        cerr << "Invalid address / Address not supported\n";
        exit(EXIT_FAILURE);
    }

    if ((status = connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address))) < 0)
    {
        cerr << "Connection failed\n";
        exit(EXIT_FAILURE);
    }

    send(client_fd, pidToSend.c_str(), strlen(pidToSend.c_str()), 0);

    cout << welcome;

    cout << "Please enter your name: ";
    cin.getline(clientName, sizeof(clientName));
    send(client_fd, clientName, strlen(clientName), 0);

    cout << "Please enter room id: ";
    cin >> roomId;
    cin.ignore();
    roomIdToSend = htonl(roomId);
    send(client_fd, &roomIdToSend, sizeof(roomIdToSend), 0);

    setSocketNonBlocking(client_fd);
    thread sendThread(sendMessage, ref(client_fd));
    thread readThread(readMessage, ref(client_fd));

    sendThread.join();
    readThread.join();

    cout << "Unregister\n";

    close(client_fd);
    return 0;
}