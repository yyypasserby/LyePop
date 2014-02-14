/*************************************************************************
    > File Name: pthreadFunc.c
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年09月01日 星期日 10时16分55秒
 ************************************************************************/

#include "sockHead.h"
#include "pthreadHead.h"


void createPthread(pthread_t * threadfd, void * ( *function )( void * ), void* arg ){
    int res = pthread_create( threadfd, NULL, function, arg );
    if( res !=0 ){
        perror("can't create pthread!\n");
        exit(1);
    }
}

void joinPthread( pthread_t * threadfd, void * thread_res ){
    int res = pthread_join( *threadfd, & thread_res );
    if( res != 0 ){
        perror("can't join pthread!\n");
        exit(1);
    }
}

void createDetachPthread( pthread_t * threadfd, void *( * function )( void * ), void * arg ){
    pthread_attr_t thread_attr;
    int res;
    res = pthread_attr_init(&thread_attr);
    if (res != 0) {
        perror("Attribute creation failed");
        exit( 1 ); 
    }
    res = pthread_attr_setdetachstate( &thread_attr,PTHREAD_CREATE_DETACHED );
    if ( res != 0 ){
        perror("Setting detached attribute failed");
        exit( 1 );
    }
    res = pthread_create( threadfd, &thread_attr, function, arg );
    if (res !=0) {
        perror("Thread creation failed");
        exit( 1 );
    }
    pthread_attr_destroy(&thread_attr);
}
