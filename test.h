#include <winsock2.h>
#include <iostream>
#include <map>
#include <vector>

#pragma comment(lib, "ws2_32.lib")  // Для линковки Winsock2

// forward declarations
class Reactor;
class EventHandler;

// --------------------------------------------------------------------------
// EventHandler: базовый класс обработчика события
// --------------------------------------------------------------------------
class EventHandler {
public:
    virtual ~EventHandler() = default;

    // Вызывается, когда сокет готов к чтению
    virtual void handleRead(SOCKET sock, Reactor* reactor) = 0;

    // Вызывается, когда сокет готов к записи (если нужно)
    // Для простоты можем не использовать в данном примере
    virtual void handleWrite(SOCKET sock, Reactor* reactor) {}

    // Можно добавить еще методы, если нужны события ошибок и т.д.
};

// --------------------------------------------------------------------------
// Reactor: хранит (sock -> EventHandler) и крутит главный цикл select()
// --------------------------------------------------------------------------
class Reactor {
public:
    Reactor() = default;

    // Регистрируем сокет и связываем его с обработчиком
    void registerHandler(SOCKET sock, EventHandler* handler) {
        handlers_[sock] = handler;
    }

    // Удаляем сокет и обработчика (например, при закрытии соединения)
    void removeHandler(SOCKET sock) {
        closesocket(sock);
        handlers_.erase(sock);
    }

    // Запуск главного цикла
    void eventLoop() {
        while (true) {
            // Соберём все сокеты в fd_set
            fd_set readSet;
            FD_ZERO(&readSet);

            // Заполним readSet всеми зарегистрированными сокетами
            for (auto& kv : handlers_) {
                SOCKET s = kv.first;
                FD_SET(s, &readSet);
            }

            // Таймаут (можно сделать nullptr, если хотим блокироваться бесконечно)
            struct timeval tv;
            tv.tv_sec = 1;   // 1 секунда
            tv.tv_usec = 0;

            // select — ждем событий в readSet
            int result = select(0, &readSet, NULL, NULL, &tv);
            if (result == SOCKET_ERROR) {
                std::cerr << "select error: " << WSAGetLastError() << std::endl;
                break;
            }

            // Если результат 0 — таймаут, никто не готов к чтению
            if (result == 0) {
                // Можно что-то делать в "фоновом" режиме
                continue;
            }

            // Проверяем, какие сокеты стали "ready to read"
            std::vector<SOCKET> readySockets;
            readySockets.reserve(result);

            for (auto& kv : handlers_) {
                SOCKET s = kv.first;
                if (FD_ISSET(s, &readSet)) {
                    readySockets.push_back(s);
                }
            }

            // Теперь вызываем handleRead() у соответствующих обработчиков
            for (SOCKET s : readySockets) {
                auto it = handlers_.find(s);
                if (it != handlers_.end()) {
                    EventHandler* handler = it->second;
                    handler->handleRead(s, this);
                }
            }
        }
    }

private:
    // Карта "сокет -> обработчик"
    std::map<SOCKET, EventHandler*> handlers_;
};

// --------------------------------------------------------------------------
// ServerHandler: обрабатывает серверный сокет (accept новых подключений)
// --------------------------------------------------------------------------
class ServerHandler : public EventHandler {
public:
    explicit ServerHandler(SOCKET listenSock) : listenSock_(listenSock) {}

    // handleRead для серверного сокета означает "есть входящее подключение"
    void handleRead(SOCKET sock, Reactor* reactor) override {
        // Принимаем клиента
        sockaddr_in clientAddr;
        int addrLen = sizeof(clientAddr);
        SOCKET clientSock = accept(sock, (sockaddr*)&clientAddr, &addrLen);
        if (clientSock == INVALID_SOCKET) {
            std::cerr << "accept failed: " << WSAGetLastError() << std::endl;
            return;
        }

        std::cout << "New client accepted! Socket=" << clientSock << std::endl;

        // Устанавливаем неблокирующий режим (можно, если нужно)
        u_long mode = 1; // 1 означает неблокирующий
        ioctlsocket(clientSock, FIONBIO, &mode);

        // Регистрируем новый сокет в Reactor c ClientHandler
        reactor->registerHandler(clientSock, new ClientHandler());
    }

private:
    SOCKET listenSock_;
};

// --------------------------------------------------------------------------
// ClientHandler: обрабатывает клиентский сокет (чтение/echo)
// --------------------------------------------------------------------------
class ClientHandler : public EventHandler {
public:
    void handleRead(SOCKET sock, Reactor* reactor) override {
        char buffer[1024];
        ZeroMemory(buffer, sizeof(buffer));

        int bytesReceived = recv(sock, buffer, sizeof(buffer), 0);
        if (bytesReceived > 0) {
            std::cout << "Received from client[" << sock << "]: " << buffer << std::endl;

            // Простейший эхо-ответ
            int bytesSent = send(sock, buffer, bytesReceived, 0);
            if (bytesSent == SOCKET_ERROR) {
                std::cerr << "send failed: " << WSAGetLastError() << std::endl;
                // Можно удалить сокет
                reactor->removeHandler(sock);
                delete this; // освобождаем текущий handler
            }
        }
        else if (bytesReceived == 0) {
            // Клиент закрыл соединение
            std::cout << "Client[" << sock << "] disconnected." << std::endl;
            reactor->removeHandler(sock);
            delete this;
        }
        else {
            // Ошибка recv
            int err = WSAGetLastError();
            if (err != WSAEWOULDBLOCK) {
                std::cerr << "recv error: " << err << std::endl;
                reactor->removeHandler(sock);
                delete this;
            }
        }
    }
};

// --------------------------------------------------------------------------
// Функция инициализации Winsock и запуска Reactor
// --------------------------------------------------------------------------
int main() {
    // 1. Инициализируем Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        return 1;
    }

    // 2. Создаём слушающий сокет
    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 3. Связываем с портом и слушаем
    sockaddr_in serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(12345);

    if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "bind failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen failed: " << WSAGetLastError() << std::endl;
        closesocket(listenSock);
        WSACleanup();
        return 1;
    }

    // Переводим слушающий сокет в неблокирующий режим (рекомендуется)
    u_long mode = 1; // 1 означает неблокирующий
    ioctlsocket(listenSock, FIONBIO, &mode);

    // 4. Создаём Reactor
    Reactor reactor;

    // 5. Регистрируем серверный сокет с ServerHandler
    ServerHandler* serverHandler = new ServerHandler(listenSock);
    reactor.registerHandler(listenSock, serverHandler);

    // 6. Запускаем event loop
    std::cout << "Server is running on port 12345. Press Ctrl+C to stop.\n";
    reactor.eventLoop();

    // 7. Cleanup (по выходу из цикла)
    closesocket(listenSock);
    WSACleanup();
    return 0;
}