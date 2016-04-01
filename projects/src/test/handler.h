#ifndef __HANDLER_H__
#define __HANDLER_H__

class Coro;

void msg_handle1(void *custom_data, Coro* coro);

void msg_handle2(void *custom_data, Coro* coro);

void msg_handle_default(void *custom_data, Coro* coro);

#endif

