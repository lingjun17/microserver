#ifndef __CORO_H__
#define __CORO_H__

#include "handler.h"
#include <stdint.h>
#include <ucontext.h>
#include <map>
#include <netinet/in.h>

#define ADD_HANDLE(cmd, func) do{instance.handler_map.insert(std::pair<uint32_t, corofunc>(cmd, func));} while(0);
#define CORO_STACK_SIZE 8*1024*1024

typedef void (*corofunc)(void* custom_data, Coro *coro);



class Coro
{
public:
    Coro();
    ~Coro();
    void resume();
    void yield();
    void setCmd(int cmd);
    void init(int clientfd, uint32_t size);

private:
    ucontext_t uctx;
    void *stack_mem;
    uint32_t stack_size;
public:
    int fd; //map key
    corofunc func;
};

class Session
{
public:
    int getUid();
    char* getClientAddr();
    int getClientPort();
private:
    sockaddr_in client_addr;
    uint32_t uid;
};




class CoroMgr
{
public:
    static CoroMgr& getInstance()
    {
        static CoroMgr instance;
        //新加的handler加在这里
        return instance;
    }

    void init()
    {
        CoroMgr &instance = CoroMgr::getInstance();
        ADD_HANDLE(1, msg_handle1);
        ADD_HANDLE(2, msg_handle2);
    }
    
    void add_coro(uint32_t fd, Coro* coro)
    {
        coro_map.insert(std::pair<uint32_t, Coro*>(fd, coro));
    }

    void del_coro(uint32_t fd)
    {
        it = coro_map.find(fd);
        if(it != coro_map.end())
        {
            coro_map.erase(it);
        }
    }

    Coro* get_coro(uint32_t fd)
    {
        it = coro_map.find(fd);
        if(it != coro_map.end())
        {
            return it->second;
        }
        return NULL;
    }

    void schedule()
    {
        for(it = coro_map.begin(); it != coro_map.end(); ++it)
        {
            it->second->resume();
        }
    }

    //根据fd获取到之前注册的handler
    corofunc getHandlerByCmd(int cmd)
    {
        handler_iter = handler_map.find(cmd);
        if(handler_iter == handler_map.end())
        {
            return msg_handle_default;
        }
        else
            return handler_iter->second;
    }
private:
    CoroMgr()
    {
        //初始化 cmd->handler
    }
    CoroMgr(const CoroMgr&);
    CoroMgr & operator = (const CoroMgr&);
    void add_handlers();
    std::map<uint32_t, Coro*> coro_map;
    std::map<uint32_t, Coro*>::iterator it;
    std::map<uint32_t, corofunc> handler_map;
    std::map<uint32_t, corofunc>::iterator handler_iter;
};


#endif
