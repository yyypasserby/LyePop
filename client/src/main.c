#include <fcntl.h>
#include <gdk/gdkcursor.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

enum { ICON_COLUMN, NAME_COLUMN, NUM_COLUMN, SIG_COLUMN, LOC_COLUMN };

// Main Window
static GtkWidget *dataWindow1;
static GtkWidget *dataWindow2;
static GtkWidget *dataWindow3;
static GtkWidget *menuWindow;
static GtkWidget *mainWindow;
static GtkWidget *backwindow;

// Chat Window
static GtkWidget *chatWindow;
static GtkWidget *recordWindow;
static GtkWidget *faceWindow;

// Friend List Window
static GtkWidget *frinedListWindow;
GtkWidget *listOnlineFriendTreeView;
GtkWidget *listOfflineFriendTreeView;

// Relationship Manage Window
static GtkWidget *friendManageTreeView;
static GtkTreeSelection *addselection;
static GtkWidget *searchWindow;
static GtkWidget *friendManageWindow;

static gint count1 = 1;
static gint count2 = 1;
static gint count3 = 1;
static gint count4 = 1;
static gint datawinxadd = 197;
static gint datawinyadd = 125;
static gint main_count = 0;
static gint mainWinX;
static gint mainWinY;

// Avatar
static gint selfIcon = 16;

typedef struct showCloseFriendInfo {
  gint icon;
  gchar *name;
  gchar *signature;
  gchar *location;
  gint condition;
} info;

info closeFriend[3] = {{1, "aaa", "aaaaa", "aaaaaa", 1},
                       {2, "bbbb", "bbbbbb", "bbbbb", 1},
                       {3, "ccc", "ccccccc", "cccc", 1}};
info friendlist1[3] = {{1, "aaa", "aaaaa", "aaaaaa", 1},
                       {2, "bbbb", "bbbbbb", "bbbbb", 1},
                       {3, "ccc", "ccccccc", "cccc", 1}};
info friendlist2[3] = {{1, "aaa", "aaaaa", "aaaaaa", 0},
                       {2, "bbbb", "bbbbbb", "bbbbb", 0},
                       {3, "ccc", "ccccccc", "cccc", 0}};

void create_background_window();
void create_dataWindow1();
void create_dataWindow2();
void create_dataWindow3();
void create_set_menu();
void create_close_window();
void create_search_window();
void create_friend_manage_window();
void destroy_all_window();
void createFriendWindow();
gboolean on_button_press();

void quick_begin_to_chat() {
  destroy_all_window();
  // create_chat_window();
}

void destroy_window(GtkWidget *Widget, gpointer data) {
  gtk_widget_destroy((GtkWidget *)data);
}

void button_event1() {
  if (count1 == 1) {
    create_dataWindow1();
    count1--;
  } else {
    gtk_widget_destroy(dataWindow1);
    count1++;
  }
}

void button_event2() {
  if (count2 == 1) {
    create_dataWindow2();
    count2--;
  } else {
    gtk_widget_destroy(dataWindow2);
    count2++;
  }
}

void button_event3() {
  if (count3 == 1) {
    create_dataWindow3();
    count3--;
  } else {
    gtk_widget_destroy(dataWindow3);
    count3++;
  }
}

void button_event4() {
  if (count4 == 1) {
    create_set_menu();
    count4--;
  } else {
    gtk_widget_destroy(menuWindow);
    count4++;
  }
}

void button_event5() { gtk_main_quit(); }

void destroy_all_window() {
  gtk_widget_destroy(mainWindow);
  gtk_widget_destroy(menuWindow);
  gtk_widget_destroy(backwindow);
  gtk_widget_destroy(dataWindow1);
  gtk_widget_destroy(dataWindow2);
  gtk_widget_destroy(dataWindow3);
}

void create_background_window() {
  GtkWidget *button;
  GtkWidget *eventbox;
  GtkWidget *image;
  GdkPixbuf *backPixbuf = NULL;
  GdkBitmap *backBitmap = NULL;
  GdkPixmap *backPixmap = NULL;
  GtkWidget *box = gtk_hbox_new(FALSE, 0);
  backwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(backwindow), FALSE);
  gtk_widget_set_app_paintable(backwindow, TRUE);

  gtk_widget_realize(backwindow);

  gtk_container_add(GTK_CONTAINER(backwindow), box);

  backPixbuf =
      gdk_pixbuf_new_from_file("./picture/mainbutton/workspace2.png", NULL);
  gdk_pixbuf_render_pixmap_and_mask(backPixbuf, &backPixmap, &backBitmap, 128);
  gtk_widget_shape_combine_mask(backwindow, backBitmap, 0, 0);
  gdk_window_set_back_pixmap(backwindow->window, backPixmap, FALSE);

  g_object_unref(backPixbuf);
  g_object_unref(backBitmap);
  g_object_unref(backPixmap);

  eventbox = gtk_event_box_new();
  gtk_box_pack_start_defaults(GTK_BOX(box), eventbox);
  button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox), button);
  image = gtk_image_new_from_file("./picture/mainbutton/workspace2.png");
  gtk_container_add(GTK_CONTAINER(button), image);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(destroy_all_window),
                   NULL);
  gtk_window_set_opacity(GTK_WINDOW(backwindow), 0.4);
  gtk_widget_show_all(backwindow);
}

void create_dataWindow1() {
  GtkWidget *box;
  GtkWidget *box1, *box2, *box3, *box4;
  GtkWidget *image;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label22;
  GtkWidget *label32;
  GtkWidget *label42;
  GdkPixbuf *data1Pixbuf = NULL;
  GdkBitmap *data1Bitmap = NULL;
  GdkPixmap *data1Pixmap = NULL;

  dataWindow1 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(dataWindow1), FALSE);
  gtk_window_set_title(GTK_WINDOW(dataWindow1), "Data");
  gtk_window_set_default_size(GTK_WINDOW(dataWindow1), 200, 300);

  gtk_window_move(GTK_WINDOW(dataWindow1), mainWinX - datawinxadd,
                  mainWinY - datawinyadd);
  gtk_widget_set_app_paintable(dataWindow1, TRUE);
  gtk_widget_realize(dataWindow1);

  data1Pixbuf =
      gdk_pixbuf_new_from_file("./picture/mainbutton/leftframe.png", NULL);
  gdk_pixbuf_render_pixmap_and_mask(data1Pixbuf, &data1Pixmap, &data1Bitmap,
                                    128);
  gtk_widget_shape_combine_mask(dataWindow1, data1Bitmap, 0, 0);
  gdk_window_set_back_pixmap(dataWindow1->window, data1Pixmap, FALSE);

  g_object_unref(data1Pixbuf);
  g_object_unref(data1Bitmap);
  g_object_unref(data1Pixmap);

  box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(dataWindow1), box);

  box1 = gtk_vbox_new(FALSE, 0);
  box2 = gtk_vbox_new(FALSE, 0);
  box3 = gtk_vbox_new(FALSE, 0);
  box4 = gtk_vbox_new(FALSE, 0);
  label2 = gtk_label_new("Nickname:");
  label3 = gtk_label_new("Signature:");
  label4 = gtk_label_new("Location:");
  label22 = gtk_label_new(closeFriend[0].name);
  label32 = gtk_label_new(closeFriend[0].signature);
  label42 = gtk_label_new(closeFriend[0].location);
  gchar *iconPath =
      g_strdup_printf("./picture/photograph/%db.jpg", closeFriend[0].icon);
  image = gtk_image_new_from_file(iconPath);
  gtk_container_add(GTK_CONTAINER(box1), image);
  gtk_box_pack_start(GTK_BOX(box2), label2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box2), label22, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(box3), label3, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box3), label32, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label3), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(box4), label4, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box4), label42, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label4), GTK_JUSTIFY_CENTER);
  gtk_box_pack_end(GTK_BOX(box), box4, FALSE, FALSE, 30);
  gtk_box_pack_end(GTK_BOX(box), box3, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(box), box2, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(box), box1, FALSE, FALSE, 20);
  gtk_widget_show_all(dataWindow1);
}

void create_dataWindow2() {
  GtkWidget *box;
  GtkWidget *box1, *box2, *box3, *box4;
  GtkWidget *image;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label22;
  GtkWidget *label32;
  GtkWidget *label42;
  GdkPixbuf *data2Pixbuf = NULL;
  GdkBitmap *data2Bitmap = NULL;
  GdkPixmap *data2Pixmap = NULL;
  dataWindow2 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(dataWindow2), FALSE);
  gtk_window_set_title(GTK_WINDOW(dataWindow2), "Data");
  gtk_window_set_default_size(GTK_WINDOW(dataWindow2), 200, 300);

  gtk_window_move(GTK_WINDOW(dataWindow2), mainWinX - datawinxadd,
                  mainWinY - datawinyadd + 80);
  gtk_widget_set_app_paintable(dataWindow2, TRUE);

  data2Pixbuf =
      gdk_pixbuf_new_from_file("./picture/mainbutton/leftframe.png", NULL);
  gdk_pixbuf_render_pixmap_and_mask(data2Pixbuf, &data2Pixmap, &data2Bitmap,
                                    128);
  gtk_widget_shape_combine_mask(dataWindow2, data2Bitmap, 0, 0);
  gdk_window_set_back_pixmap(dataWindow2->window, data2Pixmap, FALSE);

  g_object_unref(data2Pixbuf);
  g_object_unref(data2Bitmap);
  g_object_unref(data2Pixmap);

  box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(dataWindow2), box);

  box1 = gtk_vbox_new(FALSE, 0);
  box2 = gtk_vbox_new(FALSE, 0);
  box3 = gtk_vbox_new(FALSE, 0);
  box4 = gtk_vbox_new(FALSE, 0);
  label2 = gtk_label_new("Nickname:");
  label3 = gtk_label_new("Signature:");
  label4 = gtk_label_new("Location:");
  label22 = gtk_label_new(closeFriend[1].name);
  label32 = gtk_label_new(closeFriend[1].signature);
  label42 = gtk_label_new(closeFriend[1].location);
  gchar *iconPath =
      g_strdup_printf("./picture/photograph/%db.jpg", closeFriend[1].icon);
  image = gtk_image_new_from_file(iconPath);
  gtk_container_add(GTK_CONTAINER(box1), image);
  gtk_box_pack_start(GTK_BOX(box2), label2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box2), label22, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(box3), label3, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box3), label32, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label3), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(box4), label4, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box4), label42, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label4), GTK_JUSTIFY_CENTER);
  gtk_box_pack_end(GTK_BOX(box), box4, FALSE, FALSE, 30);
  gtk_box_pack_end(GTK_BOX(box), box3, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(box), box2, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(box), box1, FALSE, FALSE, 20);

  gtk_widget_show_all(dataWindow2);
}

void create_dataWindow3() {
  GtkWidget *box;
  GtkWidget *box1, *box2, *box3, *box4;
  GtkWidget *image;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *label4;
  GtkWidget *label22;
  GtkWidget *label32;
  GtkWidget *label42;
  GdkPixbuf *data3Pixbuf = NULL;
  GdkBitmap *data3Bitmap = NULL;
  GdkPixmap *data3Pixmap = NULL;

  dataWindow3 = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_decorated(GTK_WINDOW(dataWindow3), FALSE);
  gtk_window_set_title(GTK_WINDOW(dataWindow3), "Data");
  gtk_window_set_default_size(GTK_WINDOW(dataWindow3), 200, 300);

  gtk_window_move(GTK_WINDOW(dataWindow3), mainWinX - datawinxadd,
                  mainWinY - datawinyadd + 160);
  gtk_widget_set_app_paintable(dataWindow3, TRUE);

  data3Pixbuf =
      gdk_pixbuf_new_from_file("./picture/mainbutton/leftframe.png", NULL);
  gdk_pixbuf_render_pixmap_and_mask(data3Pixbuf, &data3Pixmap, &data3Bitmap,
                                    128);
  gtk_widget_shape_combine_mask(dataWindow3, data3Bitmap, 0, 0);
  gdk_window_set_back_pixmap(dataWindow3->window, data3Pixmap, FALSE);

  g_object_unref(data3Pixbuf);
  g_object_unref(data3Bitmap);
  g_object_unref(data3Pixmap);

  box = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(dataWindow3), box);

  box1 = gtk_vbox_new(FALSE, 0);
  box2 = gtk_vbox_new(FALSE, 0);
  box3 = gtk_vbox_new(FALSE, 0);
  box4 = gtk_vbox_new(FALSE, 0);
  label2 = gtk_label_new("Nickname:");
  label3 = gtk_label_new("Signature:");
  label4 = gtk_label_new("Location:");
  label22 = gtk_label_new(closeFriend[2].name);
  label32 = gtk_label_new(closeFriend[2].signature);
  label42 = gtk_label_new(closeFriend[2].location);
  gchar *iconPath =
      g_strdup_printf("./picture/photograph/%db.jpg", closeFriend[2].icon);
  image = gtk_image_new_from_file(iconPath);
  gtk_container_add(GTK_CONTAINER(box1), image);
  gtk_box_pack_start(GTK_BOX(box2), label2, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box2), label22, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label2), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(box3), label3, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box3), label32, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label3), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(box4), label4, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box4), label42, FALSE, FALSE, 0);
  gtk_label_set_justify(GTK_LABEL(label4), GTK_JUSTIFY_CENTER);
  gtk_box_pack_end(GTK_BOX(box), box4, FALSE, FALSE, 30);
  gtk_box_pack_end(GTK_BOX(box), box3, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(box), box2, FALSE, FALSE, 10);
  gtk_box_pack_end(GTK_BOX(box), box1, FALSE, FALSE, 20);

  gtk_widget_show_all(dataWindow3);
}

void create_main_window() {
  GtkWidget *box;
  GtkWidget *leftbox, *middlebox, *rightbox;
  GtkWidget *recent_button1, *recent_button2, *recent_button3;
  GtkWidget *more_button;
  GtkWidget *close_button;
  GtkWidget *rlr_image1, *rlr_image2, *rlr_image3, *rlm_image, *rlc_image;
  GdkPixbuf *mainPixbuf = NULL;
  GdkBitmap *mainBitmap = NULL;
  GdkPixmap *mainPixmap = NULL;
  GtkWidget *eventbox1, *eventbox2, *eventbox3, *eventbox4, *eventbox5;
  GdkGC *gc;
  GdkColormap *colormap;
  GdkBitmap *window_shape_bitmap;
  GdkColor black;
  GdkColor white;

  mainWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(mainWindow), 70, 400);

  gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_MOUSE);
  gtk_window_get_position(GTK_WINDOW(mainWindow), &mainWinX, &mainWinY);

  gtk_window_set_decorated(GTK_WINDOW(mainWindow), FALSE);
  gtk_widget_set_app_paintable(mainWindow, TRUE);
  gtk_widget_realize(mainWindow);

  mainPixbuf = gdk_pixbuf_new_from_file("mainbackground2.png", NULL);
  gdk_pixbuf_render_pixmap_and_mask(mainPixbuf, &mainPixmap, &mainBitmap, 128);
  gtk_widget_shape_combine_mask(mainWindow, mainBitmap, 0, 0);
  gdk_window_set_back_pixmap(mainWindow->window, mainPixmap, FALSE);

  g_object_unref(mainPixbuf);
  g_object_unref(mainPixmap);
  g_object_unref(mainBitmap);

  box = gtk_hbox_new(FALSE, 0);
  leftbox = gtk_vbox_new(FALSE, 0);
  middlebox = gtk_vbox_new(FALSE, 0);
  rightbox = gtk_vbox_new(FALSE, 0);

  gtk_container_add(GTK_CONTAINER(mainWindow), box);

  gtk_box_pack_start(GTK_BOX(box), leftbox, TRUE, TRUE, 8);
  gtk_box_pack_start(GTK_BOX(box), middlebox, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(box), rightbox, TRUE, TRUE, 5);

  eventbox1 = gtk_event_box_new();
  eventbox2 = gtk_event_box_new();
  eventbox3 = gtk_event_box_new();
  eventbox4 = gtk_event_box_new();
  eventbox5 = gtk_event_box_new();

  gtk_box_pack_start(GTK_BOX(middlebox), eventbox1, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(middlebox), eventbox2, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(middlebox), eventbox3, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(middlebox), eventbox4, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(middlebox), eventbox5, TRUE, TRUE, 5);

  recent_button1 = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox1), recent_button1);
  rlr_image1 = gtk_image_new_from_file("./picture/mainbutton/mainbutton1.png");
  gtk_container_add(GTK_CONTAINER(recent_button1), rlr_image1);

  recent_button2 = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox2), recent_button2);
  rlr_image2 = gtk_image_new_from_file("./picture/mainbutton/mainbutton2.png");
  gtk_container_add(GTK_CONTAINER(recent_button2), rlr_image2);

  recent_button3 = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox3), recent_button3);
  rlr_image3 = gtk_image_new_from_file("./picture/mainbutton/mainbutton3.png");
  gtk_container_add(GTK_CONTAINER(recent_button3), rlr_image3);

  more_button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox4), more_button);
  rlm_image = gtk_image_new_from_file("./picture/mainbutton/mainbutton4.png");
  gtk_container_add(GTK_CONTAINER(more_button), rlm_image);

  close_button = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox5), close_button);
  rlc_image = gtk_image_new_from_file("./picture/mainbutton/mainbutton5.png");
  gtk_container_add(GTK_CONTAINER(close_button), rlc_image);

  colormap = gdk_colormap_get_system();
  gdk_color_black(colormap, &black);
  gdk_color_white(colormap, &white);

  window_shape_bitmap = (GdkBitmap *)gdk_pixmap_new(NULL, 70, 70, 1);

  gc = gdk_gc_new(window_shape_bitmap);

  gdk_gc_set_foreground(gc, &black);
  gdk_gc_set_background(gc, &white);

  gdk_draw_rectangle(window_shape_bitmap, gc, TRUE, 5, 5, 62, 62);
  gdk_gc_set_foreground(gc, &white);
  gdk_gc_set_background(gc, &black);

  gdk_draw_arc(window_shape_bitmap, gc, TRUE, 5, 5, 62, 62, 0, 360 * 64);

  gtk_widget_shape_combine_mask(eventbox1, window_shape_bitmap, 0, 0);
  gtk_widget_shape_combine_mask(eventbox2, window_shape_bitmap, 0, 0);
  gtk_widget_shape_combine_mask(eventbox3, window_shape_bitmap, 0, 0);
  gtk_widget_shape_combine_mask(eventbox4, window_shape_bitmap, 0, 0);
  gtk_widget_shape_combine_mask(eventbox5, window_shape_bitmap, 0, 0);

  gtk_widget_show_all(mainWindow);

  g_signal_connect(G_OBJECT(recent_button1), "enter_notify_event",
                   G_CALLBACK(button_event1), NULL);
  g_signal_connect(G_OBJECT(recent_button1), "leave_notify_event",
                   G_CALLBACK(button_event1), NULL);
  g_signal_connect(G_OBJECT(recent_button1), "clicked",
                   G_CALLBACK(destroy_all_window), NULL);
  g_signal_connect(G_OBJECT(recent_button2), "enter_notify_event",
                   G_CALLBACK(button_event2), NULL);
  g_signal_connect(G_OBJECT(recent_button2), "leave_notify_event",
                   G_CALLBACK(button_event2), NULL);
  g_signal_connect(G_OBJECT(recent_button2), "clicked",
                   G_CALLBACK(destroy_all_window), NULL);
  g_signal_connect(G_OBJECT(recent_button3), "enter_notify_event",
                   G_CALLBACK(button_event3), NULL);
  g_signal_connect(G_OBJECT(recent_button3), "leave_notify_event",
                   G_CALLBACK(button_event3), NULL);
  g_signal_connect(G_OBJECT(recent_button3), "clicked",
                   G_CALLBACK(destroy_all_window), NULL);
  g_signal_connect(G_OBJECT(more_button), "clicked", G_CALLBACK(button_event4),
                   NULL);
  g_signal_connect(G_OBJECT(close_button), "clicked", G_CALLBACK(button_event5),
                   NULL);
}

void create_set_menu() {
  GtkWidget *box;
  GtkWidget *leftbox, *middlebox, *rightbox;
  GtkWidget *friendListButton;
  GtkWidget *friendManageButton;
  GtkWidget *friendSearchButton;
  GtkWidget *settingButton;
  GtkWidget *rlm_image, *rls_image, *rlf_image, *rlc_image;

  GdkPixbuf *pixbuf = NULL;
  GdkBitmap *bitmap = NULL;
  GdkPixmap *pixmap = NULL;
  GtkWidget *eventbox1, *eventbox2, *eventbox3, *eventbox4;
  GdkGC *gc;
  GdkColormap *colormap;
  GdkBitmap *window_shape_bitmap;
  GdkColor black;
  GdkColor white;

  menuWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_default_size(GTK_WINDOW(menuWindow), 70, 260);
  gtk_window_set_decorated(GTK_WINDOW(menuWindow), FALSE);
  gtk_window_move(GTK_WINDOW(menuWindow), mainWinX + 60, mainWinY - 7);
  gtk_widget_set_app_paintable(menuWindow, TRUE);
  gtk_widget_realize(menuWindow);

  pixbuf =
      gdk_pixbuf_new_from_file("./picture/menubutton/menubackground.png", NULL);

  gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap, 128);

  gtk_widget_shape_combine_mask(menuWindow, bitmap, 0, 0);

  gdk_window_set_back_pixmap(menuWindow->window, pixmap, FALSE);

  g_object_unref(pixbuf);
  g_object_unref(bitmap);
  g_object_unref(pixmap);

  box = gtk_hbox_new(FALSE, 0);
  leftbox = gtk_vbox_new(FALSE, 0);
  middlebox = gtk_vbox_new(FALSE, 0);
  rightbox = gtk_vbox_new(FALSE, 0);

  gtk_container_add(GTK_CONTAINER(menuWindow), box);

  gtk_box_pack_start(GTK_BOX(box), leftbox, TRUE, TRUE, 8);
  gtk_box_pack_start(GTK_BOX(box), middlebox, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(box), rightbox, TRUE, TRUE, 5);

  eventbox1 = gtk_event_box_new();
  eventbox2 = gtk_event_box_new();
  eventbox3 = gtk_event_box_new();
  eventbox4 = gtk_event_box_new();

  gtk_box_pack_start(GTK_BOX(middlebox), eventbox1, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(middlebox), eventbox2, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(middlebox), eventbox3, TRUE, TRUE, 5);
  gtk_box_pack_start(GTK_BOX(middlebox), eventbox4, TRUE, TRUE, 5);

  friendListButton = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox1), friendListButton);
  rlm_image = gtk_image_new_from_file("./picture/mainbutton/mainbutton6.png");
  gtk_container_add(GTK_CONTAINER(friendListButton), rlm_image);

  friendManageButton = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox2), friendManageButton);
  rlf_image = gtk_image_new_from_file("./picture/mainbutton/mainbutton7.png");
  gtk_container_add(GTK_CONTAINER(friendManageButton), rlf_image);

  friendSearchButton = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox3), friendSearchButton);
  rls_image = gtk_image_new_from_file("./picture/mainbutton/mainbutton8.png");
  gtk_container_add(GTK_CONTAINER(friendSearchButton), rls_image);

  settingButton = gtk_button_new();
  gtk_container_add(GTK_CONTAINER(eventbox4), settingButton);
  rlc_image = gtk_image_new_from_file("./picture/mainbutton/mainbutton9.png");
  gtk_container_add(GTK_CONTAINER(settingButton), rlc_image);

  colormap = gdk_colormap_get_system();
  gdk_color_black(colormap, &black);
  gdk_color_white(colormap, &white);

  window_shape_bitmap = (GdkBitmap *)gdk_pixmap_new(NULL, 70, 70, 1);

  gc = gdk_gc_new(window_shape_bitmap);

  gdk_gc_set_foreground(gc, &black);
  gdk_gc_set_background(gc, &white);

  gdk_draw_rectangle(window_shape_bitmap, gc, TRUE, 5, 5, 62, 62);
  gdk_gc_set_foreground(gc, &white);
  gdk_gc_set_background(gc, &black);

  gdk_draw_arc(window_shape_bitmap, gc, TRUE, 5, 5, 62, 62, 0, 360 * 64);

  gtk_widget_shape_combine_mask(eventbox1, window_shape_bitmap, 0, 0);
  gtk_widget_shape_combine_mask(eventbox2, window_shape_bitmap, 0, 0);
  gtk_widget_shape_combine_mask(eventbox3, window_shape_bitmap, 0, 0);
  gtk_widget_shape_combine_mask(eventbox4, window_shape_bitmap, 0, 0);
  g_signal_connect_swapped(G_OBJECT(friendListButton), "clicked",
                           G_CALLBACK(destroy_all_window), NULL);

  g_signal_connect_swapped(G_OBJECT(friendManageButton), "clicked",
                           G_CALLBACK(destroy_all_window), NULL);

  g_signal_connect_swapped(G_OBJECT(friendSearchButton), "clicked",
                           G_CALLBACK(destroy_all_window), NULL);

  gtk_widget_show_all(menuWindow);
}
