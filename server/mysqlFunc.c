/*************************************************************************
  > File Name: mysqlFunc.c
  > Author: yaoyiyun
  > Mail: yiyunyao@sina.cn
  > Created Time: 2013年09月05日 星期四 15时25分38秒
 ************************************************************************/

#include "mysqlHead.h"
MYSQL * mysqlInit(){
    MYSQL * mysql = mysql_init(NULL);
    if(mysql_real_connect( mysql, "localhost", "root", "root", "linpop", 0, NULL, 0 ) == NULL) {
        fprintf(stderr, "Error connecting to database: %s\n", mysql_error( mysql ));
        exit(1);
    }
    return mysql;
}

bool mysqlUserValidate( MYSQL * mysql, User * user ){
    MYSQL_RES * res;
    MYSQL_ROW user_attr;
    char query[ bufsize ];
    sprintf( query, "SELECT nickname,signname FROM user WHERE username='%s' AND password='%s'", user->username, user->password );
    res = mysqlQuery( mysql, query );
    if ( mysqlIsEmpty( res ) ){
        printf("login failed\n");
        mysql_free_result( res );
        return false;
    }
    else{
        printf("login success!\n");
        if(( user_attr = mysql_fetch_row( res )) == NULL){
            user = NULL;
            return;
        }
        strcpy( user->nickname, user_attr[0] );
        strcpy( user->signname, user_attr[1] );
        mysql_free_result( res );
        return true;
    }
}

bool mysqlUserGetIp( MYSQL * mysql, char * username, char * ip ){
    char query[ bufsize ];
    MYSQL_RES * res;
    MYSQL_ROW user_attr;
    sprintf( query, "SELECT ipaddress FROM userol WHERE username='%s'", username );
    res = mysqlQuery( mysql, query );
    if( ( user_attr = mysql_fetch_row( res ) ) == NULL ){
        printf( "%s is not online!\n", username );
        ip = NULL;
        return false;
    }
    strcpy( ip, user_attr[0] );
    return true;
}


void mysqlUserLogout( MYSQL * mysql, char * username ){
    char query[ bufsize ];
    sprintf( query, "DELETE FROM userol WHERE username='%s'", username );
    mysqlQuery( mysql, query );
}

void mysqlUserLogin( MYSQL * mysql, User * user ){
    char query[ bufsize ];
    sprintf( query, "REPLACE userol SET username='%s', ipaddress='%s'", user->username, user->ipaddress );
    mysqlQuery( mysql, query );
}


void mysqlUserSetInfo( MYSQL * mysql, User * user ){
    char query[ bufsize ];
    sprintf( query, "UPDATE user SET nickname='%s', signname='%s' WHERE username='%s'", user->nickname, user->signname, user->username );
    mysqlQuery( mysql, query );
}

void mysqlUserGetInfo( MYSQL * mysql, User * user ){
    MYSQL_RES *res;
    MYSQL_ROW user_attr;
    char query[ bufsize ];
    sprintf( query, "SELECT nickname,signname FROM user WHERE username='%s'", user->username );
    res = mysqlQuery( mysql, query );
    if(( user_attr = mysql_fetch_row( res )) == NULL){
        user = NULL;
        return;
    }
    strcpy( user->nickname, user_attr[0] );
    strcpy( user->signname, user_attr[1] );
    mysql_free_result( res );
}

void mysqlUserGetInfoById( MYSQL * mysql, char * id, User * user){
    MYSQL_RES *res;
    MYSQL_ROW user_attr;
    char query[ bufsize ];
    sprintf( query, "SELECT username, nickname, signname FROM user WHERE id='%s'", id );
    res = mysqlQuery( mysql, query );
    if(( user_attr = mysql_fetch_row( res )) == NULL){
        user = NULL;
        return;
    }
    strcpy( user->username, user_attr[0] );
    strcpy( user->nickname, user_attr[1] );
    strcpy( user->signname, user_attr[2] );
    mysql_free_result( res );
}

void mysqlUserGetId( MYSQL * mysql, char * username, char * userid ){
    MYSQL_RES * res;
    MYSQL_ROW user_id;
    char query[ bufsize ];
    sprintf( query, "SELECT id FROM user WHERE username='%s'", username );
    res = mysqlQuery( mysql, query );
    if( (user_id = mysql_fetch_row( res )) == NULL ){
        userid = NULL;
        return;
    }
    strcpy( userid, user_id[0] );
}

void mysqlUserGetFriendList( MYSQL * mysql, char * username, User ** friend_list, unsigned int * friend_list_len ){
    printf("%p\n", *friend_list);
    MYSQL_RES * res;
    MYSQL_ROW user_friends_id;
    char userid[10];
    char query[ bufsize ];
    int i;
    mysqlUserGetId( mysql, username, userid ); 
    sprintf( query, "SELECT to_user_id FROM user_friends WHERE from_user_id='%s'", userid );
    res = mysqlQuery( mysql, query );
    *friend_list_len = mysql_num_rows( res );
    printf( "you have %d friends!\n", *friend_list_len );
    User * _friend_list = (User *)malloc( sizeof( User ) * (*friend_list_len));
    printf("%p\n", _friend_list);
    User t_user;
    for( i= 0; i < *friend_list_len; ++i){
        if( (user_friends_id = mysql_fetch_row( res )) == NULL ){
            break;
        }
        mysqlUserGetInfoById( mysql, user_friends_id[0], &t_user );

        strcpy( _friend_list[i].username , t_user.username);
        printf( "%s\n", _friend_list[i].username );
        strcpy( _friend_list[i].nickname , t_user.nickname);
        strcpy( _friend_list[i].signname , t_user.signname);
    }
    *friend_list = _friend_list;
    printf("%p\n", *friend_list);
    mysql_free_result( res );
}

void mysqlUserAddFriend( MYSQL * mysql, char * username, char * friendname ){
    char query[ bufsize ];
    char userid[ 10 ];
    char friendid[ 10 ];
    mysqlUserGetId( mysql, username, userid );
    mysqlUserGetId( mysql, friendname, friendid );
    sprintf( query, "REPLACE INTO user_friends( from_user_id, to_user_id ) values ('%s', '%s')", userid, friendid );
    mysqlQuery( mysql, query );
}

void mysqlUserChangePWD( MYSQL * mysql, User * user ){
    char query[ bufsize ];
    sprintf( query, "UPDATE user SET password='%s' WHERE username='%s'", user->password, user->username );
    mysqlQuery( mysql, query );
}

MYSQL_RES * mysqlQuery( MYSQL * mysql,  const char * query ){
    MYSQL_RES * res = NULL;
    printf( "%s\n", query);
    if( 0 != mysql_query( mysql, query )){
        fprintf( stderr, "%s\n", mysql_error( mysql ));
        return res;
    }
    res = mysql_store_result( mysql );
    return res;
}

bool mysqlIsEmpty( MYSQL_RES * res ){
    int num_row = mysql_num_rows( res );
    int num_col = mysql_num_fields( res );
    printf( "rows:%d cols:%d\n", num_row, num_col );
    if( num_row == 0 )
        return true;
    else
        return false;
}
