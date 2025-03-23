//
// Created by pozni on 3/15/2025.
//

#ifndef IEVENTHANDLER_H
#define IEVENTHANDLER_H

#include "Winsock2.h"

class ISocket;


class IEventHandler {
public:
    virtual ~IEventHandler() = default;

    virtual void HandleRead(SOCKET socket, ISocket* reactor)        = 0;
    virtual void HandleWrite(SOCKET socket, ISocket* reactor)       = 0;
    virtual void HandleError(SOCKET socket, ISocket* reactor)       = 0;
    virtual bool HasDataToSend()                                    = 0;

    uint32_t m_sessionId = 0;

};



#endif //IEVENTHANDLER_H
