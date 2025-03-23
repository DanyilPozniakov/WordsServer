//
// Created by pozni on 3/9/2025.
//

#ifndef TCPSERVERSOCKET_H
#define TCPSERVERSOCKET_H

#include <Winsock2.h>

#include <atomic>
#include <array>
#include <utility>
#include <map>

#include <IEventHandler.h>
#include <ServerHandler.h>
#include <ISocket.h>


class TCPServerSocket : public ISocket
{
public:
    explicit TCPServerSocket(const std::string& ip, const std::string& port);
    ~TCPServerSocket() override;

    bool Initialize()                   override;
    void Close()                        override;

    void SetTimeout(int milliseconds) const;
    void RegisterHandler(SOCKET socket, IEventHandler* handler) override;
    void RemoveHandler(SOCKET socket) override;


    [[nodiscard]] bool IsOpen() const;


    void eventLoop();
    std::atomic<bool> m_isRunning = false;

private:
    std::pair<SOCKET, ServerHandler*>   m_listener;
    std::map<SOCKET, IEventHandler*>    m_clientSockets;








    // void redirectToLogger(ILogger* logger, const std::string& message) const;
};


#endif //TCPSERVERSOCKET_H
