//
// Created by pozni on 3/2/2025.
//

#include <iostream>
#include <WordsServer.h>

#include "Sockets/TCP/TCPServerSocket.h"


int main(int argc, char* argv[])
{


    std::thread t1([&]()
    {
        TCPServerSocket server("localhost", "3434");
        server.Initialize();
        server.m_isRunning = true;
        while (true)
        {
            server.eventLoop();
        }
    });

    t1.join();
    return 0;
}
