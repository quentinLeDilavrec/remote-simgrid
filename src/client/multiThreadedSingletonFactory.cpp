#include "multiThreadedSingletonFactory.hpp"
#include "../common.hpp"
#include "../rsg/services.hpp"
#include <iostream>
#include <stack>
#include <algorithm>    // std::copy

MultiThreadedSingletonFactory *MultiThreadedSingletonFactory::pInstance = NULL;
std::mutex MultiThreadedSingletonFactory::mtx;
std::mutex MultiThreadedSingletonFactory::threadMutex;

MultiThreadedSingletonFactory& MultiThreadedSingletonFactory::getInstance()  {
    if(pInstance == NULL) {
        pInstance = new MultiThreadedSingletonFactory();
    }
    return *pInstance;
}

MultiThreadedSingletonFactory::~MultiThreadedSingletonFactory() {
    delete pClients;
    delete pThreads;
    delete pMainThreadID;
}

Client &MultiThreadedSingletonFactory::getClient(std::thread::id id) {
    MultiThreadedSingletonFactory::mtx.lock();
    Client *res;
    std::hash<std::thread::id> hasher;
    try {
        res = pClients->at(id);
    } catch (std::out_of_range& e) {
        res = new Client("localhost", 9090);
        res->init();
        pClients->insert({std::this_thread::get_id(), res});
        *pMainThreadID = hasher(std::this_thread::get_id());        
    }
    MultiThreadedSingletonFactory::mtx.unlock();
    return *res;
}

void MultiThreadedSingletonFactory::registerClient(Client *client) {
    MultiThreadedSingletonFactory::mtx.lock();
    std::hash<std::thread::id> hasher;
    pClients->insert({std::this_thread::get_id(), client});
    if(pClients->size() == 1) {
        *pMainThreadID = hasher(std::this_thread::get_id());
    }    
    MultiThreadedSingletonFactory::mtx.unlock();
}

Client &MultiThreadedSingletonFactory::getClientOrCreate(std::thread::id id, int rpcPort) {
    Client *res;
    MultiThreadedSingletonFactory::mtx.lock();
    std::hash<std::thread::id> hasher;
    try {
        res = pClients->at(id);
    } catch (std::out_of_range& e) {
        res = new Client("localhost", 9090);
        res->connectToRpc(rpcPort);
        pClients->insert({std::this_thread::get_id(), res});
        if(pClients->size() == 1) {
            *pMainThreadID = hasher(std::this_thread::get_id());
        }
    }
    MultiThreadedSingletonFactory::mtx.unlock();
    return *res;
}

void MultiThreadedSingletonFactory::clearClient(std::thread::id id) {
    MultiThreadedSingletonFactory::mtx.lock();
    try {
        Client *client;
        client = pClients->at(id);
        client->close();
        client->reset();
        delete client;
        pClients->erase(id);
    } catch (std::out_of_range& e) {
        std::cerr << "Engine already cleared or not existing " << std::endl;
    }
    MultiThreadedSingletonFactory::mtx.unlock();
    std::hash<std::thread::id> hasher;
    size_t hash = hasher(id);
    if(hash == *pMainThreadID) {
        //FIXME We need to discuss of how to manage the actors life span.
        // When I first design the waitAll, I thought we might want to spawn a main actor which spawn other actors.
        // and while the child are still alived, the main leave. But It leads to many sync problème with all the thread.
        // This design will force any actor to wait for their childs.

        // waitAll();
        // if(pThreads->size() > 0) {
        //     std::cerr << "ERROR : rsg process is leaving whereas some actors may still be alives." << std::endl;
        // }
        delete pInstance;
    }
}

void MultiThreadedSingletonFactory::registerNewThread(std::thread *thread) {
    MultiThreadedSingletonFactory::threadMutex.lock();
    pThreads->push_back(thread);
    MultiThreadedSingletonFactory::threadMutex.unlock();
}

void MultiThreadedSingletonFactory::flushAll() {
    for(auto it = pClients->begin(); it != pClients->end(); ++it) {
        it->second->flush();
    }
}

void MultiThreadedSingletonFactory::waitAll() {
    
    typedef std::vector<std::thread*>::iterator it_type;
    
    std::stack<std::thread*> joined;
    std::vector<std::thread*> copied;
    MultiThreadedSingletonFactory::threadMutex.lock();
    bool stop = pThreads->empty();
    MultiThreadedSingletonFactory::threadMutex.unlock();
    
    while(!stop) {
        MultiThreadedSingletonFactory::threadMutex.lock();
        std::copy ( pThreads->begin(), pThreads->end(),   std::back_inserter(copied));
        MultiThreadedSingletonFactory::threadMutex.unlock();
        
        for(it_type iterator = copied.begin(); iterator != copied.end(); iterator++) {
            if((*iterator)->joinable()) {
                (*iterator)->join();
                joined.push(*iterator);
            } else {
                debug_process("Unjoinable thread remaiming in the vector");
            }
        }
        
        while(!joined.empty()) {
            std::thread *t = joined.top();
            joined.pop();
            MultiThreadedSingletonFactory::threadMutex.lock();
            it_type elem = std::find (pThreads->begin(), pThreads->end(), t);        
            pThreads->erase(elem);
            MultiThreadedSingletonFactory::threadMutex.unlock();
            delete t;
        }
        
        sleep(0.01);
        
        MultiThreadedSingletonFactory::threadMutex.lock();
        stop = pThreads->empty();
        MultiThreadedSingletonFactory::threadMutex.unlock();
        copied.clear();
    }  
}

void MultiThreadedSingletonFactory::clearAll(bool keepConnectionsOpen) {

    for(auto it = pThreads->begin(); it != pThreads->end(); ++it) {
        // if(((*it)->joinable()))
        // (*it)->detach();
        // delete *it;
        //FIXME In some cases, this thread clean up leads to a segmentation fault. (In sames to appears with fork) 
    }
    pThreads->clear();
    
    for(auto it = pClients->begin(); it != pClients->end(); ++it) {
        if(!keepConnectionsOpen) {
            it->second->close();
            it->second->reset();
        }
        delete it->second;
    }
    pClients->clear();
}


