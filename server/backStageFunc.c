/*************************************************************************
    > File Name: backStageFunc.c
    > Author: yaoyiyun
    > Mail: yiyunyao@sina.cn
    > Created Time: 2013年09月08日 星期日 21时00分57秒
 ************************************************************************/

#include"backStageHead.h"

void loginResponse( cJSON * function, char * sendBuffer ){
    User user;
    MYSQL * mysql = mysqlInit();

    printf( "%s\n", cJSON_Print( function ));
    cJSON * jsonresponse = cJSON_CreateObject();
    cJSON * jsonuser_info = cJSON_GetObjectItem( function, "user" );
    strcpy( user.ipaddress, cJSON_GetObjectItem( function, "ipaddress" )->valuestring );
    strcpy( user.username, cJSON_GetObjectItem( jsonuser_info, "username" )->valuestring );
    strcpy( user.password, cJSON_GetObjectItem( jsonuser_info, "password" )->valuestring );
    cJSON_AddNumberToObject( jsonresponse, "type", LOGIN_REQUEST );
    if( mysqlUserValidate( mysql, &user )){
        cJSON_AddNumberToObject( jsonresponse, "result", ON_SUCCESS );
        cJSON_AddItemToObject( jsonresponse, "user", jsonuser_info );
        cJSON_AddStringToObject( jsonuser_info, "nickname", user.nickname );
        cJSON_AddStringToObject( jsonuser_info, "signname", user.signname );
        cJSON_AddStringToObject( jsonuser_info, "ipaddress", user.ipaddress );
        mysqlUserLogin( mysql, &user );
    }
    else 
        cJSON_AddNumberToObject( jsonresponse, "result", ON_FAILURE );
    strcpy( sendBuffer, cJSON_Print( jsonresponse ) );
    mysql_close( mysql );
    free( jsonresponse );
    free( jsonuser_info );
}

void logoutResponse( cJSON * function, char * sendBuffer ){
    MYSQL * mysql = mysqlInit();
    printf( "%s\n", cJSON_Print(function));

    char username[ bufsize ];
    cJSON * jsonuser_info = cJSON_GetObjectItem( function, "user" );
    strcpy( username, cJSON_GetObjectItem( jsonuser_info, "username" )->valuestring );
    printf( "%s\n", username );
    mysqlUserLogout( mysql, username );
    mysql_close( mysql );
    free( jsonuser_info );
}

void getfriendResponse( cJSON * function, char * sendBuffer ){
    User * friend_list;
    unsigned int friend_list_len;
    MYSQL * mysql = mysqlInit();
    char username[ bufsize ];
    int i;
    cJSON * jsonuser_info = cJSON_GetObjectItem( function, "user" );
    strcpy( username, cJSON_GetObjectItem( jsonuser_info, "username" )->valuestring );
    printf( "%s\n", username );

    mysqlUserGetFriendList( mysql, username, &friend_list, &friend_list_len );
    cJSON * jsonresponse = cJSON_CreateObject();
    cJSON_AddNumberToObject( jsonresponse, "type", GETFRIEND_REQUEST );
    cJSON_AddNumberToObject( jsonresponse, "result", ON_SUCCESS );
    cJSON * jsonfriend_list = cJSON_CreateArray();
    cJSON_AddItemToObject( jsonresponse, "friend_ofl_list", jsonfriend_list );
    cJSON * jsonfriendol_list = cJSON_CreateArray();
    cJSON_AddItemToObject( jsonresponse, "friend_ol_list", jsonfriendol_list );
    cJSON * jsonfriend;
    char * t_ipaddress = ( char * )malloc( bufsize );
    for( i = 0; i < friend_list_len; ++i ){
        if( mysqlUserGetIp( mysql, friend_list[i].username, t_ipaddress ) ){
            cJSON_AddItemToArray( jsonfriendol_list, jsonfriend=cJSON_CreateArray());
            cJSON_AddItemToArray( jsonfriend, cJSON_CreateString( friend_list[i].username ));
            cJSON_AddItemToArray( jsonfriend, cJSON_CreateString( friend_list[i].nickname ));
            cJSON_AddItemToArray( jsonfriend, cJSON_CreateString( friend_list[i].signname ));
            cJSON_AddItemToArray( jsonfriend, cJSON_CreateString( t_ipaddress ));
        }
        else{
            cJSON_AddItemToArray( jsonfriend_list, jsonfriend=cJSON_CreateArray());
            cJSON_AddItemToArray( jsonfriend, cJSON_CreateString( friend_list[i].username ));
            cJSON_AddItemToArray( jsonfriend, cJSON_CreateString( friend_list[i].nickname ));
            cJSON_AddItemToArray( jsonfriend, cJSON_CreateString( friend_list[i].signname ));
        }
    }
    strcpy( sendBuffer, cJSON_Print( jsonresponse ) );
    printf("%s\n", sendBuffer );
    mysql_close( mysql );
    free( friend_list );
    free( jsonfriend_list );
    free( jsonfriendol_list );
    free( jsonfriend );
    free( jsonuser_info );
    free( jsonresponse );
}

void addfriendResponse( cJSON * function, char * sendBuffer ){ 
    MYSQL * mysql = mysqlInit();
    char username[ bufsize ];
    char friendname[ bufsize ];
    strcpy( username, cJSON_GetObjectItem( function , "username" )->valuestring );
    strcpy( friendname, cJSON_GetObjectItem( function , "friendname" )->valuestring );
    mysqlUserAddFriend( mysql, username, friendname );
    mysql_close( mysql );
}

void getipResponse( cJSON * function, char * sendBuffer ){
    MYSQL * mysql = mysqlInit();

    char username[ bufsize ];
    char userip[ bufsize ];
    strcpy( username, cJSON_GetObjectItem( function, "username" )->valuestring );
    mysqlUserGetIp( mysql, username, userip );
    cJSON * jsonresponse = cJSON_CreateObject();
    if( NULL == userip )
        cJSON_AddNumberToObject( jsonresponse, "result", ON_FAILURE );
    else{
        cJSON_AddNumberToObject( jsonresponse, "result", ON_SUCCESS );
        cJSON_AddStringToObject( jsonresponse, "userip", userip );
    }
    strcpy( sendBuffer, cJSON_Print( jsonresponse ) );
    mysql_close( mysql );
    free( jsonresponse );
}

void searchResponse( cJSON * function, char * sendBuffer ){
    MYSQL * mysql = mysqlInit();
    User * user = ( User * )malloc( sizeof( User ));
    cJSON * jsonresponse = cJSON_CreateObject();
    strcpy( user->username, cJSON_GetObjectItem( function, "username")->valuestring );
    mysqlUserGetInfo( mysql, user );
    if( NULL == user )
        cJSON_AddNumberToObject( jsonresponse, "result", ON_FAILURE );
    else{
        cJSON * jsonuser_info;
        cJSON_AddItemToObject( jsonresponse, "user_info", jsonuser_info = cJSON_CreateObject() );
        cJSON_AddNumberToObject( jsonresponse, "result", ON_SUCCESS );
        cJSON_AddStringToObject( jsonuser_info, "username", user->username );
        cJSON_AddStringToObject( jsonuser_info, "nickname", user->nickname );
        cJSON_AddStringToObject( jsonuser_info, "signname", user->signname );
        free( jsonuser_info );
    }
    printf( "%s\n", cJSON_Print( jsonresponse ));
    strcpy( sendBuffer, cJSON_Print( jsonresponse ));
    mysql_close( mysql );
    free( jsonresponse );
    free( user );
}

void imaliveResponse( cJSON * function, char * sendBuffer ){
    User user;
    unsigned int friend_list_len;
    MYSQL * mysql = mysqlInit();
    int i;
    cJSON * jsonuser_info = cJSON_GetObjectItem( function, "user" );
    strcpy( user.username, cJSON_GetObjectItem( jsonuser_info, "username" )->valuestring );

    mysqlUserLogin( mysql, &user );
    printf( "%s\n", user.username );
    mysql_close( mysql );
    free( jsonuser_info );
}

void savelogResponse( cJSON * function, char * sendBuffer ){
}

void addgroupResponse( cJSON * function, char * sendBuffer ){
}

void getgroupResponse( cJSON * function, char * sendBuffer ){
}

void updateResponse( cJSON * function, char * sendBuffer ){
}

void defaultResponse( cJSON * function, char * sendBuffer ){
    printf("%s\n", cJSON_Print( function ));
    strcpy( sendBuffer, cJSON_GetObjectItem( function, "message")->valuestring );
}

//void pushUpdate( char * username, char * pushIP ){
//    MYSQL * mysql = mysqlInit();
//    User user;
//    char send_buffer[ bufsize ];
//    char userip[ bufsize ];
//    strcpy( user.username, username );
//    mysqlUserGetInfo( mysql, &user );
//    mysql( mysql, username, userip );
//    cJSON * pushData = cJSON_CreateObject();
//    cJSON * friend = cJSON_CreateObject();
//    cJSON_AddNumberToObject( pushData, "type", 10 );
//    cJSON_AddItemToObject( pushData, "friend", friend );
//    cJSON_AddStringToObject( friend, "username", user.username );
//    cJSON_AddStringToObject( friend, "nickname", user.nickname );
//    cJSON_AddStringToObject( friend, "signname", user.signname );
//    cJSON_AddStringToObject( friend, "ipaddress", userip );
//    strcpy( send_buffer, cJSON_Print( pushData ) ); 
//    printf( "%s\n", send_buffer );
//    int 
