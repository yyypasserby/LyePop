#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "../../lib/pthread_utils.h"
#include "../../lib/socket_utils.h"
#include "third_party/cJSON/cJSON.h"

#define CHAT_MESSAGE 0
#define FUNC_MESSAGE 1
#define FILE_MESSAGE 2
#define PIC_MESSAGE 3

#define bufsize 4096

enum {
  ICON_COLUMN,
  NAME_COLUMN,
  EXTRA_COLUMN,
  NUM_COLUMN,
};

extern char current_user_info[bufsize];
extern char user_pid[bufsize];
extern GtkWidget* friendListWindow;
extern GtkWidget* listOnlineFriendTreeView;  // friendlist
extern GtkWidget* listOfflineFriendTreeView;

void onDestroy(GtkWidget* object, gpointer data);
void createPidThreadShow(void* arg);

void addToFriendList(cJSON* json_friend) {
  GtkTreeModel* model;
  GtkTreeIter iter;
  char _username[bufsize], _ipaddress[bufsize];
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOnlineFriendTreeView));
  gtk_list_store_append(GTK_LIST_STORE(model), &iter);
  strcpy(_username, cJSON_GetObjectItem(json_friend, "username")->valuestring);
  strcpy(_ipaddress,
         cJSON_GetObjectItem(json_friend, "ipaddress")->valuestring);
  gchar* icon_name = g_strdup_printf("%s.png", _username);
  GdkPixbuf* _icon = gdk_pixbuf_new_from_file(icon_name, NULL);
  printf("%s : %s", _username, _ipaddress);
  gtk_list_store_set(GTK_LIST_STORE(model), &iter, ICON_COLUMN, _icon,
                     NAME_COLUMN, _username, EXTRA_COLUMN, _ipaddress, -1);
  gtk_tree_view_set_model(GTK_TREE_VIEW(listOnlineFriendTreeView), model);
}

void listBeginChat(GtkTreeView* treeview, GtkTreePath* path,
                   GtkTreeSelection* selection) {
  g_print("A row has been double-clicked!\n");
  GtkTreeModel* model;
  GtkTreeIter iter;
  char* from_user;
  char* from_user_ip;
  model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
  gtk_tree_model_get_iter(model, &iter, path);
  gtk_tree_model_get(model, &iter, NAME_COLUMN, &from_user, -1);
  gtk_tree_model_get(model, &iter, EXTRA_COLUMN, &from_user_ip, -1);
  int current_pid_len, i, pid;
  cJSON *json_user_pid, *json_user, *json_function;
  json_user_pid = cJSON_Parse(user_pid);
  current_pid_len = cJSON_GetArraySize(json_user_pid);
  for (i = 0; i < current_pid_len; ++i) {
    json_user = cJSON_GetArrayItem(json_user_pid, i);
    if (strcmp((cJSON_GetObjectItem(json_user, "destname")->valuestring),
               from_user) == 0) {
      pid = cJSON_GetObjectItem(json_user, "pid")->valueint;
      json_function = cJSON_CreateObject();
      cJSON_AddNumberToObject(json_function, "header", FUNC_MESSAGE);
      pushToPid(pid, cJSON_Print(json_function));
      break;
    }
  }
  if (i == current_pid_len) {
    printf("%s\n", from_user_ip);
    cJSON* json_user_info = cJSON_CreateObject();
    cJSON_AddStringToObject(
        json_user_info, "username",
        cJSON_GetObjectItem(cJSON_Parse(current_user_info), "username")
            ->valuestring);
    cJSON_AddStringToObject(json_user_info, "destname", from_user);
    cJSON_AddStringToObject(json_user_info, "ipaddress", from_user_ip);
    pthread_t createpid_thread;
    createDetachPthread(&createpid_thread, createPidThreadShow,
                        (void*)json_user_info);
  }
  g_print("%s\n", from_user);
}

void createPidThreadShow(void* arg) {
  int ret;
  char username[bufsize], destname[bufsize], ipaddress[bufsize];
  cJSON* json_user_info = (cJSON*)arg;
  strcpy(username,
         cJSON_GetObjectItem(json_user_info, "username")->valuestring);
  strcpy(destname,
         cJSON_GetObjectItem(json_user_info, "destname")->valuestring);
  strcpy(ipaddress,
         cJSON_GetObjectItem(json_user_info, "ipaddress")->valuestring);
  printf("this showed pid:%s\n", cJSON_Print(json_user_info));
  pid_t pid = fork();
  if (pid > 0) {
    addUserPidToQueue(destname, pid);
    printf("This is lyepop's main process .its child process with pid %d\n",
           pid);
  } else {
    ret = execlp("./chat", "./chat", username, destname, ipaddress, "1", NULL);
    if (ret < 0) {
      printf("error\n");
    }
  }
  pthread_exit(NULL);
}

GtkTreeModel* create_friend_list_model(cJSON* _friend_list, int type) {
  int i;
  int size = cJSON_GetArraySize(_friend_list);
  GtkListStore* treestore;
  GtkTreeIter iter;
  gchar* iconName;
  char username[bufsize], ipaddress[bufsize], nickname[bufsize];
  cJSON* _friend;
  treestore = gtk_list_store_new(NUM_COLUMN, GDK_TYPE_PIXBUF, G_TYPE_STRING,
                                 G_TYPE_STRING);
  for (i = 0; i < size; i++) {
    GdkPixbuf* icon;
    _friend = cJSON_GetArrayItem(_friend_list, i);
    strcpy(username, cJSON_GetArrayItem(_friend, 0)->valuestring);
    gtk_list_store_append(treestore, &iter);
    if (type == 0) {
      iconName = g_strdup_printf("./head_icon/%s.jpg", username);
      icon = gdk_pixbuf_new_from_file(iconName, NULL);
      strcpy(ipaddress, cJSON_GetArrayItem(_friend, 3)->valuestring);
      printf("%s\n", ipaddress);
      gtk_list_store_set(treestore, &iter, ICON_COLUMN, icon, NAME_COLUMN,
                         username, EXTRA_COLUMN, ipaddress, -1);
    } else {
      iconName = g_strdup_printf("./head_icon/%sh.jpg", username);
      icon = gdk_pixbuf_new_from_file(iconName, NULL);
      strcpy(nickname, cJSON_GetArrayItem(_friend, 1)->valuestring);
      gtk_list_store_set(treestore, &iter, ICON_COLUMN, icon, NAME_COLUMN,
                         username, EXTRA_COLUMN, nickname, -1);
    }
    g_free(iconName);
  }
  return GTK_TREE_MODEL(treestore);
}

void setFriendListColumns(GtkTreeView* view) {
  GtkCellRenderer* cellRenderer;
  GtkTreeViewColumn* column;
  column = gtk_tree_view_column_new();

  cellRenderer = gtk_cell_renderer_pixbuf_new();
  gtk_tree_view_column_pack_start(column, cellRenderer, FALSE);
  gtk_tree_view_column_add_attribute(column, cellRenderer, "pixbuf",
                                     ICON_COLUMN);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cellRenderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(column, cellRenderer, FALSE);
  gtk_tree_view_column_add_attribute(column, cellRenderer, "text", NAME_COLUMN);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);

  cellRenderer = gtk_cell_renderer_text_new();
  gtk_tree_view_column_pack_start(column, cellRenderer, FALSE);
  gtk_tree_view_column_add_attribute(column, cellRenderer, "text",
                                     EXTRA_COLUMN);
  gtk_tree_view_append_column(GTK_TREE_VIEW(view), column);
}

void updateFriendList(cJSON* _friend_list, GtkWidget* _friend_list_tree_view,
                      int type) {
  GtkTreeModel* model = create_friend_list_model(_friend_list, type);
  gtk_tree_view_set_model(GTK_TREE_VIEW(_friend_list_tree_view), model);
}

void createFriendWindow() {
  GtkWidget* vbox;
  GtkWidget* frame1;
  GtkWidget* frame2;
  GtkWidget* framebox1;
  GtkWidget* framebox2;
  GtkWidget* view;
  GtkWidget* hbox;
  GtkWidget* scrolledwindow1;
  GtkWidget* scrolledwindow2;
  GtkWidget* image;
  GtkWidget* buttonBox;
  GtkWidget* buttonAdd;
  GtkWidget* buttonRefresh;

  friendListWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  vbox = gtk_vbox_new(FALSE, 0);

  hbox = gtk_hbox_new(FALSE, 0);
  gchar* icon_name = g_strdup_printf("./head_icon/yyypasserbyb.jpg");
  image = gtk_image_new_from_file(icon_name);

  gtk_widget_set_size_request(hbox, 200, 80);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 0);
  GtkWidget* hbox1 = gtk_hbox_new(FALSE, 5);
  gtk_widget_set_size_request(hbox1, 80, 80);
  gtk_container_add(GTK_CONTAINER(hbox1), image);
  gtk_box_pack_start(GTK_BOX(hbox), hbox1, FALSE, FALSE, 2);
  GtkWidget* hvbox1 = gtk_vbox_new(FALSE, 5);
  gtk_widget_set_size_request(hvbox1, 80, 80);
  gtk_box_pack_start(GTK_BOX(hbox), hvbox1, FALSE, FALSE, 2);
  GtkWidget *label1, *label2;
  gchar* a = "aaaaaaaaaaaaa";
  label1 = gtk_label_new(a);
  gtk_box_pack_start(GTK_BOX(hvbox1), label1, TRUE, FALSE, 2);
  a = "bbbbbbbbbbbbbb";
  label2 = gtk_label_new(a);
  gtk_box_pack_start(GTK_BOX(hvbox1), label2, TRUE, FALSE, 2);
  framebox1 = gtk_vbox_new(FALSE, 0);
  scrolledwindow1 = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(scrolledwindow1, 220, 200);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow1),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow1),
                                      GTK_SHADOW_IN);

  framebox2 = gtk_vbox_new(FALSE, 0);
  scrolledwindow2 = gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_set_size_request(scrolledwindow2, 200, 200);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolledwindow2),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrolledwindow2),
                                      GTK_SHADOW_IN);

  gtk_window_set_default_size(GTK_WINDOW(friendListWindow), 220, 500);
  gtk_window_set_policy(GTK_WINDOW(friendListWindow), 0, 0, 0);
  gtk_window_set_position(GTK_WINDOW(friendListWindow), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(friendListWindow), 5);
  frame1 = gtk_frame_new("Online");
  gtk_container_add(GTK_CONTAINER(vbox), frame1);
  frame2 = gtk_frame_new("Offline");
  gtk_container_add(GTK_CONTAINER(vbox), frame2);

  listOnlineFriendTreeView = gtk_tree_view_new();
  listOfflineFriendTreeView = gtk_tree_view_new();
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(listOnlineFriendTreeView),
                                    FALSE);
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(listOfflineFriendTreeView),
                                    FALSE);
  setFriendListColumns(GTK_TREE_VIEW(listOnlineFriendTreeView));
  setFriendListColumns(GTK_TREE_VIEW(listOfflineFriendTreeView));

  GtkTreeSelection* selection;
  selection =
      gtk_tree_view_get_selection(GTK_TREE_VIEW(listOnlineFriendTreeView));
  g_signal_connect(listOnlineFriendTreeView, "row-activated",
                   G_CALLBACK(list_begin_chat), selection);
  selection =
      gtk_tree_view_get_selection(GTK_TREE_VIEW(listOfflineFriendTreeView));
  g_signal_connect(listOfflineFriendTreeView, "row-activated",
                   G_CALLBACK(list_begin_chat), NULL);

  gtk_container_add(GTK_CONTAINER(scrolledwindow1), listOnlineFriendTreeView);
  gtk_container_add(GTK_CONTAINER(scrolledwindow2), listOfflineFriendTreeView);
  gtk_box_pack_start(GTK_BOX(framebox1), scrolledwindow1, TRUE, TRUE, 3);
  gtk_box_pack_start(GTK_BOX(framebox2), scrolledwindow2, TRUE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(frame1), framebox1);
  gtk_container_add(GTK_CONTAINER(frame2), framebox2);

  buttonBox = gtk_hbox_new(TRUE, 5);
  GtkWidget* hbuttonBox = gtk_hbox_new(TRUE, 5);
  gtk_widget_set_size_request(hbuttonBox, 40, 20);
  gtk_box_pack_start(GTK_BOX(buttonBox), hbuttonBox, TRUE, FALSE, 3);
  gtk_box_pack_start(GTK_BOX(vbox), buttonBox, TRUE, FALSE, 5);
  buttonAdd = gtk_button_new_with_label("add");
  gtk_widget_set_size_request(buttonAdd, 60, 20);
  gtk_box_pack_start(GTK_BOX(buttonBox), buttonAdd, FALSE, FALSE, 3);
  buttonRefresh = gtk_button_new_with_label("refresh");
  gtk_widget_set_size_request(buttonAdd, 60, 20);
  gtk_box_pack_start(GTK_BOX(buttonBox), buttonRefresh, FALSE, FALSE, 3);

  // g_signal_connect(G_OBJECT(buttonAdd),"clicked",G_CALLBACK(remove_from_friend_list),listOnlineFriendTreeView);
  g_signal_connect(G_OBJECT(friendListWindow), "destroy", G_CALLBACK(onDestroy),
                   friendListWindow);
  gtk_container_add(GTK_CONTAINER(friendListWindow), vbox);
  gtk_widget_show_all(friendListWindow);
}
