/*************************************************************************
    > File Name: server.c
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年09月02日 星期一 14时21分43秒
 ************************************************************************/

#include "backStageHead.h"

#define myport 5599
#define bufsize 4096

typedef struct addr_arg{
    int connectfd;
    struct sockaddr_in cli_addr;
}ADDR_ARG;

void * start_talk( void * arg);
void pthread_talk( int connectfd, struct sockaddr_in cli_addr );
int main(int argc, char* argv[]){

    int listenfd, connectfd;
    struct sockaddr_in host_addr, pin_addr;
    int pin_addr_size;
    pthread_t multiCS;
    ADDR_ARG * arg = NULL;

    createSocket( &listenfd, 0 );
    initHostAddr( &host_addr, myport );

    printf(" init ipaddress: %s\n", inet_ntoa( host_addr.sin_addr ) );
    bindPort( &listenfd, &host_addr );
    listenPort( &listenfd );

    pin_addr_size = sizeof( pin_addr ); //THIS IS A HOLE!
    while(1){
        getAccept( &listenfd, &connectfd, &pin_addr, &pin_addr_size );

        arg = (ADDR_ARG *) malloc( sizeof( ADDR_ARG ) );
        arg->connectfd = connectfd;
        memcpy( (void *)&arg->cli_addr, &pin_addr, sizeof(pin_addr) );

        createPthread( &multiCS, start_talk, arg );
    }
    close(listenfd);
    return 0;
}

void * start_talk( void * arg){
    ADDR_ARG * info = (ADDR_ARG *)arg;
    pthread_talk( info->connectfd, info->cli_addr );

    free( info );
    pthread_exit(NULL);
}

void pthread_talk( int connectfd, struct sockaddr_in cli_addr ){

    char sendBuffer[ bufsize ];
    char recvBuffer[ bufsize ];


    char _cli_addr[bufsize];
    strcpy( _cli_addr, inet_ntoa( cli_addr.sin_addr ) );
    int _cli_port = ntohs( cli_addr.sin_port );

    int ret;

    printf("client_IP:%s:%d\n", _cli_addr, _cli_port );

    while(1){
        ret = recvPackage( &connectfd, recvBuffer, bufsize );
        if( ret != 0){
            cJSON * function = cJSON_Parse( recvBuffer );
            int header  = cJSON_GetObjectItem( function, "header" )->valueint;
            if( header == 0 ) cJSON_AddStringToObject( function, "ipaddress", _cli_addr );
            printf( "%d\n", header );
            switch( header ){
                case LOGIN_REQUEST:
                    loginResponse( function, sendBuffer );
                    break;
                case LOGOUT_REQUEST:
                    logoutResponse( function, sendBuffer );
                    break;
                case GETFRIEND_REQUEST:
                    getfriendResponse( function, sendBuffer );
                    break;
                case ADDFRIEND_REQUEST:
                    addfriendResponse( function, sendBuffer );
                    break;
                case GETIP_REQUEST:
                    getipResponse( function, sendBuffer );
                    break;
                case GETGROUP_REQUEST:
                    getgroupResponse( function, sendBuffer );
                    break;
                case ADDGROUP_REQUEST:
                    addgroupResponse( function, sendBuffer );
                    break;
                case SEARCH_REQUEST:
                    searchResponse( function, sendBuffer );
                    break;
                case SAVELOG_REQUEST:
                    savelogResponse( function, sendBuffer );
                    break;
                case UPDATE_REQUEST:
                    updateResponse( function, sendBuffer );
                    break;
                case IMALIVE_REQUEST:
                    imaliveResponse( function, sendBuffer );
                    break;
                default:
                    defaultResponse( function, sendBuffer );
                    break;
            }        
            sendMessage( &connectfd, sendBuffer, strlen(sendBuffer)+1);
        }
    }
    close( connectfd );
}






