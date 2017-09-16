#include <gdk/gdkcursor.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <libnotify/notify.h>
#include <sys/msg.h>
#include "../../lib/pthread_utils.h"
#include "../../lib/socket_utils.h"
#include "third_party/cJSON/cJSON.h"

#define serverport 5599
#define listenport 4488
#define bufsize 4096

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

#define CHAT_MESSAGE 0
#define FUNC_MESSAGE 1
#define FILE_MESSAGE 2
#define PIC_MESSAGE 3

int connectfd;
int listenfd;

struct sockaddr_in server_addr;
struct sockaddr_in any_addr;

char current_user_info[bufsize];
char user_pid[bufsize * 10];

GtkWidget* name_input;
GtkWidget* pwd_input;
GtkWidget* login_window;
GtkWidget* user_box;

GtkWidget* friendListWindow;
GtkWidget* listOnlineFriendTreeView;  // friendlist
GtkWidget* listOfflineFriendTreeView;

pthread_t server_recv_thread;
pthread_t login_thread;
pthread_t logout_thread;
pthread_t get_friend_thread;
pthread_t update_thread;
pthread_t search_thread;
pthread_t listen_thread;

void* serverRecvThread(void* arg);
void* loginThread(void* arg);
void* logoutThread(void* arg);
void* getfriendThread(void* arg);
void* updateThread(void* arg);
void* searchThread(void* arg);
void* listenThread(void* arg);
void createPidThreadNotShow(void* arg);
void updateFriendlist(cJSON* _friend_list, GtkWidget* _friend_list_tree_view,
                      int type);
void createFriendWindow();
void createLoginWindow();
void addUserPidToQueue(char* username, pid_t pid);
void notifyMessage(char* title, char* content);
void pushToPid(int pid, char* message);

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("You must take an IP address.\n");
    exit(1);
  }
  gtk_init(&argc, &argv);
  createSocket(&connectfd, 0);
  initAddr(&server_addr, argv[1], serverport);
  getConnect(&connectfd, &server_addr);

  if (!g_thread_supported()) g_thread_init(NULL);
  gdk_threads_init();

  gdk_threads_enter();

  createLoginWindow();
  createDetachPthread(&server_recv_thread, serverRecvThread, NULL);
  gtk_main();
  gdk_threads_leave();
  return 0;
}

void* serverRecvThread(void* arg) {
  char recv_buffer[bufsize];
  cJSON* json_res;
  int type, ret;
  while (1) {
    if ((ret = recvPackage(&connectfd, recv_buffer, bufsize)) == 0) continue;
    printf("%s\n", recv_buffer);
    json_res = cJSON_Parse(recv_buffer);
    type = cJSON_GetObjectItem(json_res, "type")->valueint;
    switch (type) {
      case LOGIN_REQUEST:
        createDetachPthread(&login_thread, loginThread, json_res);
        break;
      case GETFRIEND_REQUEST:
        createDetachPthread(&get_friend_thread, getfriendThread, json_res);
        break;
      case SEARCH_REQUEST:
        createDetachPthread(&search_thread, searchThread, json_res);
        break;
    }
    sleep(1);
  }
  free(json_res);
  pthread_exit(NULL);
}

void* searchThread(void* arg) {}
void* loginThread(void* arg) {
  cJSON* json_res = (cJSON*)arg;
  cJSON* function = cJSON_CreateObject();
  char send_buffer[bufsize];
  cJSON* user_info;
  int result = cJSON_GetObjectItem(json_res, "result")->valueint;
  if (result == ON_SUCCESS) {
    notifyMessage("Success!", "\nYou have logged in successfully!");
    user_info = cJSON_GetObjectItem(json_res, "user");
    cJSON_AddNumberToObject(function, "header", GETFRIEND_REQUEST);
    cJSON_AddItemToObject(function, "user", user_info);
    strcpy(send_buffer, cJSON_Print(function));
    sendMessage(&connectfd, send_buffer, strlen(send_buffer) + 1);
    strcpy(current_user_info, cJSON_Print(user_info));
    printf("%s\n", current_user_info);

    gdk_threads_enter();
    gtk_widget_destroy(login_window);
    createFriendWindow();
    create_background_window();
    create_main_window();
    gdk_threads_leave();
    createDetachPthread(&listen_thread, listenThread, NULL);
  } else {
    notifyMessage("Failed...", "\nI guess you may miss something.");
    gdk_threads_enter();
    gtk_widget_destroy(login_window);
    createLoginWindow();
    gdk_threads_leave();
  }
  pthread_exit(NULL);
}

void* listenThread(void* arg) {
  createSocket(&listenfd, 1);
  initMyAddr(&any_addr, listenport);
  bindPort(&listenfd, &any_addr);
  char recv_buffer[bufsize];
  int ret, current_pid_len, i, pid;
  char from_user[bufsize];
  cJSON *json_message, *json_user, *json_user_pid;
  pthread_t createpid_thread;
  while (1) {
    ret = recvPackageFrom(&listenfd, recv_buffer, &any_addr, bufsize);
    // if( ret <= 0 )continue;
    json_message = cJSON_Parse(recv_buffer);
    if (FUNC_MESSAGE == cJSON_GetObjectItem(json_message, "header")->valueint) {
      cJSON* json_friend = cJSON_GetObjectItem(json_message, "friend");
      char _title[bufsize], _content[bufsize];
      sprintf(_title, "%s!",
              cJSON_GetObjectItem(json_friend, "username")->valuestring);
      sprintf(_content, "\nYour friend %s is online now!",
              cJSON_GetObjectItem(json_friend, "username")->valuestring);
      notifyMessage(_title, _content);
      gdk_threads_enter();
      add_to_friend_list(json_friend);
      gdk_threads_leave();
      continue;
    }
    strcpy(from_user,
           cJSON_GetObjectItem(json_message, "username")->valuestring);
    json_user_pid = cJSON_Parse(user_pid);
    current_pid_len = cJSON_GetArraySize(json_user_pid);
    for (i = 0; i < current_pid_len; ++i) {
      json_user = cJSON_GetArrayItem(json_user_pid, i);
      if (strcmp((cJSON_GetObjectItem(json_user, "destname")->valuestring),
                 from_user) == 0) {
        pid = cJSON_GetObjectItem(json_user, "pid")->valueint;
        printf("pid:%d is get this message!\n", pid);
        char* sendtime =
            cJSON_GetObjectItem(json_message, "sendtime")->valuestring;
        char* content =
            cJSON_GetObjectItem(json_message, "content")->valuestring;
        printf("message : %s\n", content);
        pushToPid(pid, recv_buffer);
        break;
      }
    }
    if (i == current_pid_len) {
      char from_user_ip[bufsize];
      strcpy(from_user_ip, inet_ntoa(any_addr.sin_addr));
      printf("%s\n", from_user_ip);
      cJSON* json_user_info = cJSON_CreateObject();
      cJSON_AddStringToObject(
          json_user_info, "username",
          cJSON_GetObjectItem(cJSON_Parse(current_user_info), "username")
              ->valuestring);
      cJSON_AddStringToObject(json_user_info, "destname", from_user);
      cJSON_AddStringToObject(json_user_info, "ipaddress", from_user_ip);
      createDetachPthread(&createpid_thread, createPidThreadNotShow,
                          (void*)json_user_info);
    }
  }
  pthread_exit(NULL);
}

void pushToPid(int pid, char* message) {
  int msgid = msgget(pid, 0666 | IPC_CREAT);
  msgsnd(msgid, message, bufsize, 0);
}

void createPidThreadNotShow(void* arg) {
  int ret;
  char username[bufsize], destname[bufsize], ipaddress[bufsize];
  cJSON* json_user_info = (cJSON*)arg;
  strcpy(username,
         cJSON_GetObjectItem(json_user_info, "username")->valuestring);
  strcpy(destname,
         cJSON_GetObjectItem(json_user_info, "destname")->valuestring);
  strcpy(ipaddress,
         cJSON_GetObjectItem(json_user_info, "ipaddress")->valuestring);
  printf("this is pid not showed:%s\n", cJSON_Print(json_user_info));
  pid_t pid = fork();
  if (pid > 0) {
    addUserPidToQueue(destname, pid);
    printf("This is lyepop's main process .its child process with pid %d\n",
           pid);
  } else {
    ret = execlp("./chat", "./chat", username, destname, ipaddress, "0", NULL);
    if (ret < 0) {
      printf("error\n");
    }
  }
  pthread_exit(NULL);
}

void addUserPidToQueue(char* username, pid_t pid) {
  cJSON* json_user = cJSON_CreateObject();
  cJSON* json_user_pid = cJSON_Parse(user_pid);
  cJSON_AddItemToArray(json_user_pid, json_user);
  cJSON_AddStringToObject(json_user, "destname", username);
  cJSON_AddNumberToObject(json_user, "pid", pid);
  strcpy(user_pid, cJSON_Print(json_user_pid));
}

void* logoutThread(void* arg) {
  char send_buffer[bufsize];
  cJSON* function = cJSON_CreateObject();
  cJSON* user_info = cJSON_Parse(current_user_info);
  cJSON_AddNumberToObject(function, "header", LOGOUT_REQUEST);
  cJSON_AddItemToObject(function, "user", user_info);
  strcpy(send_buffer, cJSON_Print(function));
  sendMessage(&connectfd, send_buffer, strlen(send_buffer) + 1);
  int i, current_pid_len;
  pid_t pid;
  cJSON *json_pid, *json_user_pid;
  json_user_pid = cJSON_Parse(user_pid);
  current_pid_len = cJSON_GetArraySize(json_user_pid);
  for (i = 0; i < current_pid_len; ++i) {
    json_pid = cJSON_GetArrayItem(json_user_pid, i);
    pid = cJSON_GetObjectItem(json_pid, "pid")->valueint;
    printf("%d is killed!\n", pid);
    kill(pid, SIGKILL);
  }
  close(connectfd);
  close(listenfd);
  gtk_main_quit();
  pthread_exit(NULL);
}

void* getfriendThread(void* arg) {
  cJSON* json_res = (cJSON*)arg;
  cJSON *friend_ofl_list, *friend_ol_list;
  strcpy(user_pid, cJSON_Print(cJSON_CreateArray()));
  int i, sendFuncfd;
  struct sockaddr_in dest_addr;
  int result = cJSON_GetObjectItem(json_res, "result")->valueint;
  if (result == ON_SUCCESS) {
    friend_ofl_list = cJSON_GetObjectItem(json_res, "friend_ofl_list");
    friend_ol_list = cJSON_GetObjectItem(json_res, "friend_ol_list");
    int friend_ol_list_len = cJSON_GetArraySize(friend_ol_list);
    char _title[bufsize], _content[bufsize];
    sprintf(_title, "%d Friends!", friend_ol_list_len);
    sprintf(_content, "\nYou have %d friends Online.", friend_ol_list_len);
    notifyMessage(_title, _content);
    cJSON *json_friend, *function;
    char send_buffer[bufsize];
    createSocket(&sendFuncfd, 1);
    for (i = 0; i < friend_ol_list_len; ++i) {
      json_friend = cJSON_GetArrayItem(friend_ol_list, i);
      initAddr(&dest_addr, cJSON_GetArrayItem(json_friend, 3)->valuestring,
               listenport);
      printf("%s\n", current_user_info);
      function = cJSON_CreateObject();
      cJSON_AddNumberToObject(function, "header", FUNC_MESSAGE);
      cJSON_AddItemToObject(function, "friend", cJSON_Parse(current_user_info));
      strcpy(send_buffer, cJSON_Print(function));
      printf("%s\n", send_buffer);
      sendMessageTo(&sendFuncfd, send_buffer, &dest_addr,
                    strlen(send_buffer) + 1);
    }
  } else {
    notifyMessage("Failed...", "I guess you may get some net trouble.\n");
  }
  gdk_threads_enter();
  updateFriendList(friend_ol_list, listOnlineFriendTreeView, 0);
  updateFriendList(friend_ofl_list, listOfflineFriendTreeView, 1);
  gdk_threads_leave();
  pthread_exit(NULL);
}

void* updateThread(void* arg) {
  cJSON* json_res = cJSON_Parse((char*)arg);
  cJSON* json_friend = cJSON_GetObjectItem(json_res, "friend");
  char _title[bufsize], _content[bufsize];
  sprintf(_title, "%s!",
          cJSON_GetObjectItem(json_friend, "username")->valuestring);
  sprintf(_content, "\nYour friend %s is online now!",
          cJSON_GetObjectItem(json_friend, "username")->valuestring);
  notifyMessage(_title, _content);
  gdk_threads_enter();
  // add_to_list( json_friend );
  gdk_threads_leave();
  pthread_exit(NULL);
}

gboolean onMove(GtkWidget* widget, GdkEventButton* event, GdkWindowEdge edge) {
  if (event->type == GDK_BUTTON_PRESS) {
    if (event->button == 1)
      gtk_window_begin_move_drag(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                 event->button, event->x_root, event->y_root,
                                 event->time);
  }
  return FALSE;
}

void notifyMessage(char* title, char* content) {
  NotifyNotification* n;
  notify_init("Basics");
  n = notify_notification_new(title, content, NULL);
  notify_notification_set_timeout(n, 3000);  // 5 seconds
  if (!notify_notification_show(n, NULL))
    perror("notification system launched failed!\n");
  g_object_unref(G_OBJECT(n));
}

void onDestroy(GtkWidget* Widget, gpointer data) {
  gtk_widget_destroy(data);
  createDetachPthread(&logout_thread, logoutThread, NULL);
}
