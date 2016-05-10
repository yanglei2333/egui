# include <string.h>
# include <unistd.h>
# include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

# include "application.h"
# include "widget.h"
#include "log.h"

#define FILE_MAX  40000
struct window * w = NULL;
char* file_name = NULL;
struct image_view* i = NULL;
si_t width_screen=0;
si_t height_screen=0;

si_t window_callback(addr_t w, addr_t m)
{
    struct window * self = w;
    union message * msg = m;
    //window_default_callback(w, m);
    switch(msg->base.type)
    {
        case MESSAGE_TYPE_WINDOW_MAXIMIZE:
        /*
            printf("MESSAGE_TYPE_WINDOW_MAXIMIZE\n");
        */
            height_screen=self->area.height;
            width_screen=self->area.width;
            image_view_set_bounds(i, 10, 10,width_screen-20,height_screen-20);
            break;
        case MESSAGE_TYPE_WINDOW_RESTORE:
        /*
            printf("MESSAGE_TYPE_WIDGET_RESIZE\n");
        */
            image_view_set_bounds(i, 10, 10, 500, 500);
            break;

        default:
            window_default_callback(w, m);
            break;
    }
    return 0;
}

int main(int argc, char* argv[])
{
    si_t video_access_mode = VIDEO_ACCESS_MODE_BUFFER;
	si_t app_type = APPLICATION_TYPE_NORMAL;
    /**
     * open with file
     **/
    if(argc == 2)
	{
         file_name=argv[1];
    }
    else if(argc != 1)
    {
		EGUI_PRINT_ERROR("wrong parameter!\nusage: \nediterbasic\nor:\nediterbasic [filename]");
		return -1;
    }


    /* 初始化用户应用程序 */
    application_init(video_access_mode, app_type, "image_view");

    /* 申请窗口 */
    w = window_init("image_view");
    /* 申请失败 */
    if(w == NULL)
    {
        application_exit();
        return -1;
    }
	window_set_bounds(w, 200, 100, 500, 500);
    w->callback=window_callback;
		/* 默认加载图片 */
      if(file_name == NULL)
      {
         i = image_view_init("/home/yanglei/egui/resource/icons/desktop/desktop44.bmp");
      }
      else
      {
          i= image_view_init(file_name);
      }
	if(NULL == i)
	{
		application_exit();
		return -1;
	}
	image_view_set_bounds(i, 10, 10, 500, 500);


	/* 添加子控件 */
    object_attach_child(OBJECT_POINTER(w), OBJECT_POINTER(i));


    /* 添加顶层窗口 */
    application_add_window(NULL, w);
    /* 设置主窗口 */
    application_set_main_window(w);

    /* 运行 */
    application_exec();

    return 0;
}
