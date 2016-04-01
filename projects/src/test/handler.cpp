#include <stdio.h>
#include "coro.h"



void msg_handle1(void *custom_data, Coro* coro)
{
    printf("msg_handle1 \n");
    coro->yield();
    printf("msg1\n");
    coro->yield();
    printf("msg2\n");
}

void msg_handle2(void *custom_data, Coro* coro)
{
    printf("msg_handle2 \n");
}

void msg_handle_default(void *custom_data, Coro* coro)
{
    printf("msg_handle default \n");
}
