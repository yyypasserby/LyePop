/*************************************************************************
    > File Name: test1.c
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年09月05日 星期四 12时59分33秒
 ************************************************************************/
#include "mysqlHead.h"
#define bufsize 4096

int main( int argc, char * argv[] ){

    int i;
    if( argc < 3 )exit(1);
    MYSQL * mysql = mysqlInit();
    unsigned int friend_list_len;
    User * friend_list;
    mysqlUserGetFriendList( mysql, argv[1], &friend_list, &friend_list_len );
    printf("%p\n", friend_list);
    for( i = 0; i < friend_list_len; ++i )
        printf( "%s->nickname:%s signname: %s\n", friend_list[i].username, friend_list[i].nickname, friend_list[i].signname );
    free( friend_list );
    mysqlUserAddFriend( mysql, argv[1], argv[2]);
    mysqlUserGetFriendList( mysql, argv[1], &friend_list, &friend_list_len );
    for( i = 0; i < friend_list_len; ++i )
        printf( "%s->nickname:%s signname: %s\n", friend_list[i].username, friend_list[i].nickname, friend_list[i].signname );
    free( friend_list );
    mysql_close( mysql );
    return 0;
}
