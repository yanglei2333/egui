/**
 * @file desktop.c
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

# include <stdio.h>
# include <stdlib.h>

# include <base_type.h>
# include <config_parser.h>
# include <log.h>
# include <comm.h>
# include <client_lib.h>
# include "compiler.h"
# include "widget.h"

# include "desktop.h"
//# include "paint_window_decoration.h"

/* Invalid path. No specification for window.cfg */
# define TMP_ARRAY_SIZE 256
/* window样式全局对象 */
/* XXX: Not fully implemented.
 * Several attributes are not used.
 */




struct window_style desktop_default_style = 	//changed
{
	{
		/* 初始化，默认未访问 */
		0,  /* .flag */

		/* 默认工作区域 */
		0, 0, 0, 0,

		/* 默认边界宽度 */
		0,  /* .border_size */		//changed

		/* 默认宽度&高度 */
		0,  /* .maximum_width */
		0,  /* .minimum_width */
		0,  /* .maximum_height */
		0,  /* .minimum_height */

		/* 默认鼠标形状 */
		CURSOR_SHAPE_X, /* .cursor */

		/* 默认背景色 */
		0, 0, 255, 0,	/* .back_color */

		/* 默认前景色 */
		0, 0, 0, 0,  /* fore_color */
	},

	/* 默认是否可以最大化/最小化/全屏 */
	0,  /* .maximize_enable */		//changed
	0,  /* .minimize_enable */		//changed
	1,  /* .is_full_screen */		//changed

	/* 边框/标题栏尺寸 */
	0,  /* .frame_size */		//changed
	30, /* .title_bar_size */

	/** 默认窗口边框颜色 **/
	{0x46, 0xa5, 0xe5, 50},	/* .frame_color */

	/** 菜单栏背景色 **/
	{0x18, 0x29, 0x45, 50},	/* .menu_bar_backcolor */

	/** 菜单栏边框色 **/
	{0xcb, 0xf3, 0xfb, 50},	/* .menu_bar_frame_color */

	/** 标题栏字体颜色 **/
	{0xcc, 0xff, 0x99, 0},	/* .title_bar_font_color */

	/** 标题栏字体大小 **/
	FONT_MATRIX_12	/* .title_font */
};

/**
 * @brief 用window全局样式对象初始化window对象
 *
 * @param style window样式对象指针
 * @param w window指针
 *
 * @return 成功返回0，否则返回-1
 **/
static si_t desktop_init_with_default_style(struct window * w)
{
	char *config_path = get_config_path("window.cfg");
	char tmp_str[TMP_ARRAY_SIZE] = {'\0'};
	si_t tmp_int;
	struct widget_style_entry extra[] = {
		{.key="maximize_enable", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.maximize_enable)},
		{.key="minimize_enable", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.minimize_enable)},
		{.key="is_full_screen", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.is_full_screen)},

		{.key="frame_size", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.frame_size)},
		{.key="title_bar_size", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.title_bar_size)},

		{.key="frame_color_r", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.frame_color.r)},
		{.key="frame_color_g", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.frame_color.g)},
		{.key="frame_color_b", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.frame_color.b)},
		{.key="frame_color_a", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.frame_color.a)},

		{.key="menu_bar_backcolor_r", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_backcolor.r)},
		{.key="menu_bar_backcolor_g", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_backcolor.g)},
		{.key="menu_bar_backcolor_b", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_backcolor.b)},
		{.key="menu_bar_backcolor_a", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_backcolor.a)},

		{.key="menu_bar_frame_color_r", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_frame_color.r)},
		{.key="menu_bar_frame_color_g", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_frame_color.g)},
		{.key="menu_bar_frame_color_b", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_frame_color.b)},
		{.key="menu_bar_frame_color_a", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.menu_bar_frame_color.a)},

		{.key="title_bar_font_color_r", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.title_bar_font_color.r)},
		{.key="title_bar_font_color_g", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.title_bar_font_color.g)},
		{.key="title_bar_font_color_b", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.title_bar_font_color.b)},
		{.key="title_bar_font_color_a", .type=WIDGET_STYLE_TYPE_INT,
			.val=&(desktop_default_style.title_bar_font_color.a)},

		{.key="title_font", .type=WIDGET_STYLE_TYPE_STR, .val=tmp_str},
	};

	si_t res = widget_init_with_default_style(config_path,
			WIDGET_POINTER(w), &desktop_default_style.common,
			extra, sizeof(extra)/sizeof(extra[0]));
	free(config_path);

	if((tmp_int = get_font_enum_from_str(tmp_str)) >= 0) {
		desktop_default_style.title_font = tmp_int;
	}

	/* 用window默认样式初始化window各样式属性 */		//changed
	w->minimize_enable = 0;//window_default_style.minimize_enable;
	w->maximize_enable = 0;//window_default_style.maximize_enable;
	w->is_full_screen = 1;//window_default_style.is_full_screen;

	return res;
}

/* 设置桌面相关的颜色 */
static si_t
	set_desktop_color
(struct window* w,
 struct color* c)
{
	int r = c->r;
	int g = c->g;
	int b = c->b;
	int a = c->a;

	set_color(w->gd, r, g, b, a);
	return 0;
}


static si_t desktop_default_widget_show(struct window * w, union message * msg)//changed
{
	NOT_USED(msg);
	/* 设置区域 */
	set_area
		(w->gd,
		 w->area.x ,		//changed
		 w->area.y ,//changed
		 w->area.width ,//changed
		 w->area.height );//changed

	update(w->gd);
	return 0;
}


static si_t desktop_default_widget_repaint(struct window * w, union message * msg)
{
	set_desktop_color(w, &ground_purple);
	fill_rectangle(w->gd, 0, screen_h-30, w->area.width, 30);

	/* 字体 */
	set_desktop_color(w, &font_white);
	set_font(w->gd, FONT_MATRIX_12);
	/* 标题 */
	show_text(w->gd, 15, 3+screen_h-30, w->title, strlen(w->title));

	for(int i=0;i<bar_num;i++){
		set_desktop_color(w,&bar_blue);
		fill_rectangle(w->gd,(i+1)*80,1+screen_h-30,79,28);

		struct window_info * win_info_ptr = (struct window_info *) vector_at(&(desktop.window_info_vector),i);

		///////////////////////////////////////////////////////////////////////////////////////////////////		
		char* bar_path=malloc(100*sizeof(char));
		strcpy(bar_path,"/home/yanglei/egui/resource/min/");
		strcat(bar_path,win_info_ptr->title);
		strcat(bar_path,".bmp");
		struct image_view *app_image; 
		app_image = image_view_init(bar_path);
		image_view_set_bounds(app_image,(i+1)*80,1+screen_h-28,24,24);
		object_attach_child(OBJECT_POINTER(w),OBJECT_POINTER(app_image));
		////////////////////////////////////////////////////////////////////////////////////////////////////
		set_desktop_color(w, &font_white);
//		int len=strlen(win_info_ptr->title);
//		if(len>=10){
//			int j=0;
//			int n=0;
//			for(j=0;j<len;j++)
//				if((win_info_ptr->title)[j]=='_')
//					n=j;
//
//			char* t1=(char*)malloc(sizeof(char)*(n+1));
//			char* t2=(char*)malloc(sizeof(char)* (len-n-1+1));
//
//			for(j=0;j<n;j++)
//				t1[j]=(win_info_ptr->title)[j];
//			t1[j]=0;
//			for(j=0;j<len-n-1;j++)
//				t2[j]=(win_info_ptr->title)[j+n+1];
//			t2[j]=0;
//
//			set_font(w->gd, FONT_MATRIX_10);
//			show_text(w->gd, (i+1)*80 + ((10-n)/2-1)*10+12, screen_h-30, t1, strlen(t1));
//			set_font(w->gd, FONT_MATRIX_10);
//			show_text(w->gd, (i+1)*80 + ((10-len+n+1)/2-1)*10+12, 10+screen_h-30, t2, strlen(t2));
//		}
//		else{
//			set_font(w->gd, FONT_MATRIX_10);
//			show_text(w->gd, (i+1)*80 + ((10-len)/2-1)*10+12, 3+screen_h-30, win_info_ptr->title, strlen(win_info_ptr->title));
//		}
//
//
	}
	return 0;
}

/**
 * @brief 重绘桌面任务栏
 **/
extern void desktop_bar_repaint(struct window* w){
	set_area(w->gd,0,screen_h-30,w->area.width, 30);
	set_desktop_color(w, &ground_purple);
	fill_rectangle(w->gd, 0, screen_h-30, w->area.width, 30);

	/* 字体 */
	set_desktop_color(w, &font_white);
	set_font(w->gd, FONT_MATRIX_12);
	/* 标题 */
	show_text(w->gd, 15, 3+screen_h-30, w->title, strlen(w->title));

//	for(int i=0;i<bar_num;i++){
//		set_desktop_color(w,&bar_blue);
//		fill_rectangle(w->gd,(i+1)*80,1+screen_h-30,79,28);
//
//		struct window_info * win_info_ptr = (struct window_info *) vector_at(&(desktop.window_info_vector),i);
//		set_desktop_color(w, &font_white);
//		int len=strlen(win_info_ptr->title);
//		if(len>=10){
//			int j=0;
//			int n=0;
//			for(j=0;j<len;j++)
//				if((win_info_ptr->title)[j]=='_')
//					n=j;
//
//			char* t1=(char*)malloc(sizeof(char)*(n+1));
//			char* t2=(char*)malloc(sizeof(char)* (len-n-1+1));
//			for(j=0;j<n;j++)
//				t1[j]=(win_info_ptr->title)[j];
//			t1[j]=0;
//			for(j=0;j<len-n-1;j++)
//				t2[j]=(win_info_ptr->title)[j+n+1];
//			t2[j]=0;
//
//			set_font(w->gd, FONT_MATRIX_10);
//			show_text(w->gd, (i+1)*80 + ((10-n)/2-1)*10+12, screen_h-30, t1, strlen(t1));
//			set_font(w->gd, FONT_MATRIX_10);
//			show_text(w->gd, (i+1)*80 + ((10-len+n+1)/2-1)*10+12, 10+screen_h-30, t2, strlen(t2));
//		}
//		else{
//			set_font(w->gd, FONT_MATRIX_10);
//			show_text(w->gd, (i+1)*80 + ((10-len)/2-1)*10+12, 3+screen_h-30, win_info_ptr->title, strlen(win_info_ptr->title));
//		}
//
//
//	}
	desktop_default_widget_show(w, NULL);
}


static si_t desktop_default_window_activate(struct window * w, union message * msg)//unchanged
{

	if(w->visible && !w->is_activated)
	{
		w->is_activated = 1;
		desktop_default_widget_show(w, msg);
	}

	w->is_activated = 1;
	return 0;
}

static si_t desktop_default_window_deactivate(struct window * w, union message * msg)//unchanged
{
	NOT_USED(msg);


	if(w->visible && w->is_activated)
	{
		si_t x, y, width, height;
		/* 设置图形设备的工作区域 */
		x = w->area.x;
		y = w->area.y ;
		width = w->area.width;
		height = w->area.height;
		set_area(w->gd, x, y, width, height);
		update(w->gd);
	}
	w->is_activated = 0;

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

si_t desktop_default_callback(addr_t w, addr_t m)
{
	struct window * self = w;
	union message * msg = m;

	widget_default_callback(self, msg);

	switch(msg->base.type)
	{
		case MESSAGE_TYPE_WIDGET_SHOW:
			/*
			   printf("MESSAGE_TYPE_WIDGET_SHOW\n");
			 */
			desktop_default_widget_show(self, msg);
			break;


		case MESSAGE_TYPE_WIDGET_REPAINT:
			/*
			   printf("MESSAGE_TYPE_WIDGET_REPAINT\n");
			 */
			desktop_default_widget_repaint(self, msg); 
			desktop_default_widget_show(self, msg);
			break;

		case MESSAGE_TYPE_WIDGET_MOVE:
			/*
			   printf("MESSAGE_TYPE_WIDGET_MOVE\n");
			 */

			break;

		case MESSAGE_TYPE_WIDGET_RESIZE:
			/*
			   printf("MESSAGE_TYPE_WIDGET_RESIZE\n");
			 */         
			break;

		case MESSAGE_TYPE_WINDOW_ACTIVATE:
			/*
			   printf("MESSAGE_TYPE_WINDOW_ACTIVATE\n");
			 */
			desktop_default_window_activate(self, msg);
			break;

		case MESSAGE_TYPE_WINDOW_DEACTIVATE:
			/*
			   printf("MESSAGE_TYPE_WINDOW_DEACTIVATE\n");
			 */
			desktop_default_window_deactivate(self, msg);
			break;

		case MESSAGE_TYPE_WINDOW_MINIMIZE:
			/*
			   printf("MESSAGE_TYPE_WINDOW_MINIMIZE\n");
			 */           
			break;

		case MESSAGE_TYPE_WINDOW_MAXIMIZE:
			/*
			   printf("MESSAGE_TYPE_WINDOW_MAXIMIZE\n");
			 */            
			break;

		case MESSAGE_TYPE_WINDOW_RESTORE:
			/*
			   printf("MESSAGE_TYPE_WINDOW_RESTORE\n");
			 */            
			break;

		default:
			break;
	}

	return 0;
}

void desktop_repaint(struct window* w)
{
	widget_repaint(WIDGET_POINTER(w));
}

void desktop_show(struct window* w)
{
	widget_show(WIDGET_POINTER(w));
}

/**
 * @brief 重新显示窗口
 **/
void desktop_reshow(struct window* w){

	desktop_default_widget_repaint(w, NULL); 
	desktop_default_widget_show(w, NULL);
}

struct window* desktop_init(char* title)
{
	struct window * addr;

	addr = (struct window *)malloc(sizeof(struct window));

	if(addr == NULL)
	{
		EGUI_PRINT_SYS_ERROR("fail to malloc");
		return addr;
	}

	if(!(addr=widget_init_common(WIDGET_POINTER(addr), 0))) {
		return NULL;
	}

	addr->name = "struct window";

	/* 默认是否是窗口 */
	addr->is_window = 1;

	/* default not is_activated */
	addr->is_activated = 0;

	/* default status */
	addr->status = WINDOW_STATUS_NORMAL;

	/* 默认是否是模态 */
	addr->modal = 0;
	/* 默认标题 */
	addr->title = title;
	/* icon path */
	addr->icon_path = NULL;

	/* 没注册前的窗口描述符 */
	addr->descriptor = 0;

	/* 用全局样式对象初始化window样式 */
	desktop_init_with_default_style(addr);//changed

	/* 默认的回调函数 */
	addr->callback = desktop_default_callback;

	return addr;
}

si_t desktop_exit(struct window * w)
{
	return widget_exit(WIDGET_POINTER(w));
}

void desktop_set_bounds(struct window  *w, si_t x, si_t y, si_t width , si_t height)//size the window
{
	widget_set_bounds(WIDGET_POINTER(w), x, y, width, height);
}

void desktop_set_color(struct window* w, struct color* fcolor, struct color* bcolor)
{
	widget_set_color(WIDGET_POINTER(w), fcolor, bcolor);
}
