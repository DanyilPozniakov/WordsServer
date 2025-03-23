//
// Created by pozni on 3/2/2025.
//

#ifndef WORDSSERVER_H
#define WORDSSERVER_H

#include <thread>
#include <vector>


class WordsServer
{
public:
    std::thread m_reactorThread;
    std::vector<std::thread> m_workers;

};



#endif //WORDSSERVER_H
