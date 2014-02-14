#include<gtk/gtk.h>
#include<libnotify/notify.h>
#include<gdk/gdkkeysyms.h>
#include<gdk/gdkcursor.h>
#include<sys/msg.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<time.h>
#include"sockHead.h"
#include"pthreadHead.h"
#include"cJSON.h"

#define listenport 4488
#define serverport 5599 
#define bufsize 4096

#define ON_SUCCESS 0
#define ON_FAILURE 1

#define CHAT_MESSAGE 0
#define FUNC_MESSAGE 1
#define FILE_MESSAGE 2
#define PIC_MESSAGE 3

typedef struct _ProgressData { 
    GtkWidget *window; 
    GtkWidget *pbar; 
    int timer; 
    gboolean activity_mode; 
} ProgressData; 

GtkWidget * Rcv_scrolled_win;
GtkWidget * chat_window;
GtkWidget * Send_textview, * Rcv_textview;
GtkTextBuffer * Send_buffer, * Rcv_buffer, *Record_buffer;
GtkWidget * record_window, * face_window;
GdkPixbuf * facePixbuf;
int recordWindowVisible = 1;
int chatWindowVisible = 1;
int win_posx, win_posy;

int sendfd;
struct sockaddr_in dest_addr;

char username[ bufsize ];
char destname[ bufsize ];
char dest_ipaddress[ bufsize ];

void Get_Local_message(const gchar *text);
int progress_timeout( gpointer data ); 
void destroy_progress( GtkWidget *widget, ProgressData *pdata);
void send_file();
void on_file_select_ok (GtkWidget *button,GtkFileSelection *fs);
void open_file(GtkWidget *button,gpointer userdata);
void select_font(GtkWidget *button,gpointer userdata);	
void select_color(GtkButton* button, gpointer data);
void on_face_button(GtkWidget* window,gpointer i);
void select_face(GtkButton* button1, gpointer data);
void create_record_window();
void on_record_button();
void Clear_Local_message();
void Get_Local_message(const gchar *text);
void Put_Local_message(const gchar *text);
void Show_Err(char *err);
void on_send(GtkButton *SaveButton, GtkWidget *Send_textview);
void on_close(GtkButton *CloseButton,GtkWidget *Send_textview);
void createChatWindow();
void * receiveThread( void * arg );
void Put_Remote_message( cJSON * _json_message );
void notifyMessage( char * title, char * content );
void put_local_pix(GdkPixbuf*);
void control_rec_scrolled_win();
gboolean onMove (GtkWidget* widget,GdkEventButton * event, GdkWindowEdge edge);

int main( int argc, char * argv[] ){

    if( argc < 5 ){
        printf( "It must be a wrong call.\n");
        exit( 1 );
    }
    strcpy( username, argv[1] );
    strcpy( destname, argv[2] );
    strcpy( dest_ipaddress, argv[3] );
    createSocket( &sendfd, 1 );
    initAddr( &dest_addr, dest_ipaddress, listenport );
    gtk_init( &argc, &argv );
    if( strcmp( argv[ 4 ], "0" ) != 0  ){
        createChatWindow();
        chatWindowVisible = 0;
    }
    if( !g_thread_supported() )
        g_thread_init( NULL );
    gdk_threads_init();

    gdk_threads_enter();
    pthread_t receive_thread;
    createDetachPthread( &receive_thread, receiveThread, NULL );
    gtk_main();
    gdk_threads_leave();
    return 0;
}

void savebuffer(char*buffer){
    FILE *fp;
    fp=fopen( username, "at");
    if(!fp)
    {
        printf("can not open the file!\n");
    }
    time_t lt = time( NULL );

    fputs( ctime(&lt), fp );
    fputs( "I",fp);
    fputs(":",fp);
    fputs(buffer,fp);
    fputs("\n",fp);
    fclose(fp);
}

void * receiveThread( void * arg ){
    int pid = (int)getpid();
    int msgid = msgget( pid, 0666|IPC_CREAT );
    printf( "this is %d\n", msgid );
    char message[ bufsize ];
    cJSON * json_message;
    while( 1 ){
        msgrcv( msgid, message, bufsize, 0, 0 );
        printf( "%s\n", message );
        json_message = cJSON_Parse( message );
        int header = cJSON_GetObjectItem( json_message, "header")->valueint;
        switch( header ){
            case CHAT_MESSAGE:
                gdk_threads_enter();
                Put_Remote_message( json_message );
                gdk_threads_leave();
                break;
            case FUNC_MESSAGE:
                gdk_threads_enter();
                if( chatWindowVisible ){
                    createChatWindow();
                    chatWindowVisible--;
                }
                gdk_threads_leave();
                break;
            case FILE_MESSAGE:
                break;
            case PIC_MESSAGE:
                break;
        }
        sleep( 1 );
    }
}
void notifyMessage( char * title, char * content ){
    NotifyNotification *n;
    notify_init("Basics");
    n = notify_notification_new( title, content, NULL );
    notify_notification_set_timeout (n, 3000); // 5 seconds
    if (!notify_notification_show (n, NULL))
        perror("notification system launched failed!\n");
    g_object_unref(G_OBJECT(n));
}

int progress_timeout( gpointer data ) { 
    ProgressData *pdata = (ProgressData *)data; 
    gdouble new_val; 

    if (pdata->activity_mode)  
        gtk_progress_bar_pulse (GTK_PROGRESS_BAR (pdata->pbar)); 
    else{ 
        new_val = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR  
                (pdata->pbar)) + 0.01; 
        if (new_val > 1.0) 
            new_val = 0.0; 
        gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (pdata->pbar),  
                new_val); 
    } 
    return TRUE; 
}  
void destroy_progress( GtkWidget *widget, ProgressData *pdata) { 
    gtk_timeout_remove (pdata->timer); 
    pdata->timer = 0; 
    pdata->window = NULL; 
    g_free (pdata); 
} 

void send_file(){
    ProgressData *pdata; 
    GtkWidget *align; 
    GtkWidget *separator; 
    GtkWidget *table; 
    GtkWidget *button; 
    GtkWidget *check; 
    GtkWidget *vbox;
    GtkWidget *label;

    pdata = g_malloc (sizeof (ProgressData)); 

    pdata->window = gtk_window_new (GTK_WINDOW_TOPLEVEL); 
    gtk_window_set_resizable (GTK_WINDOW (pdata->window), TRUE); 
    gtk_window_set_position(GTK_WINDOW(pdata->window),GTK_WIN_POS_CENTER);  
    g_signal_connect (G_OBJECT (pdata->window), "destroy", G_CALLBACK (destroy_progress), pdata); 
    gtk_window_set_title (GTK_WINDOW (pdata->window),"Send File"); 
    gtk_container_set_border_width (GTK_CONTAINER (pdata->window), 0); 

    vbox = gtk_vbox_new (FALSE, 5); 
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 10); 
    gtk_container_add (GTK_CONTAINER (pdata->window), vbox); 
    gtk_widget_show (vbox); 

    /* 创建一个居中对齐的对象 */ 
    align = gtk_alignment_new (0.5, 0.5, 0, 0); 
    gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 5); 
    gtk_widget_show (align); 

    /* 创建进度条 */ 
    pdata->pbar = gtk_progress_bar_new (); 

    gtk_container_add (GTK_CONTAINER (align), pdata->pbar); 
    gtk_widget_show (pdata->pbar); 

    /* 加一个定时器(timer)，以更新进度条的值 */ 
    pdata->timer = gtk_timeout_add (100, progress_timeout, pdata); 

    label = gtk_label_new("File is Sending...");
    gtk_box_pack_start(GTK_BOX(vbox),label,FALSE,FALSE,10);
    gtk_widget_show(label);

    separator = gtk_hseparator_new (); 
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0); 
    gtk_widget_show (separator); 

    button = gtk_button_new_with_label ("close"); 
    g_signal_connect_swapped (G_OBJECT (button), "clicked",G_CALLBACK (gtk_widget_destroy),pdata->window); 
    gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0); 

    /* 将按钮设置为能缺省的构件 */ 
    GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT); 

    gtk_widget_grab_default (button); 
    gtk_widget_show (button); 

    gtk_widget_show (pdata->window); 
}

void on_file_select_ok (GtkWidget *button,GtkFileSelection *fs){
    /*GtkWidget *dialog;
      gchar message[1024];
      const gchar *filename;
      filename = gtk_file_selection_get_filename(fs);
      if(g_file_test(filename,G_FILE_TEST_IS_DIR))
      sprintf(message,"The directory you chosed is:%s",filename);
      else
      sprintf(message,"The file you chosed is:%s",filename);
      dialog = gtk_message_dialog_new(NULL,GTK_DIALOG_DESTROY_WITH_PARENT,GTK_MESSAGE_INFO,GTK_BUTTONS_OK,message,NULL);
      */
    gtk_widget_destroy(GTK_WIDGET(fs));
    //gtk_dialog_run(GTK_DIALOG(dialog));
    //gtk_widget_destroy(dialog);
    send_file();
}

void open_file(GtkWidget *button,gpointer userdata){
    GtkWidget* dialog ;
    dialog = gtk_file_selection_new("Please choose a file or directory:");
    gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER);
    gtk_signal_connect(GTK_OBJECT(dialog),"destroy",GTK_SIGNAL_FUNC(gtk_widget_destroy),&dialog);
    gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button),"clicked",GTK_SIGNAL_FUNC(on_file_select_ok),GTK_FILE_SELECTION(dialog));
    gtk_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button),"clicked",GTK_SIGNAL_FUNC(gtk_widget_destroy),GTK_OBJECT(dialog));
    gtk_widget_show(dialog);
}


//change font function

void select_font(GtkWidget *button,gpointer userdata){	
    char font_send[100];
    GtkResponseType result;
    /*建立FONT对话框*/
    GtkWidget *dialog=gtk_font_selection_dialog_new("Select Font");
    result=gtk_dialog_run(GTK_DIALOG(dialog));
    //若点击确定，改变字体设置
    if(result == GTK_RESPONSE_OK || result ==GTK_RESPONSE_APPLY)
    {
        PangoFontDescription *font_desc;
        gchar *fontname=gtk_font_selection_dialog_get_font_name(GTK_FONT_SELECTION_DIALOG(dialog));
        font_desc=pango_font_description_from_string(fontname);
        memset(font_send,0,100);
        strcpy(font_send,fontname);
        g_print("%s\n",font_send);
        gtk_widget_modify_font(GTK_WIDGET(Send_textview),font_desc);
        gtk_widget_modify_font(GTK_WIDGET(Rcv_textview),font_desc);
        g_free(fontname);
    }
    gtk_widget_destroy(dialog);
}

//change color function
void select_color(GtkButton* button, gpointer data)
{
    char color_send[100];
    GtkResponseType result;
    GtkColorSelection *colorsel;
    GtkWidget *dialog = gtk_color_selection_dialog_new("Select Color");
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if(result == GTK_RESPONSE_OK)
    {
        GdkColor color;
        colorsel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel);
        gtk_color_selection_get_current_color(colorsel, &color);
        //memset(color_send,0,100);
        //strcpy(color_send,GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)));
        //g_print("%s\n",color_send);
        gtk_widget_modify_text(Send_textview,GTK_STATE_NORMAL,&color);
        gtk_widget_modify_text(Rcv_textview,GTK_STATE_NORMAL,&color);
    }
    gtk_widget_destroy(dialog);	
}

//send face function
void on_face_button(GtkWidget* window,gpointer i){
    switch((int)i){
        case 1:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face1.gif",NULL);
            break;
        case 2:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face2.gif",NULL);
            break;
        case 3:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face3.gif",NULL);
            break;
        case 4:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face4.gif",NULL);
            break;
        case 5:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face5.gif",NULL);
            break;
        case 6:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face6.gif",NULL);
            break;
        case 7:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face7.gif",NULL);
            break;
        case 8:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face8.gif",NULL);
            break;
        case 9:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face9.gif",NULL);
            break;
        case 10:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face10.gif",NULL);
            break;
        case 11:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face11.gif",NULL);
            break;
        case 12:
            facePixbuf = gdk_pixbuf_new_from_file("./face/face12.gif",NULL);
            break;
    }
    put_local_pix( facePixbuf );
    gtk_widget_destroy(face_window);
    control_rec_scrolled_win();
}

void select_face(GtkButton* button1, gpointer data){
    GtkWidget *button;
    GtkWidget *image;
    GtkWidget *table;

    face_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(face_window),210,158);
    gtk_window_set_position(GTK_WINDOW(face_window),GTK_WIN_POS_MOUSE);
    table = gtk_table_new(4,3,FALSE);
    gtk_container_set_border_width(GTK_CONTAINER(table),2);
    gtk_table_set_row_spacings(GTK_TABLE(table),2);
    gtk_table_set_col_spacings(GTK_TABLE(table),2);
    gtk_container_add(GTK_CONTAINER(face_window),table);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face1.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,0,1,0,1,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)1);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face2.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,1,2,0,1,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)2);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face3.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,2,3,0,1,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)3);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face4.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,3,4,0,1,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)4);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face5.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,0,1,1,2,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)5);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face6.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,1,2,1,2,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)6);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face7.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,2,3,1,2,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)7);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face8.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,3,4,1,2,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)8);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face9.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,0,1,2,3,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)9);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face10.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,1,2,2,3,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)10);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face11.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,2,3,2,3,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)11);

    button = gtk_button_new();
    image = gtk_image_new_from_file("./face/face12.gif");
    gtk_container_add(GTK_CONTAINER(button),image);
    gtk_table_attach(GTK_TABLE(table),button,3,4,2,3,GTK_FILL,GTK_FILL,0,0);
    g_signal_connect(G_OBJECT(button),"clicked",G_CALLBACK(on_face_button),(gpointer)12);

    gtk_widget_show_all(face_window);
}

//record function
void create_record_window(){
    GtkWidget * record_textview;
    GtkWidget *left_box, *right_box, *middle_box,*box;
    GtkWidget *label;
    GdkPixbuf *pixbuf = NULL; 
    GdkBitmap *bitmap = NULL; 
    GdkPixmap *pixmap = NULL;

    record_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request(record_window, 250, 450 );
    gtk_window_set_decorated( GTK_WINDOW(record_window), FALSE );
    gtk_window_set_resizable( GTK_WINDOW(record_window), FALSE );
    //	gtk_window_set_position( GTK_WINDOW(record_window), GTK_WIN_POS_CENTER );
    gtk_window_move(GTK_WINDOW(record_window),win_posx+400, win_posy);
    gtk_widget_set_app_paintable(record_window, TRUE); 
    gtk_widget_realize(record_window);
    gtk_widget_add_events(record_window, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(record_window), "button-press-event",G_CALLBACK(onMove), NULL);
    pixbuf = gdk_pixbuf_new_from_file("chatwindow.png", NULL);       // gdk函数读取png文件 
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap, 128);    // alpha小于128认为透明 
    gtk_widget_shape_combine_mask(record_window, bitmap, 0, 0);          // 设置透明蒙板 
    gdk_window_set_back_pixmap(record_window->window, pixmap, FALSE);          // 设置窗口背景 
    g_object_unref(pixbuf); 
    g_object_unref(bitmap); 
    g_object_unref(pixmap);

    box = gtk_hbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(record_window),box);

    left_box = gtk_vbox_new(FALSE,0);
    middle_box = gtk_vbox_new(FALSE,0);
    right_box = gtk_vbox_new(FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),left_box,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(box),middle_box,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(box),right_box,TRUE,TRUE,0);

    record_textview = gtk_text_view_new();
    gtk_widget_set_size_request(record_textview,200,380);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(record_textview),10);/*设置左边距*/
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(record_textview),10);/*设置右边距*/
    Record_buffer =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(record_textview));
    GtkTextIter start,end;
    int inText=open( username, O_RDONLY|O_CREAT, S_IRUSR|S_IWUSR );
    if(inText==-1){ 
        printf("can't open\n ");
    } 
    else{ 
        gchar buffer[2048];
        bzero(buffer,2048);
        while(read(inText,buffer,2048)>0){
            gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Record_buffer),&start,&end);}
        gtk_text_buffer_insert(Record_buffer,&end,buffer,-1);
    }

    label = gtk_label_new("Chat  Record");

    //	gtk_container_add(GTK_CONTAINER(record_window),record_textview);
    gtk_box_pack_start(GTK_BOX(middle_box),label,TRUE,TRUE,20);
    gtk_box_pack_start(GTK_BOX(middle_box),record_textview,TRUE,TRUE,10);
    gtk_widget_show_all(record_window);
}

void control_rec_scrolled_win(){
    int line_to_index=100;
    int line_num=100;
    GtkAdjustment *adj;
    gdouble lower_value, upper_value, page_size, max_value, line_height, to_value;
    adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(Rcv_scrolled_win));
    lower_value = gtk_adjustment_get_lower(adj);
    upper_value = gtk_adjustment_get_upper(adj);
    page_size = gtk_adjustment_get_page_size(adj);
    max_value = upper_value - page_size;
    line_height = upper_value/line_num;
    to_value = line_height*line_to_index;
    if (to_value < lower_value)
        to_value = lower_value;
    if (to_value > max_value)
        to_value = max_value;
    gtk_adjustment_set_value(adj, to_value+10);
}

void on_record_button(){
    if( recordWindowVisible ){
        gtk_window_get_position(GTK_WINDOW( chat_window ), &win_posx, &win_posy);
        create_record_window();
        recordWindowVisible--;
    }
    else{
        gtk_widget_destroy(record_window);
        recordWindowVisible++;
    }	
}

void Clear_Local_message(){
    GtkTextIter start,end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Send_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
    gtk_text_buffer_delete(GTK_TEXT_BUFFER(Send_buffer),&start,&end);/*插入到缓冲区*/
}

void Get_Local_message(const gchar *text){
    GtkTextIter start,end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Send_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Send_buffer),&end,text,strlen(text));/*插入文本到缓冲区*/
}
void Put_Local_message(const gchar *text){
    time_t lt = time(NULL);
    GtkTextIter start,end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,ctime(&lt),-1);
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,"I say:",-1);/*插入文本到缓冲区*/
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,text,strlen(text));/*插入文本到缓冲区*/
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,"\n",1);/*插入文本到缓冲区*/
}

void put_local_pix( GdkPixbuf *pix){
    GtkTextIter start,end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,"I say:", -1);
    gtk_text_buffer_insert_pixbuf(GTK_TEXT_BUFFER(Rcv_buffer),&end,pix);
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,"\n",1);
    control_rec_scrolled_win();
}

void Put_Remote_message( cJSON * _json_message ){
    GtkTextIter start,end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);/*获得缓冲区开始和结束位置的Iter*/
    char usertime[ bufsize ];
    char * _username = cJSON_GetObjectItem( _json_message, "username" )->valuestring; 
    char * _sendtime = cJSON_GetObjectItem( _json_message, "sendtime" )->valuestring; 
    char * _content = cJSON_GetObjectItem( _json_message, "content" )->valuestring;
    if( chatWindowVisible )notifyMessage( _username, _content );
    sprintf( usertime, "%s %s said:", _sendtime, _username );
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end, usertime, strlen( usertime ));/*插入文本到缓冲区*/
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end, _content ,strlen( _content ));/*插入文本到缓冲区*/
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,"\n",1);/*插入文本到缓冲区*/
    control_rec_scrolled_win();
    control_rec_scrolled_win();
    control_rec_scrolled_win();
    control_rec_scrolled_win();
}
void Show_Err(char *err){
    GtkTextIter start,end;
    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Rcv_buffer),&start,&end);
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(Rcv_buffer),&end,err,strlen(err));
}
void on_send(GtkButton *SaveButton, GtkWidget *Send_textview){
    GtkTextIter start,end;/*定义迭代器起点终点*/
    gchar *S_text,*R_text;/*定义文字存储变量*/

    gtk_text_buffer_get_bounds(GTK_TEXT_BUFFER(Send_buffer),&start,&end);
    R_text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(Send_buffer),&start,&end,FALSE);

    if(strcmp(R_text,"")!=0){
        Clear_Local_message();
        Put_Local_message(R_text);    
        time_t lt = time( NULL );
        char send_buffer[ bufsize ];
        cJSON * json_message = cJSON_CreateObject();
        cJSON_AddNumberToObject( json_message, "header", CHAT_MESSAGE );
        cJSON_AddStringToObject( json_message, "username", username );
        cJSON_AddStringToObject( json_message, "sendtime", ctime(&lt) );
        cJSON_AddStringToObject( json_message, "content", R_text );
        strcpy( send_buffer, cJSON_Print( json_message ) );
        sendMessageTo( &sendfd, send_buffer, &dest_addr, strlen( send_buffer ) + 1 );
        savebuffer( R_text );
    }
    else
        Show_Err("消息不能为空...\n");
    free(R_text);
    control_rec_scrolled_win();
    control_rec_scrolled_win();
}
void on_close(GtkButton *CloseButton,GtkWidget *Send_textview){
    gtk_widget_destroy( chat_window );
    gtk_widget_destroy( record_window );
    chatWindowVisible++;
}

gboolean onMove (GtkWidget* widget,GdkEventButton * event, GdkWindowEdge edge){
    if (event->type == GDK_BUTTON_PRESS){
        if (event->button == 1) 
            gtk_window_begin_move_drag(GTK_WINDOW(gtk_widget_get_toplevel(widget)),event->button,event->x_root,event->y_root,event->time);
    }
    return FALSE;
}

void createChatWindow(){
    GtkWidget *Send_scrolled_win/*定义发送滚动窗口*/;
    GtkWidget *box,*chat_box,*close_box;/*定义垂直盒子*/
    GtkWidget *left_box,*right_box,*middle_box;
    GtkWidget *Button_Box1/*定义按钮盒*/,
              *Button_Box2,
              *SaveButton/*定义保存按钮*/,
              *CloseButton/*定义关闭按钮*/,
              *FileButton,*FontButton,*ColorButton,*FaceButton,
              *RecordButton,*close;
    GtkWidget *font_image,*color_image,*face_image,*file_image;
    GtkAccelGroup *gag;    
    GdkPixbuf *pixbuf = NULL; 
    GdkBitmap *bitmap = NULL; 
    GdkPixmap *pixmap = NULL;

    chat_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);/*生成主窗口*/

    gtk_window_set_title(GTK_WINDOW(chat_window),"Save Text");/*设置主窗口标题*/
    gtk_container_set_border_width(GTK_CONTAINER( chat_window ),10);/*设置主窗口边框*/
    gtk_widget_set_size_request(chat_window,400,450);/*设置主窗口初始化大小*/
    gtk_window_set_position(GTK_WINDOW(chat_window),GTK_WIN_POS_CENTER);/*设置主窗口初始位置*/
    gtk_window_set_decorated( GTK_WINDOW(chat_window), FALSE );
    gtk_window_set_resizable( GTK_WINDOW(chat_window), FALSE );
    gtk_window_set_position( GTK_WINDOW(chat_window), GTK_WIN_POS_CENTER );
    gtk_widget_set_app_paintable(chat_window, TRUE); 
    gtk_widget_realize(chat_window);
    gtk_window_get_position(GTK_WINDOW(chat_window), &win_posx, &win_posy );
    gtk_widget_add_events(chat_window, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(chat_window), "button-press-event",G_CALLBACK(onMove), NULL);

    pixbuf = gdk_pixbuf_new_from_file("./layout/chatwindow.png", NULL);       // gdk函数读取png文件 
    gdk_pixbuf_render_pixmap_and_mask(pixbuf, &pixmap, &bitmap, 128);    // alpha小于128认为透明 
    gtk_widget_shape_combine_mask(chat_window, bitmap, 0, 0);          // 设置透明蒙板 
    gdk_window_set_back_pixmap(chat_window->window, pixmap, FALSE);          // 设置窗口背景 

    g_object_unref(pixbuf); 
    g_object_unref(bitmap); 
    g_object_unref(pixmap);

    /*------------------------------设置Send_text view-------------------------*/
    Send_textview = gtk_text_view_new();/*生成text view*/
    gtk_widget_set_size_request(Send_textview,360,40);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(Send_textview),GTK_WRAP_WORD);/*处理多行显示的模式*/
    gtk_text_view_set_justification(GTK_TEXT_VIEW(Send_textview),GTK_JUSTIFY_LEFT);/*控制文字显示方向的,对齐方式*/
    gtk_text_view_set_editable(GTK_TEXT_VIEW(Send_textview),TRUE);/*允许text view内容修改*/
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(Send_textview),TRUE);/*设置光标可见*/
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(Send_textview),5);/*设置上行距*/
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(Send_textview),5);/*设置下行距*/
    gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(Send_textview),5);/*设置词距*/
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(Send_textview),10);/*设置左边距*/
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(Send_textview),10);/*设置右边距*/
    Send_buffer =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(Send_textview));/*返回text view被显示的buffer*/
    /*------------------------------设置Rcv_text view-------------------------*/
    Rcv_textview = gtk_text_view_new();/*生成text view*/
    gtk_widget_set_size_request(Rcv_textview,360,260);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(Rcv_textview),GTK_WRAP_WORD);/*处理多行显示的模式*/
    gtk_text_view_set_justification(GTK_TEXT_VIEW(Rcv_textview),GTK_JUSTIFY_LEFT);/*控制文字显示方向的,对齐方式*/
    gtk_text_view_set_editable(GTK_TEXT_VIEW(Rcv_textview),TRUE);/*允许text view内容修改*/
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(Rcv_textview),TRUE);/*设置光标可见*/
    gtk_text_view_set_pixels_above_lines(GTK_TEXT_VIEW(Rcv_textview),5);/*设置上行距*/
    gtk_text_view_set_pixels_below_lines(GTK_TEXT_VIEW(Rcv_textview),5);/*设置下行距*/
    gtk_text_view_set_pixels_inside_wrap(GTK_TEXT_VIEW(Rcv_textview),5);/*设置词距*/
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(Rcv_textview),10);/*设置左边距*/
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(Rcv_textview),10);/*设置右边距*/
    gtk_text_view_set_editable(GTK_TEXT_VIEW(Rcv_textview),FALSE);/*设置接收文字区不可被编辑*/
    Rcv_buffer =  gtk_text_view_get_buffer(GTK_TEXT_VIEW(Rcv_textview));/*返回text view被显示的buffer*/    
    /*------------------------------设置发送窗口滚动条-------------------------------*/
    Send_scrolled_win = gtk_scrolled_window_new(NULL,NULL);/*生成滚动条的窗口*/
    //    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(Send_scrolled_win),Send_textview);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Send_scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);/*滚动条属性*/
    /*------------------------------设置接收窗口滚动条-------------------------------*/
    Rcv_scrolled_win = gtk_scrolled_window_new(NULL,NULL);/*生成滚动条的窗口*/
    //   gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(Rcv_scrolled_win),Rcv_textview);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Rcv_scrolled_win),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);/*滚动条属性*/
    /*------------------------------设置垂直盒子------------------------------*/
    box = gtk_vbox_new(FALSE,0);/*生成一个垂直排布的盒子*/
    close_box = gtk_hbox_new(FALSE,0);
    chat_box = gtk_vbox_new(FALSE,0);
    left_box = gtk_vbox_new(FALSE,0);
    middle_box = gtk_vbox_new(FALSE,0);
    right_box = gtk_vbox_new(FALSE,0);

    /*------------------------------设置按钮盒子----------------------------    --*/
    Button_Box1 = gtk_hbutton_box_new();/*生成按钮盒*/
    gtk_box_set_spacing(GTK_BOX(Button_Box1),1);/*按钮之间的间隔*/
    gtk_button_box_set_layout(GTK_BUTTON_BOX(Button_Box1),GTK_BUTTONBOX_END);    /*按钮盒内部布局，风格是尾对齐*/
    gtk_container_set_border_width(GTK_CONTAINER(Button_Box1),5);/*边框宽*/

    Button_Box2 = gtk_hbutton_box_new();
    gtk_box_set_spacing(GTK_BOX(Button_Box2),2);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(Button_Box2),GTK_BUTTONBOX_START);
    gtk_container_set_border_width(GTK_CONTAINER(Button_Box2),0);
    /*------------------------------设置分割线------------------------------    --*/
    //    hseparator = gtk_hseparator_new();

    gtk_box_pack_start(GTK_BOX(box),left_box,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(box),middle_box,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(box),right_box,TRUE,TRUE,0);

    /*------------------------------设置发送按钮------------------------------*/
    SaveButton = gtk_button_new_with_label("Send");/*生成发送按钮*/
    gag = gtk_accel_group_new();
    gtk_widget_add_accelerator(SaveButton,"clicked",gag,GDK_E,GDK_CONTROL_MASK,GTK_ACCEL_VISIBLE);
    gtk_window_add_accel_group(GTK_WINDOW(chat_window),gag);
    g_signal_connect(G_OBJECT(SaveButton),"clicked",G_CALLBACK(on_send),(gpointer)Send_textview);/*给按钮加上回调函数*/

    RecordButton = gtk_button_new_with_label("Record");
    g_signal_connect(G_OBJECT(RecordButton),"clicked",G_CALLBACK(on_record_button),NULL);

    FileButton = gtk_button_new();
    gtk_widget_set_size_request(FileButton,20,20);
    file_image = gtk_image_new_from_file("./layout/sendfile.png");
    gtk_container_add(GTK_CONTAINER(FileButton),file_image);
    g_signal_connect(G_OBJECT(FileButton),"clicked",G_CALLBACK(open_file),NULL);

    FontButton = gtk_button_new();
    font_image = gtk_image_new_from_file("./layout/font.png");
    gtk_container_add(GTK_CONTAINER(FontButton),font_image);
    g_signal_connect(G_OBJECT(FontButton),"clicked",G_CALLBACK(select_font),NULL);

    ColorButton = gtk_button_new();
    color_image = gtk_image_new_from_file("./layout/color.png");
    gtk_container_add(GTK_CONTAINER(ColorButton),color_image);
    g_signal_connect(G_OBJECT(ColorButton),"clicked",G_CALLBACK(select_color),NULL);

    FaceButton = gtk_button_new();
    face_image = gtk_image_new_from_file("./layout/face.png");
    gtk_container_add(GTK_CONTAINER(FaceButton),face_image);
    g_signal_connect(G_OBJECT(FaceButton),"clicked",G_CALLBACK(select_face),NULL);

    close = gtk_button_new_with_label("x");
    g_signal_connect(G_OBJECT(close),"clicked",G_CALLBACK(on_close),(gpointer)Send_textview);
    /*------------------------------包装所有容器------------------------------*/
    gtk_container_add(GTK_CONTAINER(Rcv_scrolled_win),Rcv_textview);/*包装textview到滚动条窗口*/
    gtk_container_add(GTK_CONTAINER(Send_scrolled_win),Send_textview);/*包装滚动条窗口到主窗口*/
    gtk_box_pack_start(GTK_BOX(chat_box),Rcv_scrolled_win,FALSE,FALSE,10);
    gtk_box_pack_start(GTK_BOX(chat_box),Button_Box2,FALSE,FALSE,2);/*加入一条分割线*/
    gtk_box_pack_start(GTK_BOX(chat_box),Send_scrolled_win,FALSE,FALSE,2);/*包装滚动条窗口到主窗口*/ 
    gtk_box_pack_end(GTK_BOX(close_box),close,FALSE,FALSE,0);   
    gtk_box_pack_end(GTK_BOX(middle_box),Button_Box1,TRUE,TRUE,5);/*把按钮盒包装到vbox中*/
    gtk_box_pack_end(GTK_BOX(middle_box),chat_box,TRUE,TRUE,10);
    gtk_box_pack_end(GTK_BOX(middle_box),close_box,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(Button_Box1),RecordButton,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(Button_Box1),SaveButton,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(Button_Box2),FontButton,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(Button_Box2),ColorButton,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(Button_Box2),FaceButton,TRUE,TRUE,5);
    gtk_box_pack_start(GTK_BOX(Button_Box2),FileButton,TRUE,TRUE,5);
    gtk_container_add(GTK_CONTAINER(chat_window),box);/*将盒子封装到主窗口中去*/    

    gtk_widget_show_all(chat_window);/*显示所有东西*/
}


