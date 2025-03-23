//
// Created by pozni on 3/15/2025.
//

#ifndef SERVERHANDLER_H
#define SERVERHANDLER_H


#include <winsock2.h>

#include "ISocket.h"
#include "IEventHandler.h"



class Reactor;

class ServerHandler : public IEventHandler
{
public:
    ServerHandler();
    ~ServerHandler() override;

    void HandleAccept(SOCKET socket, ISocket* reactor);
    void HandleDisconnect(SOCKET socket, ISocket* reactor);

    void HandleRead(SOCKET socket, ISocket* reactor) override;
    void HandleWrite(SOCKET socket, ISocket* reactor) override;
    void HandleError(SOCKET socket, ISocket* reactor) override;

    bool HasDataToSend() override;

};



#endif //SERVERHANDLER_H
