//
// Created by pozni on 3/16/2025.
//

#include <iostream>
#include "ClientHandler.h"

ClientHandler::ClientHandler()
{
}

ClientHandler::~ClientHandler()
{
}


void ClientHandler::HandleRead(SOCKET socket, ISocket* reactor)
{
    int result = recv(socket, m_readBuffer.data(), DEFAULT_BUFFER_SIZE, 0);
    if(result == SOCKET_ERROR)
    {
        std::cerr << "[CLIENT HANDLER] HandleRead() -> recv failed with error: " << WSAGetLastError() << std::endl;
    }
    else if(result == 0)
    {
        std::cerr << "[CLIENT HANDLER] HandleRead() -> connection closed" << std::endl;
        reactor->RemoveHandler(socket);
    }
    else if(result > 0)
    {
        m_readBuffer[result] = '\0';
        std::string data(m_readBuffer.data(), result);
        {
            std::cout << "[CLIENT HANDLER] HandleRead() -> received data: " << data << std::endl;
            // std::lock_guard<std::mutex> lock(m_mutex);
            // m_queue.push(data);
        }
    }
    // PUSH TO PARSER AND QUEUE
}

void ClientHandler::HandleWrite(SOCKET socket, ISocket* reactor)
{
}

void ClientHandler::HandleError(SOCKET socket, ISocket* reactor)
{
}

bool ClientHandler::HasDataToSend()
{
    return false;
}

void ClientHandler::SetBufferSizes(int readSize, int writeSize)
{
}
