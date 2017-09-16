#include <mysql/mysql.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bufsize 4096

typedef struct {
  char username[40];
  char password[40];
  char nickname[40];
  char signname[260];
  char ipaddress[40];
} User;

MYSQL* mysqlInit();
MYSQL_RES* mysqlQuery(MYSQL* mysql, const char* query);
bool mysqlIsEmpty(MYSQL_RES* res);
bool mysqlUserValidate(MYSQL* mysql, User* user);
void mysqlUserChangePWD(MYSQL* myqsl, User* user);
void mysqlUserGetFriendList(MYSQL* mysql, char* username, User** friend_list,
                            unsigned int* friend_list_len);
void mysqlUserGetInfo(MYSQL* mysql, User* user);
void mysqlUserGetInfoById(MYSQL* mysql, char* id, User* user);
bool mysqlUserGetIp(MYSQL* mysql, char* username, char* ip);
void mysqlUserGetId(MYSQL* mysql, char* username, char* userid);
void mysqlUserSetInfo(MYSQL* mysql, User* user);
void mysqlUserAddFriend(MYSQL* mysql, char* username, char* friendname);
void mysqlUserLogout(MYSQL* mysql, char* username);
void mysqlUserLogin(MYSQL* mysql, User* user);
