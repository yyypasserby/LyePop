/*************************************************************************
    > File Name: pthreadHead.h
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年09月01日 星期日 10时18分12秒
 ************************************************************************/

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

void createPthread(pthread_t* threadfd, void* (*function)(void*), void* arg);
void joinPthread(pthread_t* threadfd, void* thread_res);
void createDetachPthread(pthread_t* threadfd, void* (*function)(void*),
                         void* arg);
