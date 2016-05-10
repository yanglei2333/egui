/**
 * @file
 *
 * @author Dalian University of Technology
 *
 * @section LICENSE
 *
 * Copyright (C) 2010 Dalian University of Technology
 *
 * This file is part of EDUGUI.
 *
 * EDUGUI is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * EDUGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with EDUGUI; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * All rights reserved.
**/

# include <assert.h>
# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include <time.h>

# include <unistd.h>

# include "directory_item.h"
# include "directory_content.h"
# include "my_widget.h"

#include "widget.h"
#include "client_lib.h"
#include "compiler.h"
#include "application.h"

#define DESKTOP_PATH  "/mnt/hgfs/gui/cece/c_egui_desktop/"

static struct color barely_blue = {0xcb, 0xf3, 0xfb, 0};
static struct color light_blue = {0x46, 0xa5, 0xe5, 0};
static struct color white={0xff,0xff,0xff,0};
char copy_path[200];
char delete_path[200];

char file_str[50];
char file_str1[50];
char file_str2[50];
char file_str3[50];


si_t right_click_flag = 0;/////////////////////////
si_t cut_paste_flag = 0;
si_t right_click_x =0;
si_t right_click_y =0;
si_t cur_file_index=0;
si_t cur_directory_index=0;
si_t left_or_right=1;//right=1,left=0
si_t file_or_directory=1;//file=1,directory=0

struct window* save_window = NULL;
struct text_line* save_text_line = NULL;
struct my_widget * mw_after_save;
si_t save_or_rename_flag=0;//0:save file 1:save directory 2:rename

si_t num =4;
si_t icon_flag=1;
si_t
my_widget_default_widget_show
(struct my_widget * mw,
 union message * msg)
{
    struct rectangle area;
    NOT_USED(msg);

    application_widget_absolute_area(WIDGET_POINTER(mw), &area);

    /* 设置区域 */
    set_area
    (mw->gd,
     area.x,
     area.y,
     area.width,
     area.height);

    /* 更新 */
    update(mw->gd);

    return 0;
}

si_t get_type(char * str)
{
    si_t len=strlen(str);
    si_t i=len-1;
    for(;i>0;i--)
    {
       if(str[i]=='.')
         break;
    }
    if(i==0)
    return 0;
    char key[20] = {'\0'};
    strncpy(key, str+i+1, len-1-i);
    if(strcmp(key,"txt")==0)
    return 1;
    if(strcmp(key,"h")==0)
    return 2;
    if(strcmp(key,"c")==0)
    return 3;
    if(strcmp(key,"cpp")==0)
    return 4;
    if(strcmp(key,"bmp")==0)
    return 5;
    if(strcmp(key,"zip")==0)
    return 6;
    return 0;
}

void get_icon_dir_path(char * icon_path)
{
   if(icon_flag)
   strcpy(icon_path,"/home/yanglei/egui/resource/icons/file_icon/");
   else
   strcpy(icon_path,"/home/yanglei/egui/resource/icons/file_icon2/");
}


si_t refresh(struct my_widget * mw)
{
    vector_clear(&file_list);

    directory_content(working_directory, &file_list);

    mw->start = 0;

    scroll_bar_set_view(s, 90 * ((vector_size(&file_list)-1)/num+2), 0);
    scroll_bar_update_offset(s, 0);
    scroll_bar_repaint(s);
    scroll_bar_show(s);

    my_widget_repaint(mw);
    my_widget_show(mw);
    return 0;
}

si_t save_window_ok_button_callback(void* btn, void* msg)
{
    union message* m = (union message*)msg;
    char save_file_name[100];
    struct directory_item * di_ptr;
    pid_t pid;
	switch(m->base.type)
	{
    case MESSAGE_TYPE_MOUSE_SINGLE_CLICK:
		/*
		if(0 == save_file(text_line_get_buf(save_text_line), text_line_get_buf(file_context_text_line)))
		{
			sprintf(log_label->text, "save successfully!");
		}
		else
		{
			sprintf(log_label->text, "failed to save file!");
		}*/
		sprintf(save_file_name, "%s", text_line_get_buf(save_text_line));
		if((pid = fork()) == 0)
        {
            switch(save_or_rename_flag)
            {
                case 0:
                    execl("/bin/touch","touch",save_file_name,NULL);
                    break;
                case 1:
                    execl("/bin/mkdir","mkdir",save_file_name,NULL);
                    break;
                case 2:
                    di_ptr = vector_at(&file_list, cur_file_index );
                    execl("/bin/mv","mv",di_ptr->name+2,save_file_name,NULL);
                    break;
                default:
                    break;
            }
            exit(0);
        }
        usleep(100000);
        application_del_window(save_window);
		save_window = NULL;
		save_text_line = NULL;
		break;
    default:
        button_default_callback(btn, msg);
        return 0;
        break;
	}
	refresh(mw_after_save);
    return 0;
}

si_t pop_window(struct my_widget * mw)
{
    struct button* b = NULL;
    si_t x,y;
    widget_absolute_coordinate(WIDGET_POINTER(mw), &x, &y);
    mw_after_save=mw;
    switch(save_or_rename_flag)
    {
        case 0:
            save_window = window_init("save_file_window");
            break;
        case 1:
            save_window = window_init("save_directory_window");
            break;
        case 2:
            save_window = window_init("rename_window");
            break;
        default:
            break;
     }
    if(NULL == save_window)
    {
        //EGUI_PRINT_ERROR("failed to init save window");
        application_exit();
        return -1;
    }
    window_set_bounds(save_window,x + 30, y + 30, 300, 60);
    window_set_color(save_window, NULL, &barely_blue);
    save_text_line = text_line_init(100, 0);
    if(NULL == save_text_line)
    {
        //EGUI_PRINT_ERROR("failed to init save_text_line on save window");
        application_exit();
        return -1;
    }
    text_line_set_bounds(save_text_line, 5, 5, 230, 50);
    text_line_set_placeholder(save_text_line, "input file name");
    text_line_set_color(save_text_line, &light_blue, NULL, &light_blue );
    b = button_init("OK");
    if(NULL == b)
	{
	    //EGUI_PRINT_ERROR("failed to init ok button on save window");
        application_exit();
        return -1;
	}
    button_set_bounds(b, 240, 10, 50, 40);
    button_set_color(b, &barely_blue, &light_blue );
    button_set_font(b, FONT_MATRIX_12);
    b->callback = save_window_ok_button_callback;

    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(b));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(save_text_line));
    application_add_window(NULL, save_window);
    return 0;

}

si_t pop_window2(struct my_widget * mw)
{
    struct label * name_label = NULL;
    struct label * category_label = NULL;
    struct label * size_label = NULL;
    struct label * path_label = NULL;
    struct label * atime_label = NULL;
    struct label * mtime_label = NULL;
    struct image_view* i = NULL;
    struct directory_item * di_ptr;
    struct tm *ti;

    si_t x,y;
    widget_absolute_coordinate(WIDGET_POINTER(mw), &x, &y);
    //mw_after_save=mw;
    save_window = window_init("file_attribute_window");
    if(NULL == save_window)
    {
        //EGUI_PRINT_ERROR("failed to init save window");
        application_exit();
        return -1;
    }
    window_set_bounds(save_window,x + 30, y + 30, 400, 340);
    window_set_color(save_window, NULL, &white);
    i = image_view_init("/home/yanglei/egui/resource/icons/right_click/attribute_1.bmp");
    image_view_set_bounds(i, 10, 5, 166, 330);

    if(file_or_directory==0)
    {
        struct vector new_file_list;
        char new_path[PATH_MAX];
        char *p=strrchr(working_directory,'/');
        strcpy(file_str,p+1);
        chdir("..");
        getcwd(new_path, PATH_MAX);
        /*strncpy(new_path,working_directory,p-working_directory);
        new_path[p-working_directory]=0;*/
        vector_init(&new_file_list);
        directory_content(new_path, &new_file_list);
        si_t n = (si_t)vector_size(&new_file_list);
        si_t ii;
        for(ii = 0; ii <n;ii++)
        {
            di_ptr = vector_at(&new_file_list, ii);
            if(strcmp(file_str,di_ptr->name+2)==0)
            break;
        }
        if(ii>n-1)
        return 0;
        chdir(file_str);
        name_label = label_init(file_str);
        category_label = label_init("(Directory)");
        path_label = label_init(new_path);
    }
    else
    {
       di_ptr = vector_at(&file_list, cur_file_index );
       sprintf(file_str, "%s", di_ptr->name+2);
       name_label = label_init(di_ptr->name+2);
       if(di_ptr->is_directory==1)
	      strcpy(file_str,"(Directory)");
	   else
        {
          strcpy(file_str,strrchr(file_str,'.')+1);
          strcat(file_str," (File)");
        }
       category_label = label_init(file_str);
       path_label = label_init(working_directory);
       //category_label = label_init("(File)");

    }
    label_set_bounds(name_label, 185, 32, 205, 20);
	label_set_color(name_label, NULL, &white);
	label_set_font(name_label, FONT_MATRIX_10);

	//strcpy(file_str,working_directory);
	//strcpy(a,"ddddddd");
	/*if(di_ptr->is_directory==1)
	strcpy(file_str,"(Directory)");
	else
	{
        si_t it=0;
	    si_t item=strlen(file_str);
        for(;it<item;it++)
        {
          if(file_str[it]=='.')
          break;
        }
	    sprintf(file_str, "%s", file_str+it+1);
	    strcat(file_str," (File)");
	}
	category_label = label_init(file_str);*/
	label_set_bounds(category_label, 185,84, 205, 20);
	label_set_color(category_label, NULL, &white);
	label_set_font(category_label, FONT_MATRIX_10);

	//show_text(save_window->gd,185,90,file_str,strlen(file_str));

	sprintf(file_str1,"%d", di_ptr->file_size);
	strcat(file_str1," B");
	size_label = label_init(file_str1);
	label_set_bounds(size_label, 185, 136, 205, 20);
	label_set_color(size_label, NULL, &white);
	label_set_font(size_label, FONT_MATRIX_10);

	label_set_bounds(path_label, 185, 188, 205, 20);
	label_set_color(path_label, NULL, &white);
	if(strlen(working_directory)>20)
	label_set_font(path_label, FONT_MATRIX_08);
	else
	label_set_font(path_label, FONT_MATRIX_10);

	ti=localtime(&(di_ptr->a_time));
	sprintf(file_str2, "%d-%d-%d %d:%d:%d", ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec);
	atime_label = label_init(file_str2);
	label_set_bounds(atime_label, 185, 240, 205, 20);
	label_set_color(atime_label, NULL, &white);
	label_set_font(atime_label, FONT_MATRIX_10);

	ti=localtime(&(di_ptr->m_time));
	sprintf(file_str3, "%d-%d-%d %d:%d:%d", ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec);
	mtime_label = label_init(file_str3);
	label_set_bounds(mtime_label, 185, 292, 205, 20);
	label_set_color(mtime_label, NULL, &white);
	label_set_font(mtime_label, FONT_MATRIX_10);


    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(i));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(name_label));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(category_label));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(size_label));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(path_label));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(atime_label));
    object_attach_child(OBJECT_POINTER(save_window), OBJECT_POINTER(mtime_label));
    application_add_window(NULL, save_window);
    return 0;

}

si_t rename_file(struct my_widget * mw)
{
    return pop_window(mw);
}

si_t build_file(struct my_widget * mw)
{
    return pop_window(mw);
}

si_t show_file_attribute(struct my_widget * mw)
{
    return pop_window2(mw);
}

si_t fill_icon(struct my_widget * mw,struct directory_item * di_ptr)
{
   char icon_path[200];
   get_icon_dir_path(icon_path);
   if(di_ptr->is_directory == 1)
   {
      strcat(icon_path,"dir2_1.bmp");
   }
   else if(di_ptr->is_directory == 2)
   {
      strcat(icon_path,"ex_1.bmp");
   }
   else
   {
      switch(get_type(di_ptr->name))
      {
          case 1:
                 strcat(icon_path,"txt_1.bmp");
                 break;
          case 2:
                 strcat(icon_path,"h_1.bmp");
                 break;
          case 3:
                 strcat(icon_path,"c_1.bmp");
                 break;
          case 4:
                 strcat(icon_path,"cpp_1.bmp");
                 break;
          case 5:
                 strcat(icon_path,"bmp_1.bmp");
                 break;
          case 6:
                 strcat(icon_path,"zip_1.bmp");
                 break;
          default:
                 strcat(icon_path,"unknow_1.bmp");
                 break;
       }
    }
   draw_img(mw->gd,icon_path, ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
   return 0;

}

si_t fill_icon_2(struct my_widget * mw,struct directory_item * di_ptr)
{
   char icon_path[200];
   get_icon_dir_path(icon_path);
   if(di_ptr->is_directory == 1)
   {
      strcat(icon_path,"dir2_2.bmp");
   }
   else if(di_ptr->is_directory == 2)
   {
      strcat(icon_path,"ex_2.bmp");
   }
   else
   {
      switch(get_type(di_ptr->name))
      {
          case 1:
                 strcat(icon_path,"txt_2.bmp");
                 break;
          case 2:
                 strcat(icon_path,"h_2.bmp");
                 break;
          case 3:
                 strcat(icon_path,"c_2.bmp");
                 break;
          case 4:
                 strcat(icon_path,"cpp_2.bmp");
                 break;
          case 5:
                 strcat(icon_path,"bmp_2.bmp");
                 break;
          case 6:
                 strcat(icon_path,"zip_2.bmp");
                 break;
          default:
                 strcat(icon_path,"unknow_2.bmp");
                 break;
               }
           }
   draw_img(mw->gd,icon_path, ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
   return 0;
}

si_t open_file(struct my_widget * mw)//////////////////////////////////
{
    struct directory_item * di_ptr;
    di_ptr = vector_at(&file_list,cur_file_index);
    if(di_ptr->is_directory == 1)
        {
            cur_directory_index=cur_file_index;
            chdir(di_ptr->name + 2);

            getcwd(working_directory, PATH_MAX);

            vector_clear(&file_list);

            directory_content(working_directory, &file_list);

            mw->start = 0;

            scroll_bar_set_view(s, 90 * ((vector_size(&file_list)-1)/num+2), 0);
            scroll_bar_update_offset(s, 0);
            scroll_bar_repaint(s);
            scroll_bar_show(s);

            label_repaint(l);
            label_show(l);

            my_widget_repaint(mw);
            my_widget_show(mw);
        }
        /**
         * executable regular file
         **/
        else if(di_ptr->is_directory == 2)
        {
            char* command[2];
            char all_path[PATH_MAX];
            pid_t pid;
            command[0] = di_ptr->name + 2;
            command[1] = NULL;
            strncpy(all_path, working_directory, PATH_MAX);
            strncat(all_path, di_ptr->name + 2, PATH_MAX);
            if((pid = fork()) == 0)
            {
                execv(command[0], command);
                exit(0);
            }
        }
        else if(di_ptr->is_directory == 3)
        {
            pid_t pid;
            if(get_type(di_ptr->name)==5)
           {
               char file_name[100];
               si_t item=0;
               strncpy(file_name,working_directory,100);
               item=strlen(file_name);
               file_name[item]='/';
               file_name[item+1]='\0';
               strncat(file_name,di_ptr->name+2,100);
               if((pid = fork()) == 0)
              {
                //set_color(mw->gd, 0, 0, 0, 0);
                execl("/home/yanglei/egui/_bulid/debug/samples/image_view","./image_view",file_name,NULL);
                exit(0);
              }

           }

            else
           {
               if((pid = fork()) == 0)
              {
                set_color(mw->gd, 0, 0, 0, 0);
                //execv(command[0], command);
                 // execl("/home/yanglei/egui/_build/debug/samples/editerbasic","./editerbasic",NULL);
                  execl("/home/yanglei/egui/_bulid/debug/samples/editerbasic","./editerbasic",di_ptr->name+2,NULL);
                  //execl(command[0],command);
                exit(0);
              }
           }
        }
        return 0;
}

si_t uncompress_file(struct directory_item * di_ptr)
{
    if(copy_path != NULL)
    {
        pid_t pid;
        if((pid = fork()) == 0)
        {
            execl("/usr/bin/unzip", "unzip",di_ptr->name+2, NULL);
            exit(0);
        }
    }
    usleep(400000);
    return 0;
}

si_t compress_file(struct directory_item * di_ptr)
{
    char str[100];
    char new_file_name[100];
    si_t item=0;
    strcpy(str,di_ptr->name+2);
    item=strlen(str);
    si_t i=0;
    for(;i<item;i++)
    {
       if(str[i]=='.')
         break;
    }
    strncpy(new_file_name,str,i);
    strcat(new_file_name,".zip");
    if(copy_path != NULL)
    {
        pid_t pid;
        if((pid = fork()) == 0)
        {
            execl("/usr/bin/zip", "zip","-r", new_file_name, di_ptr->name+2, NULL);
            exit(0);
        }
    }
    usleep(400000);
    return 0;
}

si_t delete_file(struct my_widget * mw,struct directory_item * di_ptr)
{
     pid_t pid;
        if((pid = fork()) == 0)
        {
            if(di_ptr->is_directory == 1)
               execl("/bin/rm", "rm", "-r",di_ptr->name + 2, NULL);
            else
               execl("/bin/rm", "rm", di_ptr->name + 2, NULL);
            exit(0);
        }
    usleep(100000);
    return 0;
}

si_t delete_file_by_path(struct my_widget * mw,struct directory_item * di_ptr)
{
    pid_t pid;
    if((pid = fork()) == 0)
    {
        if(di_ptr->is_directory == 1)
            execl("/bin/rm", "rm", "-r",copy_path, NULL);
        else
            execl("/bin/rm", "rm", copy_path, NULL);
        exit(0);
    }
    usleep(100000);
    return 0;
}

si_t cut_file(struct directory_item * di_ptr)
{
     strcpy(copy_path,working_directory);
     strncat(copy_path,"/",1);
     strncat(copy_path,di_ptr->name + 2,100);
     cut_paste_flag=2;
     return 0;
}

si_t copy_file(struct directory_item * di_ptr)
{
     strcpy(copy_path,working_directory);
     strncat(copy_path,"/",1);
     strncat(copy_path,di_ptr->name + 2,100);
     cut_paste_flag=1;
     return 0;
}

si_t paste_file(struct my_widget * mw,struct directory_item * di_ptr)
{
    if(copy_path != NULL)
    {
        pid_t pid;
        if((pid = fork()) == 0)
        {
            execl("/bin/cp", "cp", "-a", copy_path,working_directory, NULL);
            exit(0);
        }
    }
    usleep(100000);
    return 0;
}

si_t
my_widget_default_widget_repaint
(struct my_widget * mw,
 union message * msg)
{
    struct directory_item * di_ptr;
    struct rectangle area;
    si_t x, y, font_height, i, n, x1, y1, temp;
    NOT_USED(msg);

    /* 获得左上角的绝对坐标 */
    widget_absolute_coordinate(WIDGET_POINTER(mw), &x, &y);

    /* 获得绝对的工作区域 */
    /* 将会舍弃不在父控件内的部分*/
    application_widget_absolute_area(WIDGET_POINTER(mw), &area);

    /* 设置区域 */
    set_area
    (mw->gd,
     area.x,
     area.y,
     area.width,
     area.height);

    set_color
    (mw->gd,
     mw->back_color.r,
     mw->back_color.g,
     mw->back_color.b,
     mw->back_color.a);

    fill_rectangle
    (mw->gd,
     area.x,
     area.y,
     area.width,
     area.height);

    /**
     * draw frame
     **/
    set_color
    (mw->gd,
     mw->frame_color.r,
     mw->frame_color.g,
     mw->frame_color.b,
     mw->frame_color.a);
    draw_rectangle
    (mw->gd,
     area.x,
     area.y,
     area.width,
     area.height);
    draw_rectangle
    (mw->gd,
     area.x + 1,
     area.y + 1,
     area.width - 2,
     area.height - 2);

    area.x += 5;
    area.y += 5;
    area.width -= 10;
    area.height -= 10;
    x += 5;
    y += 5;

    set_color
    (mw->gd,
     mw->fore_color.r,
     mw->fore_color.g,
     mw->fore_color.b,
     mw->fore_color.a);

    set_font(mw->gd, mw->font);
    /* 获得字体的高度 */
    font_height = get_font_height(mw->gd);

    n = (si_t)vector_size(&file_list);

    x1 = x ;
    y1 = y ;
    char file_name[15]="";
    /* 目录项开始的索引 */
        temp = mw->start;
    if(n -temp> mw->count)
    n=mw->count;
    else
    n=n -temp;
    for(i = 0; i <n; ++ i)
        {
            memset(file_name,0,sizeof(file_name));
            di_ptr = vector_at(&file_list, temp ++);
            set_area(mw->gd,x1+110*(i%num)+30,y1+90*(i/num)+15,60,58);
            set_color(mw->gd,mw->back_color.r,mw->back_color.g, mw->back_color.b,mw->back_color.a);
            fill_icon(mw,di_ptr);

           set_area(mw->gd,area.x,area.y,area.width,area.height);
           set_color(mw->gd,mw->fore_color.r,mw->fore_color.g,mw->fore_color.b, mw->fore_color.a);
             if(strlen(di_ptr->name)>12)
             {
               strncpy(file_name, di_ptr->name+2, 9);
               strncpy(file_name+9, "...", 3);
             }
             else
                strcpy(file_name, di_ptr->name+2);
            show_text
            (mw->gd,
             x1+110*(i%num)+30-(strlen(file_name) - 7)*4,
             y1+90*(i/num)+60+12,
             file_name,
             strlen(file_name));
        }
    return 0;
}

si_t
my_widget_default_mouse_press
(struct my_widget * mw,
 union message * msg)
{
     struct rectangle area;
    si_t x, y, x1,y1, index;
    struct directory_item * di_ptr;

    if(msg->mouse.code == INPUT_CODE_MOUSE_L_KEY)
    {
        /* 获得左上角的绝对坐标 */
        widget_absolute_coordinate(WIDGET_POINTER(mw), &x, &y);

        /* 获得绝对的工作区域 */
        /* 将会舍弃不在父控件内的部分*/
        application_widget_absolute_area(WIDGET_POINTER(mw), &area);

        y1 = msg->mouse.cursor_position.y - y;
        x1 = msg->mouse.cursor_position.x - x;
        if(right_click_flag!=0)
        {
           return 0;
        }

        my_widget_repaint(mw);
        my_widget_show(mw);
        if(x1<110*((x1-30)/110)+30-1||x1>110*((x1-30)/110)+100-1||y1<90*((y1-15)/90)+15+2||y1>90*((y1-15)/90)+78+2)
           return 0;

        index = (y1-15)/90*num +(x1-30)/110;
        if(index >= (si_t)vector_size(&file_list))
           return 0;
         di_ptr = vector_at(&file_list, index + mw->start);
         set_area(mw->gd,x+5+110*(index%num)+30,y+5+90*(index/num)+15,60,60);
         set_color(mw->gd,mw->back_color.r,mw->back_color.g, mw->back_color.b,mw->back_color.a);
         fill_icon_2(mw,di_ptr);
         set_area(mw->gd,area.x,area.y,area.width,area.height);
    }
    return 0;
}

si_t
my_widget_default_mouse_release
(struct my_widget * mw,
 union message * msg)
{
    struct rectangle area;
    si_t x, y, x1,y1, index;//可以省略x1,x2
    struct directory_item * di_ptr;

    if(msg->mouse.code == INPUT_CODE_MOUSE_R_KEY)
    {
        /* 获得左上角的绝对坐标 */
        widget_absolute_coordinate(WIDGET_POINTER(mw), &x, &y);

        /* 获得绝对的工作区域 */
        /* 将会舍弃不在父控件内的部分*/
        application_widget_absolute_area(WIDGET_POINTER(mw), &area);

        /* 双击了那个目录项 */
        y1 = msg->mouse.cursor_position.y - y;
        x1 = msg->mouse.cursor_position.x - x;
        set_color(mw->gd, 255, 255, 255, 0);

        my_widget_repaint(mw);
        my_widget_show(mw);
        index = (y1-15)/90*num +(x1-30)/110;
        right_click_x= (x1<(area.width - 160)? x1: (area.width-160));

        if(index >= (si_t)vector_size(&file_list)||x1<110*((x1-30)/110)+30-1||x1>110*((x1-30)/110)+100-1||y1<90*((y1-15)/90)+15+2||y1>90*((y1-15)/90)+78+2)/////
        {
            right_click_flag=1;
            right_click_y= (y1<area.height-175 ?y1:area.height -175);
            set_area(mw->gd,x+right_click_x,y+right_click_y,160,175);
            set_color(mw->gd,mw->back_color.r,mw->back_color.g, mw->back_color.b,mw->back_color.a);
           if(cut_paste_flag == 0)
           {
		        draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_2_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
           }
           else
           {
               draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_1_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
           }
        }
        else
        {
            cur_file_index=index+mw->start;
            right_click_flag=2;
            right_click_y= (y1< area.height-243 ?y1:area.height -243);
            set_area(mw->gd,x+right_click_x,y+right_click_y,160,243);
            set_color(mw->gd,mw->back_color.r,mw->back_color.g, mw->back_color.b,mw->back_color.a);
            di_ptr = vector_at(&file_list,cur_file_index);
            if(get_type(di_ptr->name)!=6)
            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
            else
            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
        }


    }
    set_area(mw->gd,area.x,area.y,area.width,area.height);
    return 0;
}

si_t
my_widget_default_mouse_single_click
(struct my_widget * mw,
 union message * msg)
{
    struct rectangle area;
    struct directory_item * di_ptr;
    si_t x, y, x1,y1, index;
    if(msg->mouse.code == INPUT_CODE_MOUSE_L_KEY)
    {
        /* 获得左上角的绝对坐标 */
        widget_absolute_coordinate(WIDGET_POINTER(mw), &x, &y);

        /* 获得绝对的工作区域 */
        /* 将会舍弃不在父控件内的部分*/
        application_widget_absolute_area(WIDGET_POINTER(mw), &area);

        y1 = msg->mouse.cursor_position.y - y;
        x1 = msg->mouse.cursor_position.x - x;
        if(right_click_flag==3)
        {
            set_area(mw->gd,x+right_click_x+160,y+right_click_y+105,160,70);
            right_click_flag=0;
            if(x1>right_click_x&&x1<right_click_x+160&&y1>right_click_y&&y1<right_click_y+175)
            {
                right_click_flag=1;
                my_widget_repaint(mw);
            }
            else if((left_or_right==1&&x1>right_click_x+160&&x1<right_click_x+320||left_or_right==0&&x1>right_click_x-160&&x1<right_click_x)&&y1>right_click_y+105&&y1<right_click_y+175)
            {
                switch((y1-right_click_y-105)/35)
                   {
                        case 0:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/3_1.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            save_or_rename_flag=1;
                            build_file(mw);
                            break;
                        case 1:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/3_2.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            save_or_rename_flag=0;
                            build_file(mw);
                            break;
                        default:
                            break;
                   }
            }
            else
            {
                set_area(mw->gd,area.x,area.y,area.width,area.height);
                my_widget_repaint(mw);
                my_widget_show(mw);
                return 0;
            }
        }

        if(right_click_flag==4)
        {
            set_area(mw->gd,x+right_click_x+160,y+right_click_y,160,105);
            right_click_flag=0;
            if(x1>right_click_x&&x1<right_click_x+160&&y1>right_click_y&&y1<right_click_y+175)
            {
                right_click_flag=1;
                my_widget_repaint(mw);
            }
            else if((left_or_right==1&&x1>right_click_x+160&&x1<right_click_x+320||left_or_right==0&&x1>right_click_x-160&&x1<right_click_x)&&y1>right_click_y&&y1<right_click_y+105)
            {
                switch((y1-right_click_y)/35)
                   {
                        case 0:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/4_1.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            sort_flag=1;
                            refresh(mw);
                            break;
                        case 1:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/4_2.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            sort_flag=2;
                            refresh(mw);
                            break;
                        case 2:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/4_3.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            sort_flag=3;
                            refresh(mw);
                            break;
                        default:
                            break;
                   }
            }
            else
            {
                set_area(mw->gd,area.x,area.y,area.width,area.height);
                my_widget_repaint(mw);
                my_widget_show(mw);
                return 0;
            }
        }
        //my_widget_repaint(mw);
        //my_widget_show(mw);
        if(right_click_flag==1)
        {
            right_click_flag=0;
            if(x1<right_click_x||x1>right_click_x+160||y1<right_click_y||y1>right_click_y+175)
            {
                right_click_flag=0;
                my_widget_repaint(mw);
                my_widget_show(mw);
            }
            else
            {
                set_area(mw->gd,x+right_click_x,y+right_click_y,160,175);
                if(cut_paste_flag==0)
                {
                    switch((y1-right_click_y)/35)
                   {
                        case 0://排序方式
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_2_1.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            if(right_click_x< (area.width-320))
                            {
                               left_or_right=1;
                               set_area(mw->gd,x+right_click_x+160,y+right_click_y,160,105);
                            }
                            else
                            {
                               left_or_right=0;
                               set_area(mw->gd,x+right_click_x-160,y+right_click_y,160,105);
                            }
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/4_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            right_click_flag=4;
                            my_widget_show(mw);
                            break;
                        case 1://刷新
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_2_2.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            refresh(mw);
                            break;
                        case 2://粘贴
                            break;
                        case 3://新建
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_2_4.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            if(right_click_x< (area.width-320))
                            {
                               left_or_right=1;
                               set_area(mw->gd,x+right_click_x+160,y+right_click_y+105,160,70);
                            }
                            else
                            {
                               left_or_right=0;
                               set_area(mw->gd,x+right_click_x-160,y+right_click_y+105,160,70);
                            }
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/3_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            right_click_flag=3;
                            my_widget_show(mw);
                            break;
                        case 4://属性
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_2_5.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            cur_file_index=cur_directory_index;
                            file_or_directory=0;
                            show_file_attribute(mw);
                            break;
                        default:
                            break;
                   }
                }
                else
                {
                    switch((y1-right_click_y)/35)
                   {
                        case 0:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_1_1.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            if(right_click_x< (area.width-320))
                            {
                               left_or_right=1;
                               set_area(mw->gd,x+right_click_x+160,y+right_click_y,160,105);
                            }
                            else
                            {
                               left_or_right=0;
                               set_area(mw->gd,x+right_click_x-160,y+right_click_y,160,105);
                            }
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/4_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            right_click_flag=4;
                            my_widget_show(mw);
                            break;
                        case 1:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_1_2.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            refresh(mw);
                            break;
                        case 2:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_1_3.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            paste_file(mw,di_ptr);
                            if(cut_paste_flag==2)
                            delete_file_by_path(mw,copy_path);
                            refresh(mw);
                            cut_paste_flag=0;
                            break;
                        case 3:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_1_4.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            if(right_click_x< (area.width-320))
                            {
                               left_or_right=1;
                               set_area(mw->gd,x+right_click_x+160,y+right_click_y+105,160,70);
                            }
                            else
                            {
                               left_or_right=0;
                               set_area(mw->gd,x+right_click_x-160,y+right_click_y+105,160,70);
                            }
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/3_0.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            right_click_flag=3;
                            my_widget_show(mw);
                            break;
                        case 4:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/1_1_5.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            cur_file_index=cur_directory_index;
                            file_or_directory=0;
                            show_file_attribute(mw);
                            break;
                        default:
                            break;
                   }
                }

            }

            return 0;
        }
        else if(right_click_flag==2)
        {
            right_click_flag=0;
            if(x1<right_click_x||x1>right_click_x+160||y1<right_click_y||y1>right_click_y+243)
            {
                right_click_flag=0;
                my_widget_repaint(mw);
                my_widget_show(mw);
            }
            else
            {
                set_area(mw->gd,x+right_click_x,y+right_click_y,160,243);
                di_ptr = vector_at(&file_list,cur_file_index);
                if(get_type(di_ptr->name)!=6)
                {
                    switch((y1-right_click_y)/35)
                   {
                        case 0:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_1.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                             set_area(mw->gd,area.x,area.y,area.width,area.height);
                            my_widget_repaint(mw);
                            my_widget_show(mw);
                            open_file(mw);
                            return 0;
                        case 1://剪切
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_2.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            cut_file(di_ptr);
                            break;
                        case 2://复制
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_3.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            copy_file(di_ptr);
                            break;
                        case 3: //删除
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_4.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            delete_file(mw,di_ptr);
                            refresh(mw);
                            break;
                        case 4://重命名
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_5.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            save_or_rename_flag=2;
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            rename_file(mw);
                            break;
                        case 5://压缩
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_6.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            compress_file(di_ptr);
                            refresh(mw);
                            break;
                        case 6://属性
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_1_7.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            file_or_directory=1;
                            show_file_attribute(mw);
                            break;
                        default:
                            break;
                   }
                }
                else
                {
                    switch((y1-right_click_y)/35)
                   {
                        case 0:
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_1.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            my_widget_repaint(mw);
                            my_widget_show(mw);
                            open_file(mw);
                            ;
                        case 1://剪切
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_2.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            cut_file(di_ptr);
                            break;
                        case 2://复制
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_3.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            copy_file(di_ptr);
                            break;
                        case 3: //删除
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_4.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            delete_file(mw,di_ptr);
                            refresh(mw);
                            break;
                        case 4://重命名
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_5.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            save_or_rename_flag=2;
                            set_area(mw->gd,area.x,area.y,area.width,area.height);
                            rename_file(mw);
                            break;
                        case 5://解压缩
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_6.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            uncompress_file(di_ptr);
                            refresh(mw);
                            break;
                        case 6://属性
                            draw_img(mw->gd, "/home/yanglei/egui/resource/icons/right_click/2_2_7.bmp", ALIGN_HORIZONTAL_TYPE_CENTER | ALIGN_VERTICAL_TYPE_CENTER );
                            file_or_directory=1;
                            show_file_attribute(mw);
                            break;
                        default:
                            break;
                   }
                }

            }
            set_area(mw->gd,area.x,area.y,area.width,area.height);
            my_widget_repaint(mw);
            my_widget_show(mw);
            return 0;
        }
    }


    return 0;
}

si_t
my_widget_default_mouse_double_click
(struct my_widget * mw,
 union message * msg)
{
    struct rectangle area;
    struct directory_item * di_ptr;
    si_t x, y, x1,y1, index;

    if(msg->mouse.code == INPUT_CODE_MOUSE_L_KEY)
    {
        /* 获得左上角的绝对坐标 */
        widget_absolute_coordinate(WIDGET_POINTER(mw), &x, &y);

        /* 获得绝对的工作区域 */
        /* 将会舍弃不在父控件内的部分*/
        application_widget_absolute_area(WIDGET_POINTER(mw), &area);

        /* 双击了那个目录项 */
        y1 = msg->mouse.cursor_position.y - y;
        x1 = msg->mouse.cursor_position.x - x;

        if(x1<110*((x1-30)/110)+30-1||x1>110*((x1-30)/110)+100-1||y1<90*((y1-15)/90)+15+2||y1>90*((y1-15)/90)+78+2)
           return 0;


        index = (y1-15)/90*num +(x1-30)/110;
        if(index >= (si_t)vector_size(&file_list))
            return 0;

        di_ptr = vector_at(&file_list, index + mw->start);
        cur_file_index=index+mw->start;

        open_file(mw);

    }

    return 0;
}

si_t
my_widget_default_callback
(void * self,
 void * msg)
{
    struct my_widget * mw = self;
    union message * m = msg;

    switch(m->base.type)
    {
        case MESSAGE_TYPE_WIDGET_REPAINT:
            my_widget_default_widget_repaint(mw, m);
            my_widget_default_widget_show(mw, m);
            break;

        case MESSAGE_TYPE_WIDGET_SHOW:
            my_widget_default_widget_show(mw, m);
            break;

        case MESSAGE_TYPE_MOUSE_PRESS:
            my_widget_default_mouse_press(mw, m);
            my_widget_default_widget_show(mw, m);
            break;

        case MESSAGE_TYPE_MOUSE_RELEASE:
            my_widget_default_mouse_release(mw, m);
            my_widget_default_widget_show(mw, m);
            break;

        case MESSAGE_TYPE_MOUSE_SINGLE_CLICK:
            my_widget_default_mouse_single_click(mw, m);
            break;

        case MESSAGE_TYPE_MOUSE_DOUBLE_CLICK:
            my_widget_default_mouse_double_click(mw, m);
            break;

        case MESSAGE_TYPE_WINDOW_CLOSE:
            vector_exit(&file_list);
            break;

        default:
            break;
    }

    return 0;
}

si_t
my_widget_repaint
(struct my_widget * mw)
{
    union message msg;

    msg.widget_repaint.area.x = mw->area.x;
    msg.widget_repaint.area.y = mw->area.y;
    msg.widget_repaint.area.width = mw->area.width;
    msg.widget_repaint.area.height = mw->area.height;

    my_widget_default_widget_repaint(mw, &msg);

    return 0;
}

si_t
my_widget_show
(struct my_widget * mw)
{
    union message msg;

    msg.widget_repaint.area.x = mw->area.x;
    msg.widget_repaint.area.y = mw->area.y;
    msg.widget_repaint.area.width = mw->area.width;
    msg.widget_repaint.area.height = mw->area.height;

    my_widget_default_widget_show(mw, &msg);

    return 0;
}

void *
my_widget_init
(si_t id)
{
    struct my_widget * addr;

    /* 分配存储空间 */
    addr = (struct my_widget *)malloc(sizeof(struct my_widget));

    if(addr == NULL)
    {
        printf("ERROR AT LINE %d OF FILE %s\n", __LINE__, __FILE__);
        perror("malloc");

        return NULL;
    }

    /* 申请图形设备 */
    addr->gd = graphics_device_init(0, 0, 0, 0, 0, 0 ,0 ,0 ,0);

    /* 申请失败 */
    if(addr->gd == 0)
    {
        /* 释放存储空间 */
        free(addr);

        return NULL;
    }

    /* struct button 的成员 */
    addr->parent = NULL;
    addr->lchild = NULL;
    addr->rchild = NULL;
    addr->name = "struct my_widget";
    addr->id = id;

    /* 默认工作区域 */
    addr->area.x = 0;
    addr->area.y = 0;
    addr->area.width = 0;
    addr->area.height = 0;

    /* 默认边界宽度 */
    addr->border_size = 0;

    /* 默认最大宽度 */
    addr->maximum_width = 0; /* should use macro */
    /* 默认最大高度 */
    addr->maximum_height = 0; /* should use macro */
    /* 默认最小宽度 */
    addr->minimum_width = 0;
    /* 默认最小高度 */
    addr->minimum_height = 0;

    /* 默认鼠标形状 */
    addr->cursor = CURSOR_SHAPE_X;

    /* 默认是否能处理键盘输入消息 */
    addr->input_enable = 0;

    /* 默认是否可以刷新 */
    addr->update_enable = 1;

    /* 默认是否可见 */
    addr->visible = 1;

    /* 默认是否拥有键盘焦点*/
    addr->keybd_focus = 0;

    /* 默认是否是窗口 */
    addr->is_window = 0;

    //addr->back_color = barely_blue;
    addr->frame_color = light_blue;
    addr->back_color = white;
    addr->font = FONT_MATRIX_08;

    /* 默认的回调函数 */
    addr->callback = my_widget_default_callback;

    icon_flag=0;

    return addr;
}

/*
    如果对象在对象树中就不应该调用，
    application 会自动为你释放相应的资源，
    如果不在对象树中，你可以调用这个函数。
*/
si_t
my_widget_exit
(struct my_widget * mw)
{
    graphics_device_exit(mw->gd);

    free(mw);

    return 0;
}
