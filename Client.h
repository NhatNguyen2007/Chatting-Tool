#include <iostream>
#include <cstring>

using namespace std;

class Client
{
private:
    int clientSocket;
    pid_t pID;
    char *clientName;

public:
    Client(int clientSocket, pid_t pID, char* clientName) : clientSocket{clientSocket}, pID{pID}, clientName{nullptr}
    {
        this->clientName = new char[strlen(clientName) + 1];
        strcpy(this->clientName, clientName);
    }

    Client(const Client& rhs) : clientSocket{rhs.clientSocket}, pID{rhs.pID}, clientName{nullptr}
    {
        this->clientName = new char[strlen(rhs.clientName) + 1];
        strcpy(this->clientName, rhs.clientName);
    }

    ~Client()
    {
        delete[] clientName;
    }

    Client &operator=(const Client *rhs);

    bool operator==(const Client& rhs) const;

    int getClientSocket()const;

    pid_t getPID() const;

    void setName(const char *clientName);

    const char *getName() const;
};