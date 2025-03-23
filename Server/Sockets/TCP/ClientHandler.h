//
// Created by pozni on 3/16/2025.
//

#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H


#include <winsock2.h>
#include <IEventHandler.h>

#include "ISocket.h"
#include <array>


class ClientHandler : public IEventHandler
{
public:
    ClientHandler();
    ~ClientHandler() override;


    void HandleRead(SOCKET socket, ISocket* reactor) override;
    void HandleWrite(SOCKET socket, ISocket* reactor) override;
    void HandleError(SOCKET socket, ISocket* reactor) override;
    bool HasDataToSend() override;

    void SetBufferSizes(int readSize, int writeSize);

private:
    int m_sessionId = 0;
    int m_requestsCount = 0;
    int m_responsesCount = 0;


    std::array<char, DEFAULT_BUFFER_SIZE> m_readBuffer = {};
    std::array<char, DEFAULT_BUFFER_SIZE> m_writeBuffer = {};
};


#endif //CLIENTHANDLER_H
