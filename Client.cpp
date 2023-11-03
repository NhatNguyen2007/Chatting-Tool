#include "Client.h"

Client &Client::operator=(const Client *rhs)
{
    if (this == rhs)
        return *this;
    this->clientSocket = rhs->clientSocket;
    this->pID = rhs->pID;
    delete[] clientName;
    this->clientName = new char[strlen(rhs->clientName) + 1];
    strcpy(this->clientName, rhs->clientName);
    return *this;
}

bool Client::operator==(const Client &rhs) const
{
    if ((this->clientSocket == rhs.clientSocket) && (this->pID == rhs.pID) && (strcmp(this->clientName, rhs.clientName) == 0))
        return true;
    return false;
}

int Client::getClientSocket() const
{
    return this->clientSocket;
}

pid_t Client::getPID() const
{
    return this->pID;
}

void Client::setName(const char *clientName)
{
    delete[] this->clientName;
    this->clientName = new char[strlen(clientName) + 1];
    strcpy(this->clientName, clientName);
}

const char *Client::getName() const
{
    return this->clientName;
}
