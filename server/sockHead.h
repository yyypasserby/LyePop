/*************************************************************************
    > File Name: net_head.h
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年08月26日 星期一 13时39分15秒
 ************************************************************************/

#include<stdbool.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#include<net/if.h>
#include<sys/ioctl.h>

#define ETH_INTERFACE_NAME "wlan0"

void createSocket( int * socketfd, int TYPE );
void zeroinAddr( struct sockaddr_in *addr );
void initMyAddr( struct sockaddr_in * addr, const int myport );
void initHostAddr( struct sockaddr_in * addr, const int myport );
void initAddr( struct sockaddr_in * addr, char * ipaddress, const int myport );
void bindPort( int *socketfd, struct sockaddr_in *addr);
void listenPort( int *socketfd );
void getAccept( int *socketfd, int *connect_socketfd, struct sockaddr_in *cli_addr, int *cli_addr_size );
int recvPackage( int *connect_socketfd, void * buf, int bufsize );
void getConnect( int *socketfd, struct sockaddr_in * pin_addr);
void sendMessage( int * send_socket, void * buf, int bufsize ); 
char * gethostIP( char * addr );
