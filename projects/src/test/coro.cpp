#include "coro.h"
#include "handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdint.h>
#include <vector>
#include <map>

#define CMD uint32_t 


extern ucontext_t uctx_main;

void coro_agent(uint32_t low, uint32_t hi)
{
   Coro* hcoro = (Coro*) ((uintptr_t)low | (uintptr_t)hi << 32);
   hcoro->func(NULL, hcoro);
   CoroMgr::getInstance().del_coro(hcoro->fd);
   hcoro->yield();
   //此处需要从map中删除掉该coro
}


Coro::Coro()
{
}

void Coro::init(int clientfd, uint32_t size)
{
    stack_mem = malloc(size);
    if(!stack_mem)
    {
        printf("malloc coro_stack failed...\n");
    }
    stack_size = size;
    getcontext(&uctx);
    uctx.uc_stack.ss_sp = stack_mem;
    uctx.uc_stack.ss_size = stack_size;
    uctx.uc_link = &uctx_main;
    uintptr_t ptr = (uintptr_t)this;
    makecontext(&uctx, (void (*)(void))coro_agent, 2, (uint32_t)ptr, (uint32_t)(ptr>>32));
    fd = clientfd;
}

Coro::~Coro()
{
    free(stack_mem);
}

void Coro::setCmd(int cmd)
{
    func = CoroMgr::getInstance().getHandlerByCmd(cmd);
}

void Coro::resume()
{
    swapcontext(&uctx_main, &uctx);
}

void Coro::yield()
{
    swapcontext(&uctx, &uctx_main);
}


