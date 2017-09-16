#include "../../lib/socket_utils.h"
#include "../../lib/third_party/cJSON.h"
#include "mysql_utils.h"

#define LOGIN_REQUEST 0
#define LOGOUT_REQUEST 1
#define GETFRIEND_REQUEST 2
#define ADDFRIEND_REQUEST 3
#define GETIP_REQUEST 4
#define GETGROUP_REQUEST 5
#define ADDGROUP_REQUEST 6
#define SEARCH_REQUEST 7
#define SAVELOG_REQUEST 8
#define IMALIVE_REQUEST 9
#define UPDATE_REQUEST 10

#define ON_SUCCESS 0
#define ON_FAILURE 1

void loginResponse(cJSON* function, char* sendBuffer);
void logoutResponse(cJSON* function, char* sendBuffer);
void getfriendResponse(cJSON* function, char* sendBuffer);
void addfriendResponse(cJSON* function, char* sendBuffer);
void getipResponse(cJSON* function, char* sendBuffer);
void searchResponse(cJSON* function, char* sendBuffer);
void savelogResponse(cJSON* function, char* sendBuffer);
void addgroupResponse(cJSON* function, char* sendBuffer);
void getgroupResponse(cJSON* function, char* sendBuffer);
void imaliveResponse(cJSON* function, char* sendBuffer);
void updateResponse(cJSON* function, char* sendBuffer);
void defaultResponse(cJSON* function, char* sendBuffer);
