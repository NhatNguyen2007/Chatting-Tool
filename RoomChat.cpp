#include <iostream>
#include <vector>
#include "Client.h"

using namespace std;

class GroupChat
{
private:
    int roomID;
    vector<Client> clientList;

public:
    GroupChat(int roomID) : roomID{roomID} {}

    ~GroupChat() {}

    void addClient(const Client &client)
    {
        this->clientList.push_back(client);
    }

    void removeClient(Client &client)
    {
        for (auto it = clientList.begin(); it != clientList.end(); it++)
        {
            if (*it == client)
            {
                clientList.erase(it);
                break;
            }
        }
    }

    int getRoomID() const
    {
        return this->roomID;
    }

    vector<Client> getClientList() const
    {
        return this->clientList;
    }
};