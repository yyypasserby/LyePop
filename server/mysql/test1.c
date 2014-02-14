/*************************************************************************
    > File Name: test1.c
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年09月06日 星期五 10时44分37秒
 ************************************************************************/

#include<stdio.h>
#include "mysqlHead.h"

int main( int argc, char * argv[] ){
    if( argc < 3 )exit(1);
    MYSQL * mysql = mysqlInit();
    User * myuser = ( User * )malloc( sizeof( User ) );
    strcpy( myuser->username, argv[1] );
    strcpy( myuser->password, argv[2] );
    if( mysqlUserValidate( mysql, myuser ))
        printf(" success! \n");
    else
        printf(" failed! \n ");
    return 0;
}
