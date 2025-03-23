//
// Created by pozni on 3/23/2025.
//

#include <iostream>
#include "ClientHandler.h"
#include "TaskManager.h"


TaskManager::TaskManager()
{
}

TaskManager::~TaskManager()
{
}

void TaskManager::AddConnection(ClientHandler* connection)
{
    m_connections.push_back(connection);
    std::cout << "[TASK MANAGER]!!! Connection added" << std::endl;
}

bool TaskManager::CheckConnections()
{
    bool hasData = false;
    for(const auto& connection : m_connections)
    {
        if(connection->HasDataToParse())
        {
            hasData = true;
            std::string data = connection->GetDataToParse();
            std::cout << "[TASK MANAGER]!!! Data to parse: " << data << std::endl;
        }
    }
    return hasData;
}
