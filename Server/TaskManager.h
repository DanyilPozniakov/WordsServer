//
// Created by pozni on 3/23/2025.
//

#ifndef TASKMANAGER_H
#define TASKMANAGER_H


#include <unordered_set>

class Task
{

};


#include "TCPServerSocket.h"

class ClientHandler;

class TaskManager
{
public:
    TaskManager();
    virtual ~TaskManager();


    void AddConnection(ClientHandler* connection);

    bool CheckConnections();

private:
    std::vector<ClientHandler*> m_connections;

    //std::queue<Task> taskQueueInProgress;
    // -->> handler
    //std::queue<Task> taskQueueReady;
};


#endif //TASKMANAGER_H
