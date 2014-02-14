/*************************************************************************
    > File Name: sockFunc.c
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年08月26日 星期一 22时21分35秒
 ************************************************************************/

#include "sockHead.h"

void createSocket( int *socketfd, int TYPE ){
    if( TYPE == 0 )
        *socketfd = socket( AF_INET, SOCK_STREAM, 0);
    else
        *socketfd = socket( AF_INET, SOCK_DGRAM, 0);

    if( *socketfd < 0 ){
        perror("Cant create socket.\n");
        exit(1);
    }
    else{
        if( TYPE == 0 )
            printf( "create TCP socket successfully!\n");
        else
            printf( "create UDP socket successfully!\n");
    }
    int opt = SO_REUSEADDR;
    if(setsockopt(*socketfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsocketopt failed!\n");
        exit(1);
    }
    else
        printf("socket port reused set!\n");
}

void zeroinAddr(struct sockaddr_in *addr){
    memset(addr, 0, sizeof(*addr));
}

char * getHostIP( char * addr ){
    int sock_fd;
    struct ifreq ifr;
    if( ( sock_fd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 ){
        perror( "Create socket failed" );
        exit( 1 );
    }
    strncpy( ifr.ifr_name, ETH_INTERFACE_NAME, IF_NAMESIZE );
    ifr.ifr_name[ IF_NAMESIZE - 1 ] = '\0';
    if( ioctl( sock_fd, SIOCGIFADDR, &ifr ) < 0 ){
        perror( "Get interface address failed" );
        exit( 1 );
    }
    addr = inet_ntoa( ( ( struct sockaddr_in* )( &ifr.ifr_addr ) )->sin_addr );
    printf("IP address is %s\n", addr);
    return addr;
}

void initMyAddr(struct sockaddr_in *addr, const int myport){
    zeroinAddr(addr);
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl( INADDR_ANY );
    addr->sin_port = htons(myport);
}

void initAddr( struct sockaddr_in * addr, char * ipaddress, const int myport ){
    zeroinAddr( addr );
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = inet_addr( ipaddress );
    addr->sin_port = htons( myport );
}

void initHostAddr( struct sockaddr_in * addr, const int myport ){
    zeroinAddr( addr );
    char hostIP[ 40 ];
    strcpy( hostIP, getHostIP( hostIP ) );
    printf( "%s\n", hostIP );
    initAddr( addr, hostIP, myport );
}

void bindPort( int *socketfd, struct sockaddr_in * addr ){
    if( bind( *socketfd, ( struct sockaddr * )addr, sizeof( *addr ) ) < 0 ){
        perror("Cant bind socket!\n");
        exit(1);
    }
    else
        printf("bind %s : %d successfully!\n", inet_ntoa( addr->sin_addr ), ntohs( addr->sin_port));
}

void listenPort( int *socketfd){
    if(listen(*socketfd, 5 ) < 0){
        perror("listen error!\n");
        exit(1);
    }
}

void getAccept( int *socketfd, int *connect_socketfd, struct sockaddr_in *cli_addr, int *cli_addr_size ){
    if((*connect_socketfd = accept(*socketfd, (struct sockaddr *)cli_addr, (socklen_t *) cli_addr_size)) < 0 ){
        perror("accept error!\n");
        exit(1);
    }
    //char cli_IP[20] = inet_ntoa( cli_sockIP );
    printf("client_IP:%s:%d\n", inet_ntoa( cli_addr->sin_addr ), ntohs( cli_addr->sin_port ));
}

void getConnect( int * send_socketfd, struct sockaddr_in *pin_addr){
    if(connect(*send_socketfd,(struct sockaddr *) pin_addr, sizeof(*pin_addr))==-1){
        perror("connect error!\n");
        exit(1);
    }
}

int recvPackage( int *connect_socketfd, void *buf, int bufsize ){
    int ret;
    if( (ret = recv(*connect_socketfd, buf, bufsize, 0)) == -1){
        perror("cant recv packages!\n");
        exit(1);
    }
    sleep(1);
    return ret;
}

void sendMessage(int *send_socketfd, void * buf, int bufsize ) {
    if(send(*send_socketfd, buf, bufsize, MSG_DONTROUTE)==-1){
        perror("send error!\n");
        exit(1);
    }
    sleep(1);
}

int recvPackageFrom( int * connectfd, void * buf, struct sockaddr_in * send_addr, int bufsize ){
    int ret;
    int addr_len = sizeof( struct sockaddr_in );
    if((ret = recvfrom( *connectfd, buf, bufsize, 0, (struct sockaddr *)send_addr, &addr_len )) == -1 ){
        perror("can't recv from other client!\n");
    }
    printf("recieve from %s:%d : %s\n", inet_ntoa( send_addr->sin_addr ), ntohs( send_addr->sin_port ), (char * )buf );
    sleep(1);
}


void sendMessageTo( int * connectfd, void * buf, struct sockaddr_in * dest_addr, int bufsize ){
    int addr_len = sizeof( struct sockaddr_in );
    if( sendto( *connectfd, buf, bufsize, 0, (struct sockaddr *)dest_addr, addr_len) == -1 ){
        perror("can't send to other client!\n");
    }
    sleep(1);
}
