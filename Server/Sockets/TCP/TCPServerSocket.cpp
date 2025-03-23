//
// Created by pozni on 3/9/2025.
//

#include <exception>
#include <iostream>


#include "ServerHandler.h"
#include "TCPServerSocket.h"

WSADATA ISocket::wsaData = {};

TCPServerSocket::TCPServerSocket(const std::string& ip, const std::string& port)
{
    ISocket::ip = ip;
    ISocket::port = port;
}

TCPServerSocket::~TCPServerSocket()
{
    //
}




bool TCPServerSocket::Initialize()
{
    // Initialize WSA, MAKE SINGLETON
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
    }

    addrinfo hints = {};
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family     = AF_INET;
    hints.ai_socktype   = SOCK_STREAM;
    hints.ai_protocol   = IPPROTO_TCP;
    hints.ai_flags      = AI_PASSIVE; //SERVER

    addrinfo* result = nullptr;

    auto status = getaddrinfo(ISocket::ip.c_str(), ISocket::port.c_str(), &hints, &result);
    if (status != 0)
    {
        // TODO: Add handling status codes // REDIRECT TO LOGGER !!!!!!!!!!!!
        std::cout << "getaddrinfo failed with error: " << status << std::endl;
        return false;
    }

    m_listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (m_listenSocket == INVALID_SOCKET)
    {
        // TODO: Add handling status codes // REDIRECT TO LOGGER !!!!!!!!!!!!
        std::cerr << "socket failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        return false;
    }

    // Bind the socket
    status = bind(m_listenSocket, result->ai_addr, result->ai_addrlen);
    if (status == SOCKET_ERROR)
    {
        if (status == WSAEADDRINUSE)
        {
            // TODO: Add handling status codes // REDIRECT TO LOGGER !!!!!!!!!!!!
            std::cerr << "Address already in use" << std::endl;
            freeaddrinfo(result);
            closesocket(m_listenSocket);
            return false;
        }
        // TODO: Add handling status codes // REDIRECT TO LOGGER !!!!!!!!!!!!
        std::cerr << "bind failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(m_listenSocket);
    }

    freeaddrinfo(result);

    //Listen for incoming connections
    if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        // TODO: Add handling status codes // REDIRECT TO LOGGER !!!!!!!!!!!!
        std::cout << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(m_listenSocket);
        return false;
    }
    m_listener = std::make_pair(m_listenSocket, new ServerHandler());
    std::cout << "[SERVER SOCKET] Server initialized" << std::endl;
    return true;
}

void TCPServerSocket::Close()
{
}

void TCPServerSocket::SetTimeout(int milliseconds) const
{
    //
}

void TCPServerSocket::RegisterHandler(SOCKET socket, IEventHandler* handler)
{
    if (handler == nullptr)
    {
        throw std::invalid_argument("Handler is nullptr");
    }
    m_clientSockets[socket] = handler;
    std::cout << "[SERVER SOCKET] New handler registered" << std::endl;

}

void TCPServerSocket::RemoveHandler(SOCKET socket)
{
    auto handler = m_clientSockets[socket]->m_sessionId;
    delete m_clientSockets[socket];
    m_clientSockets.erase(socket);
    std::cout << "[SERVER SOCKET] Handler removed " << handler << std::endl;
}

bool TCPServerSocket::IsOpen() const
{
    return true;
}

void TCPServerSocket::eventLoop()
{
    timeval m_timeout = {0, 100};
    while (m_isRunning)
    {
        FD_SET listener;
        FD_ZERO(&listener);
        FD_SET(m_listenSocket, &listener);

        {
            // Accepting logic
            timeval timeout = {0, 250};
            int result = select(0, &listener, nullptr, nullptr, &timeout);
            if (result == SOCKET_ERROR)
            {
                // TODO: LOGER !!!
                std::cerr << "[SERVER SOCKET][ERROR] (eventLoop() -> select(server) failed with error: " << WSAGetLastError()
                    << std::endl;
            }
            if (FD_ISSET(m_listenSocket, &listener))
            {
                m_listener.second->HandleAccept(m_listenSocket, this);
            }
        }
        // --------------------------------
        {
            // IO LOGIC
            FD_SET readSet;
            FD_SET writeSet;
            FD_SET errorSet;
            FD_ZERO(&readSet);
            FD_ZERO(&writeSet);
            FD_ZERO(&errorSet);

            if (m_clientSockets.empty()) continue;
            for (const auto& sock : m_clientSockets)
            {
                if(sock.first == INVALID_SOCKET)
                {
                    std::cerr << "[SERVER SOCKET] Invalid socket" << std::endl;
                    continue;
                }
                FD_SET(sock.first, &readSet);
                FD_SET(sock.first, &errorSet);
                if(sock.second->HasDataToSend())
                {
                    FD_SET(sock.first, &writeSet);
                }
            }


            int result = select(0, &readSet, &writeSet, nullptr, &m_timeout);
            if (result == SOCKET_ERROR)
            {
                std::cerr << "[SERVER SOCKET]-> eventLoop() -> select( read ) failed with error: " << WSAGetLastError()
                    << std::endl;
            }
            else if (result > 0)
            {
                for (const auto& sock : m_clientSockets)
                {
                    if (FD_ISSET(sock.first, &readSet))
                    {
                        sock.second->HandleRead(sock.first, this);
                    }
                    if (FD_ISSET(sock.first, &errorSet))
                    {
                        sock.second->HandleError(sock.first, this);
                    }
                    if (FD_ISSET(sock.first, &writeSet))
                    {
                        sock.second->HandleWrite(sock.first, this);
                    }
                }
            }
        }
        // ---------------------------------------------
        {
        }
    }
}
