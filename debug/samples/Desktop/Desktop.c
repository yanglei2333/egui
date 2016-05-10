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
 * it under the terms of the GNU General Publsh Lshense as published by
 * the Free Software Foundation, either version 2 of the Lshense, or
 * (at your option) any later version.
 *
 * EDUGUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Publsh Lshense for more details.
 *
 * You should have received a copy of the GNU General Publsh Lshense
 * along with EDUGUI; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 * All rights reserved.
**/

# include <stdio.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <string.h>
# include <unistd.h>
# include <time.h>
# include <dirent.h>


# include "geometry.h"
# include "comm.h"
# include "application.h"
# include "client_lib.h"
# include "widget.h"

# include "desktop.h"
# include "taskbar.h"
# include "right_click_menu.h"


int app_number=0;
struct image_view * Desktop_im = NULL;
struct window * Desktop_w = NULL;
struct desktop_info desktop;
struct vector sh_desktop_vector;
struct rectangle** area_pptr = NULL;
int** flag_pptr = NULL;

/* 初始化任务栏状态 */
si_t bar_num=0;
/* 初始化屏幕大小 */
si_t screen_w = 0;
si_t screen_h = 0;

si_t area_num_x=0;
si_t area_num_y=0;

struct image_view * bar_img[10];
/*任务栏图片*/


//找到但当前区域已有快捷方式，返回0；
//找到且当前区域无快捷方式，返回1；
//找不到返回-1；
int find_witch_area(struct point p, struct point* area_num){
	for(int i=0;i<area_num_x;i++)
		for(int j=0;j<area_num_y;j++)
			if(p.x>= area_pptr[i][j].x && p.x<= area_pptr[i][j].x+area_pptr[i][j].width && p.y>= area_pptr[i][j].y && p.y<= area_pptr[i][j].y+area_pptr[i][j].height){
				area_num->x=i, area_num->y=j;
				if(flag_pptr[i][j])
					return 0;
				return 1;
			}
	return -1;
}
//当前区域是否存在快捷方式
int is_area_being(struct point p){
	for(int i=0;i<area_num_x;i++)
		for(int j=0;j<area_num_y;j++)
			if(p.x>= area_pptr[i][j].x && p.x<= area_pptr[i][j].x+area_pptr[i][j].width && p.y>= area_pptr[i][j].y && p.y<= area_pptr[i][j].y+area_pptr[i][j].height){
				if(flag_pptr[i][j])
					return 1;
				return 0;
			}
	return 0;
}
//成功返回1；失败返回-1
int reset_shortcut_bound(struct shortcut* sh, struct point area_num){
	if(flag_pptr[area_num.x][area_num.y])
		return -1;	
	int x=area_pptr[area_num.x][area_num.y].x + 13;
	int y=area_pptr[area_num.x][area_num.y].y + 5;
	int width= 54;
	int height= 70;
	shortcut_set_bounds(sh, x, y, width, height);
	if(flag_pptr[sh->position_x][sh->position_y] ==1)
		flag_pptr[sh->position_x][sh->position_y] = 0;
	flag_pptr[area_num.x][area_num.y] = 1;
	sh->position_x = area_num.x;
	sh->position_y = area_num.y;	
	return 1;
}

si_t diff_timeval(struct timeval * t1, struct timeval * t2, struct timeval * result)
{
    if(t1->tv_usec < t2->tv_usec)
    {
        result->tv_usec = t1->tv_usec + 1000000 - t2->tv_usec;
        result->tv_sec = t1->tv_sec - 1 - t2->tv_sec;
    }
    else
    {
        result->tv_usec = t1->tv_usec - t2->tv_usec;
        result->tv_sec = t1->tv_sec - t2->tv_sec;
    }

    return 0;
}


si_t shortcut_act(struct shortcut* sh_ptr){
	shortcut_set_img_path(sh_ptr, sh_ptr->img_normal_path);
	sh_ptr->flag=0;
	pid_t id;
	id = fork();
	char* full_path=(char*)malloc(strlen(sh_ptr->app_path)+strlen(sh_ptr->app_name)+1);
	char* act=(char*)malloc(2+strlen(sh_ptr->app_name)+1);
	strcpy(full_path,sh_ptr->app_path);
	strcat(full_path,sh_ptr->app_name);
	act[0]='.',act[1]='/';
	strcat(act,sh_ptr->app_name);
	if(id == 0){
		if(strstr(sh_ptr->app_name,"image_view")!=NULL && sh_ptr->is_real==1)
			execl(full_path,act,sh_ptr->link_file_path,NULL);
		else if(strstr(sh_ptr->app_name,"editerbasic")!=NULL && sh_ptr->is_real==1)
			execl(full_path,act,sh_ptr->link_file_path,NULL);
		else if(strstr(sh_ptr->app_name,"file_browser")!=NULL && sh_ptr->is_real==1)
			execl(full_path,act,sh_ptr->link_file_path,NULL);
		else if(strstr(sh_ptr->app_name,"NULL")!=NULL && sh_ptr->is_real==1)
			;
		else
			execl(full_path,act,NULL);
	}
}


int change_file(char* file,char* src,char* des){
	char file2[]="/home/yanglei/egui/debug/samples/Desktop/shortcut/temp11";
    FILE *in=fopen(file,"r");
    FILE *out=fopen(file2,"w");  /*out是充当临时文件作用*/
    char buffer[1024];
	memset(buffer , 0, sizeof(char)*1024);
    if(!in)
    {
        printf("cann't open file\n");
        exit(1);
    }
    if(!out)
    {
        printf("cann't create file\n");
        exit(1);
    }
        /*开始复制*/
    while(fgets(buffer,1024,in))
    {
       //if(ch=='a') ch='p';
	   char* ptr = strstr(buffer,src);
	   if(ptr!=NULL){
			strcpy(buffer,src);
			strcat(buffer,des);
			strcat(buffer,"\n");
	   }
       fputs(buffer,out);
	   memset(buffer , 0, sizeof(char)*1024);
    }
    fclose(in);  
	fclose(out);  
    unlink(file); /*删除test.txt*/
	chmod(file2, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IWGRP|S_IXGRP|S_IROTH|S_IWOTH|S_IXOTH);
    rename(file2,file); /*改名*/
	return 0;
}

si_t shortcut_callback(void * sh, void * msg)
{
	struct shortcut * sh_ptr=sh;
	union message * m = msg;
	switch(m->base.type)
	{
		case MESSAGE_TYPE_MOUSE_PRESS:
			sh_ptr->last_do_time=m->base.time;			  
			break;

		case MESSAGE_TYPE_MOUSE_RELEASE:
		{
			sh_ptr->last_do_time.tv_sec=0;	
			sh_ptr->last_do_time.tv_usec=0;
			 
		}
			break;
		case MESSAGE_TYPE_MOUSE_SINGLE_CLICK:
		{
			//左键点击
			if(m->mouse.code == INPUT_CODE_MOUSE_L_KEY){
				if(sh_ptr->flag==0){
					for(int i=0;i<app_number;i++){
						struct shortcut* sh = vector_at(&sh_desktop_vector, i);
						shortcut_set_img_path(sh, sh->img_normal_path);
						sh->flag=0;
					}
					shortcut_set_img_path(sh_ptr, sh_ptr->img_select_path);
					sh_ptr->flag=1;
				}
				else{
					shortcut_set_img_path(sh_ptr, sh_ptr->img_normal_path);
					sh_ptr->flag=0;
				}

				sh_ptr->last_do_time.tv_sec=0;	
				sh_ptr->last_do_time.tv_usec=0;
				// 重绘所有桌面快捷方式
				for(int i=0;i<app_number;i++){
					struct shortcut* sh = vector_at(&sh_desktop_vector, i);
			  		shortcut_repaint(sh);
			  		shortcut_show(sh);
				}
			}
			//右键单击
			else if(m->mouse.code == INPUT_CODE_MOUSE_R_KEY){

				for(int i=0;i<app_number;i++){
					struct shortcut* sh = vector_at(&sh_desktop_vector, i);
					shortcut_set_img_path(sh, sh->img_normal_path);
					sh->flag=0;
				}
				shortcut_set_img_path(sh_ptr, sh_ptr->img_select_path);
				sh_ptr->flag=1;
				sh_ptr->last_do_time.tv_sec=0;	
				sh_ptr->last_do_time.tv_usec=0;
				// 重绘所有桌面快捷方式
				for(int i=0;i<app_number;i++){
					struct shortcut* sh = vector_at(&sh_desktop_vector, i);
			  		shortcut_repaint(sh);
			  		shortcut_show(sh);
				}

				//right_click_menu_cancel();
				right_click_menu_handle(m);
			}
		}
			break;
	
		case MESSAGE_TYPE_MOUSE_DOUBLE_CLICK:
			shortcut_act(sh_ptr);
			break;
		default:
			shortcut_default_callback(sh,msg);
			break;
	}
	return 0;
}



/* 桌面的处理函数 */
void desktop_handler(addr_t arg, union message* msg)
{
	struct desktop_info* desktop_info_ptr = (struct desktop_info*)arg;
	si_t window_descripter = message_get_window_descriptor(msg);	
	/* 初始化 */
	static flag = 0;
	if(flag == 0){
		desktop_info_init(desktop_info_ptr);
		flag = 1;
	}
		
	switch(message_get_type(msg))
	{
		//当敲击回车时，若有快捷方式被选中，则启动程序
		case MESSAGE_TYPE_KEYBD_PRESS:
			if(message_get_keybd_code(msg)==INPUT_CODE_KEYBD_ENTER){				
				struct shortcut* sh_desktop_ptr = NULL;
				int find_num=0;  //查找正在作用的桌面图标
				for(find_num=0; find_num < app_number; find_num++){
					sh_desktop_ptr = vector_at(&sh_desktop_vector, find_num);
					if(sh_desktop_ptr->flag==1)
						break;
				}
				sh_desktop_ptr->flag=0;
				sh_desktop_ptr->last_do_time.tv_sec=0;	
				sh_desktop_ptr->last_do_time.tv_usec=0;
				shortcut_set_img_path(sh_desktop_ptr, sh_desktop_ptr->img_normal_path);
				shortcut_repaint(sh_desktop_ptr);
				shortcut_show(sh_desktop_ptr);
				// 启动要打开的程序 
				if(find_num < app_number){	
					shortcut_act(sh_desktop_ptr);
				}
			}
		break;


		/* 处理桌面图标拖动的情况 */
		case MESSAGE_TYPE_MOUSE_RELEASE:
		 {	
				struct shortcut* sh_desktop_ptr = NULL;
				int find_num=0;  //查找正在作用的桌面图标
				for(find_num=0; find_num < app_number; find_num++){
					sh_desktop_ptr = vector_at(&sh_desktop_vector, find_num);
					if(sh_desktop_ptr->last_do_time.tv_sec != 0 || sh_desktop_ptr->last_do_time.tv_usec != 0)
						break;
				}
				/* 找到正在起作用的桌面图标，执行拖动处理 */
				if(find_num < app_number) {			
					struct timeval difference = {0};
					diff_timeval(&(msg->base.time), &(sh_desktop_ptr->last_do_time), &difference);
					if(difference.tv_sec == 0 && difference.tv_usec < 300000){	//按下时间超过0.3秒认为拖动
						sh_desktop_ptr->last_do_time.tv_sec=0;	
						sh_desktop_ptr->last_do_time.tv_usec=0;
					}
					else{
						//设置新位置
			  			struct point new_point = msg->base.cursor_position;
						struct point area_num;
						if( find_witch_area(new_point, &area_num) == 1){
							
							reset_shortcut_bound(sh_desktop_ptr, area_num);
							char new_position[3];
							int xx=(int)area_num.x,yy=(int)area_num.y;
							new_position[0]=(char)(xx+48),new_position[1]=(char)(yy+48),new_position[2]=0;
							if(sh_desktop_ptr->is_real!=1){
								change_file(sh_desktop_ptr->link_file_path,"POSITION:",new_position);
							}
						}
			  			/* 桌面刷新重绘 */
			  			image_view_reshow(Desktop_im);
  			  			/* 所有图标重绘 */				
			  			for(int i=0;i<app_number;i++){
							struct shortcut* sh = vector_at(&sh_desktop_vector, i);
			  				shortcut_repaint(sh);
			  				shortcut_show(sh);
			  			}
						sh_desktop_ptr->last_do_time.tv_sec=0;	
						sh_desktop_ptr->last_do_time.tv_usec=0;
					}
				}
				/*右键菜单取消*/
				right_click_menu_cancel();		  
		 }
		break;
		/* 任务栏的激活处理 */
		case MESSAGE_TYPE_MOUSE_SINGLE_CLICK:
		{
			//左键单击
			if(msg->mouse.code == INPUT_CODE_MOUSE_L_KEY){
				struct point* point_ptr=message_get_cursor_position(msg);
				si_t x=point_ptr->x, y=point_ptr->y;
				if(x>80 && x<(bar_num +1)*80 && y<screen_h && y>screen_h-30){
					int i=(x-80)/80;
					struct window_info * win_info_ptr = (struct window_info *) vector_at(&(desktop_info_ptr->window_info_vector),i);
					activate_window(win_info_ptr->window_descripter);
				} 


				struct point new_point = msg->base.cursor_position;
				struct point area_num;
				//当前区域无快捷方式
				if( find_witch_area(new_point, &area_num) == 1){
					struct shortcut* sh_desktop_ptr = NULL;
					int find_num=0;  //查找正在作用的桌面图标
					for(find_num=0; find_num < app_number; find_num++){
						sh_desktop_ptr = vector_at(&sh_desktop_vector, find_num);
						if(sh_desktop_ptr->flag==1)
							break;
					}
					if(find_num < app_number){
						// 重绘所有桌面快捷方式
						for(int i=0;i<app_number;i++){
							struct shortcut* sh = vector_at(&sh_desktop_vector, i);
							sh->flag=0;
							shortcut_set_img_path(sh, sh->img_normal_path);
					  		shortcut_repaint(sh);
					  		shortcut_show(sh);
						}
					}
				}
			}
			//中建单击
			else if(msg->mouse.code == INPUT_CODE_MOUSE_M_KEY){
				/*右键菜单取消*/
				right_click_menu_cancel();
			}
			//右键单击
			else if(msg->mouse.code == INPUT_CODE_MOUSE_R_KEY){
				struct point p = msg->base.cursor_position;
				if(is_area_being(p)==0)
					right_click_menu_handle(msg);
			}

		}
		break;

		/* 有程序启动时记录并添加到任务栏 */
		case MESSAGE_TYPE_WINDOW_REGISTER:
		{
			bar_num++;
			
			struct window_info * win_info_ptr = (struct window_info *)malloc(sizeof(struct window_info));
			char *title= (char*)malloc(20);
			
			strcpy(title,(msg->window_register).title);
			window_info_init(win_info_ptr, title, window_descripter);
			vector_push_back(desktop_info_ptr, win_info_ptr, sizeof(struct window_info));	
			desktop_bar_repaint(global_application.main_window);			
    	}
		break;

		/* 程序推出时从记录中删除并在任务栏关闭 */	
		case MESSAGE_TYPE_APP_WINDOW_CANCEL:
		{
			bar_num--;
			int i = desktop_info_find_window_num(desktop_info_ptr, window_descripter);
			int j=i-1;
			for(;j<bar_num-2;j++)
				bar_img[j]=bar_img[j+1];
			vector_erase(&(desktop_info_ptr->window_info_vector),i);
			desktop_bar_repaint(global_application.main_window);
		}
		break;


	}
	
}




/*
    桌面主程序入口
*/
int main()
{
    si_t video_access_mode = VIDEO_ACCESS_MODE_BUFFER;
	si_t app_type = APPLICATION_TYPE_DESKTOP;
	
	int i=0;

    /* 初始化桌面程序 */
    application_init(video_access_mode, app_type, "desktop");
    application_desktop_set(&desktop, desktop_handler);
    /* 申请桌面主窗口 */
    Desktop_w = desktop_init("EGUI");
    /* 申请失败 */
    if(Desktop_w == NULL)
    {
        application_exit(); 
        return -1;
    }
	/* 根据用户屏幕配置桌面大小 */
    screen_w = get_screen_size_w();
    screen_h = get_screen_size_h();
	desktop_set_bounds(Desktop_w, 0, 0, screen_w, screen_h);
	desktop_set_color(Desktop_w, NULL, &xue_blue);

	area_num_x=screen_w/80;
	area_pptr=(struct rectangle**) malloc(sizeof(struct rectangle*)*area_num_x);
	flag_pptr=(int**) malloc(sizeof(int*)*area_num_x);
	area_num_y=(screen_h - 30)/80;
	for(i=0;i<area_num_x;i++){
		area_pptr[i]=(struct rectangle*) malloc(sizeof(struct rectangle)*area_num_y);
		flag_pptr[i]=(int*) malloc(sizeof(int)*area_num_y);
	}
	for(int j=0;j<area_num_x;j++)
		for(int k=0;k<area_num_y;k++){
			area_pptr[j][k].x=j*80;
			area_pptr[j][k].y=k*80;
			area_pptr[j][k].width=80;
			area_pptr[j][k].height=80;
			flag_pptr[j][k]=0;
	}



	/* 画背景图片 */
    Desktop_im = image_view_init("/home/yanglei/egui/resource/icons/desktop/desktop11.bmp");
    if(Desktop_im == NULL)
    {
        application_exit();
        return -1;
    }
	image_view_set_bounds(Desktop_im, 0, 0, screen_w ,screen_h-30);





	/* 配置各个桌面按钮 */
	struct shortcut* sh_desktop_ptr = NULL;
	//load_shortcut();
	{
		vector_init(&sh_desktop_vector);
		DIR * dir_ptr;
		char path[]="/home/yanglei/egui/debug/samples/Desktop/shortcut/";
		char full_path[1024]={0};
		struct dirent * dirent_ptr;
		char content[1024];
		struct stat file_info;
		FILE *fp;
		dir_ptr = opendir(path);
		int num=1;//默认的位置
	
		if(dir_ptr == NULL)
		    return -1;

	//////////////////////////////////////////////////////
		while((dirent_ptr = readdir(dir_ptr)) != 0)
		{
		    
		    //if(strlen(dirent_ptr->d_name) >2 && (strstr(dirent_ptr->d_name,"shortcut")!=NULL)){
			if(strlen(dirent_ptr->d_name) >0 && strcmp(dirent_ptr->d_name,".")!=0 && strcmp(dirent_ptr->d_name,"..")!=0){
		        strcpy(full_path,path);
		        strcat(full_path,dirent_ptr->d_name);
				lstat(full_path, &file_info);
		        if((fp=fopen(full_path,"r"))==NULL)
		        {
		            //return -1;
		        }
				/* 申请快捷方式 */
				struct shortcut* sh_desktop_ptr = shortcut_init(2);
				/* 申请失败 */
				if(sh_desktop_ptr == NULL)
				{
		    		application_exit();
		    		//return -1;
				}
			

				
				shortcut_set_is_text_visiable(sh_desktop_ptr ,1);
				vector_push_back(&sh_desktop_vector, sh_desktop_ptr, sizeof(struct shortcut));
				struct shortcut* sh= vector_at(&sh_desktop_vector, app_number);
				//将快捷方式（widgt）与快捷方式文件/待链接的文件路径存到widgt里,以待之后对文件进行修改
				strcpy(sh->link_file_path,full_path);
				int count=0;
				//当前文件是快捷方式文件，直接读取其中信息
				if(strstr(dirent_ptr->d_name,".shortcut")!=NULL){

		        	while(fgets(content,1024,fp)){
						content[strlen(content)-1]=0;
						if(count==0)	//(NAME:,实际内容在第5位)
							shortcut_set_text(sh,content+5);
						else if(count==1){
							shortcut_set_img_path(sh,content);
							shortcut_set_img_normal_path(sh,content);
						}
						else if(count==2)
							shortcut_set_img_select_path(sh,content);
						else if(count==3)
							strcpy(sh->app_name, content);
						else if(count==4)
							strcpy(sh->app_path, content);
						//如果指定位置
						else if(count==5){
							//若没有指定位置，则默认初始化位置(POSITION:XX,实际位置在第9,10位)
							if(content[9]=='0' && content[10]=='0'){
							//直到将该快捷方式分配好位置为止
								struct point p;
								while(1){
									p.x=num/area_num_y;
									p.y=num%area_num_y;
									num++;
									if(reset_shortcut_bound(sh,p)==1)
										break;
								}
								char new_position[3];
								new_position[0]=(char)(p.x+48),new_position[1]=(char)(p.y+48),new_position[2]=0;
								change_file(sh->link_file_path,"POSITION:",new_position);
							}
							//位置不是(0,0)
							else{
								struct point p;
								p.x=content[9]-48,p.y=content[10]-48;
								while(1){
									if(reset_shortcut_bound(sh,p)==1){
										char new_position[3];
										new_position[0]=(char)(p.x+48),new_position[1]=(char)(p.y+48),new_position[2]=0;
										change_file(sh->link_file_path,"POSITION:",new_position);
										break;
									}
									p.x=p.x+(p.y+1)/area_num_y,p.y=(p.y+1)%area_num_y;
								}
							}
						}
						//与之关联的是快捷方式文件
						sh->is_real=0;
						memset(content , 0, sizeof(char)*1024);
						count++;
		        	}
				}
				//当前文件是真实的图片
				else if(strstr(dirent_ptr->d_name,".bmp")!=NULL){
					shortcut_set_text(sh,dirent_ptr->d_name);
					shortcut_set_img_path(sh,"/home/yanglei/egui/resource/icons/file_icon/bmp_1.bmp");
					shortcut_set_img_normal_path(sh,"/home/yanglei/egui/resource/icons/file_icon/bmp_1.bmp");
					shortcut_set_img_select_path(sh,"/home/yanglei/egui/resource/icons/file_icon/bmp_2.bmp");
					strcpy(sh->app_name, "image_view");
				    strcpy(sh->app_path, "/home/yanglei/egui/_bulid/debug/samples/");				
					//直到将该快捷方式分配好位置为止
					while(1){
						struct point p;
						p.x=num/area_num_y;
						p.y=num%area_num_y;
						num++;
						if(reset_shortcut_bound(sh,p)==1)
							break;
					}		
					//与之关联的是真实文件			
					sh->is_real=1;
				}
				//当前文件是其他文本文件
				else if(S_ISREG(file_info.st_mode)){
					shortcut_set_text(sh,dirent_ptr->d_name);
					shortcut_set_img_path(sh,"/home/yanglei/egui/resource/icons/file_icon2/txt_1.bmp");
					shortcut_set_img_normal_path(sh,"/home/yanglei/egui/resource/icons/file_icon2/txt_1.bmp");
					shortcut_set_img_select_path(sh,"/home/yanglei/egui/resource/icons/file_icon2/txt_2.bmp");
					strcpy(sh->app_name, "editerbasic");
				    strcpy(sh->app_path, "/home/yanglei/egui/_bulid/debug/samples/");				
					//直到将该快捷方式分配好位置为止
					while(1){
						struct point p;
						p.x=num/area_num_y;
						p.y=num%area_num_y;
						num++;
						if(reset_shortcut_bound(sh,p)==1)
							break;
					}					
					//与之关联的是真实文件			
					sh->is_real=1;
				}
				//当前是文件夹（目录）
				else if(S_ISDIR(file_info.st_mode)){
					shortcut_set_text(sh,dirent_ptr->d_name);
					shortcut_set_img_path(sh,"/home/yanglei/egui/resource/icons/file_icon/dir1_1.bmp");
					shortcut_set_img_normal_path(sh,"/home/yanglei/egui/resource/icons/file_icon/dir1_1.bmp");
					shortcut_set_img_select_path(sh,"/home/yanglei/egui/resource/icons/file_icon/dir1_2.bmp");
					strcpy(sh->app_name, "file_browser");
				    strcpy(sh->app_path, "/home/yanglei/egui/_bulid/debug/samples/file_browser/");				
					//直到将该快捷方式分配好位置为止
					while(1){
						struct point p;
						p.x=num/area_num_y;
						p.y=num%area_num_y;
						num++;
						if(reset_shortcut_bound(sh,p)==1)
							break;
					}
					//与之关联的是真实文件			
					sh->is_real=1;
				}
				//当前文件是其他未知文件
				else{
					shortcut_set_text(sh,dirent_ptr->d_name);
					shortcut_set_img_path(sh,"/home/yanglei/egui/resource/icons/file_icon2/ex_1.bmp");
					shortcut_set_img_normal_path(sh,"/home/yanglei/egui/resource/icons/file_icon2/ex_1.bmp");
					shortcut_set_img_select_path(sh,"/home/yanglei/egui/resource/icons/file_icon2/ex_2.bmp");
					strcpy(sh->app_name, "NULL");
				    strcpy(sh->app_path, "NULL");				
					//直到将该快捷方式分配好位置为止
					while(1){
						struct point p;
						p.x=num/area_num_y;
						p.y=num%area_num_y;
						num++;
						if(reset_shortcut_bound(sh,p)==1)
							break;
					}					
					//与之关联的是真实文件			
					sh->is_real=1;
				}

				sh->callback = shortcut_callback;			
				app_number++;
				fclose(fp);
		    }
		}
		closedir(dir_ptr);
	}






	

    /* 背景图片添加到窗口 */
    object_attach_child(OBJECT_POINTER(Desktop_w), OBJECT_POINTER(Desktop_im));   
    /* 将shortcut添加到窗口 */
    for(i=0;i<app_number;i++){
		sh_desktop_ptr = vector_at(&sh_desktop_vector, i);
    	object_attach_child(OBJECT_POINTER(Desktop_im), OBJECT_POINTER(sh_desktop_ptr));
	}
   

     /* 右键菜单初始化 */
     right_click_menu_init();

    /* 添加顶层窗口 */
    application_add_window(NULL, Desktop_w);
    /* 设置主窗口 */
    application_set_main_window(Desktop_w);
    /* 运行 */
    application_exec();

    return 0;
}
