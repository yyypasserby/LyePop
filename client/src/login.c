#include <gdk/gdkcursor.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "../../lib/pthread_utils.h"
#include "../../lib/socket_utils.h"
#include "third_party/cJSON/cJSON.h"

#define INPUT_HEIGHT 40
#define INPUT_WIDTH 200
#define BUTTON_HEIGHT 40
#define BUTTON_WIDTH 130
#define bufsize 4096

#define LOGIN_REQUEST 0
extern int connectfd;
extern int listenfd;
extern struct sockaddr_in server_addr;
extern struct sockaddr_in any_addr;

extern GtkWidget* name_input;
extern GtkWidget* pwd_input;
extern GtkWidget* login_window;
extern GtkWidget* user_box;

extern char current_user_info[bufsize];

gboolean onMove(GtkWidget* widget, GdkEventButton* event, GdkWindowEdge edge);

void* sendLoginRequest(void* arg);

void onQuit(GtkWidget* button, gpointer data) {
  close(connectfd);
  gtk_main_quit();
  exit(0);
}
void onCheck(GtkWidget* check, gpointer data) {
  g_print("remember password.\n");
}

void onLogin(GtkWidget* login_button, gpointer data) {
  GtkWidget* load_box;
  GtkWidget* cancel_button;
  GtkWidget* load_image;
  pthread_t send_login_request;
  createDetachPthread(&send_login_request, sendLoginRequest, NULL);
  gtk_container_remove(GTK_CONTAINER(login_window), user_box);

  load_box = gtk_vbox_new(FALSE, 0);
  load_image = gtk_image_new_from_file("./layout/loading.gif");
  gtk_container_add(GTK_CONTAINER(login_window), load_box);
  gtk_box_pack_end(GTK_BOX(load_box), load_image, FALSE, FALSE, 20);
  gtk_widget_show_all(login_window);
}

void* sendLoginRequest(void* arg) {
  char send_buffer[bufsize];
  cJSON* function = cJSON_CreateObject();
  cJSON* user = cJSON_CreateObject();
  cJSON_AddNumberToObject(function, "header", LOGIN_REQUEST);
  cJSON_AddItemToObject(function, "user", user);
  cJSON_AddStringToObject(user, "username",
                          gtk_entry_get_text(GTK_ENTRY(name_input)));
  cJSON_AddStringToObject(user, "password",
                          gtk_entry_get_text(GTK_ENTRY(pwd_input)));
  strcpy(send_buffer, cJSON_Print(function));
  printf("%s\n", send_buffer);
  sendMessage(&connectfd, send_buffer, strlen(send_buffer) + 1);
}

void onRegister(GtkWidget* reg_button, gpointer data) {
  g_print("remember password.\n");
}

void createLoginWindow() {
  GdkPixbuf* pixbuf = NULL;
  GdkPixmap* pixmap = NULL;
  GdkBitmap* bitmap = NULL;

  GtkWidget* login_button;
  GtkWidget* register_button;
  GtkWidget* close_button;
  GtkWidget* check;

  GtkWidget* username_image;
  GtkWidget* password_image;
  GtkWidget* login_button_image;
  GtkWidget* register_button_image;

  login_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(login_window), FALSE);
  gtk_window_set_default_size(GTK_WINDOW(login_window), 450, 350);
  gtk_widget_set_app_paintable(login_window, TRUE);
  gtk_window_set_position(GTK_WINDOW(login_window), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(login_window), 0);
  gtk_widget_realize(login_window);

  gtk_widget_add_events(login_window, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(G_OBJECT(login_window), "button-press-event",
                   G_CALLBACK(onMove), NULL);

  pixbuf = gdk_pixbuf_new_from_file("./layout/loginBG.png",
                                    NULL);  // gdk函数读取png文件
  gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap,
                                    128);  // alpha小于128认为透明
  gtk_widget_shape_combine_mask(login_window, bitmap, 0, 0);  // 设置透明蒙板
  gdk_window_set_back_pixmap(login_window->window, pixmap,
                             FALSE);  // 设置窗口背景
  g_object_unref(pixbuf);
  g_object_unref(bitmap);
  g_object_unref(pixmap);

  user_box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(login_window), user_box);

  GtkWidget* close_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_set_size_request(close_box, 450, 120);
  gtk_box_pack_start(GTK_BOX(user_box), close_box, TRUE, FALSE, 0);

  GtkWidget* username_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_set_size_request(username_box, 450, 40);
  gtk_box_pack_start(GTK_BOX(user_box), username_box, TRUE, FALSE, 0);
  username_image = gtk_image_new_from_file("./layout/username.png");
  name_input = gtk_entry_new();
  gtk_widget_set_size_request(name_input, INPUT_WIDTH, INPUT_HEIGHT);
  gtk_box_pack_start(GTK_BOX(username_box), username_image, FALSE, FALSE, 40);
  gtk_box_pack_start(GTK_BOX(username_box), name_input, FALSE, FALSE, 0);

  GtkWidget* password_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_set_size_request(password_box, 450, 40);
  gtk_box_pack_start(GTK_BOX(user_box), password_box, TRUE, FALSE, 0);
  password_image = gtk_image_new_from_file("./layout/password.png");
  pwd_input = gtk_entry_new();
  gtk_widget_set_size_request(pwd_input, INPUT_WIDTH, INPUT_HEIGHT);
  gtk_entry_set_visibility(GTK_ENTRY(pwd_input), FALSE);
  gtk_box_pack_start(GTK_BOX(password_box), password_image, FALSE, FALSE, 40);
  gtk_box_pack_start(GTK_BOX(password_box), pwd_input, FALSE, FALSE, 0);

  GtkWidget* function_box = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(user_box), function_box, TRUE, FALSE, 0);
  gtk_widget_set_size_request(function_box, 450, 15);
  check = gtk_check_button_new_with_label("Remember me");
  g_signal_connect(G_OBJECT(check), "clicked", G_CALLBACK(onCheck), NULL);
  gtk_box_pack_start(GTK_BOX(function_box), check, FALSE, FALSE, 200);

  GtkWidget* action_box = gtk_hbox_new(FALSE, 0);
  gtk_widget_set_size_request(action_box, 450, 40);
  gtk_box_pack_start(GTK_BOX(user_box), action_box, TRUE, FALSE, 0);
  login_button = gtk_button_new();
  gtk_widget_set_size_request(login_button, BUTTON_WIDTH, BUTTON_HEIGHT);
  login_button_image = gtk_image_new_from_file("./layout/loginbutton.png");
  gtk_container_add(GTK_CONTAINER(login_button), login_button_image);
  gtk_box_pack_start(GTK_BOX(action_box), login_button, FALSE, FALSE, 60);

  register_button = gtk_button_new();
  gtk_widget_set_size_request(register_button, BUTTON_WIDTH, BUTTON_HEIGHT);
  register_button_image =
      gtk_image_new_from_file("./layout/registerbutton.png");
  gtk_container_add(GTK_CONTAINER(register_button), register_button_image);
  gtk_box_pack_start(GTK_BOX(action_box), register_button, FALSE, FALSE, 0);

  close_button = gtk_button_new_with_label("x");
  gtk_widget_set_size_request(close_button, 20, 20);
  gtk_box_pack_start(GTK_BOX(action_box), close_button, FALSE, FALSE, 15);

  g_signal_connect(G_OBJECT(login_button), "clicked", G_CALLBACK(onLogin),
                   NULL);
  g_signal_connect(G_OBJECT(register_button), "clicked", G_CALLBACK(onRegister),
                   NULL);

  g_signal_connect_swapped(G_OBJECT(close_button), "clicked",
                           G_CALLBACK(onQuit), login_window);

  gtk_widget_show_all(login_window);
}
