/**
 * @file desktop.h
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

# ifndef _DESKTOP_H_
# define _DESKTOP_H_ 1

# include <base_type.h>
# include <comm.h> 
# include <widget.h>

# include "color_base.h"
# include "taskbar.h"
extern si_t bar_num;
extern si_t screen_w;
extern si_t screen_h;
extern struct window_style desktop_default_style;

///////////////////////////////////////////////////////
extern struct image_view * bar_img[];


/**
 * 将地址强制转换成 struct window *
 **/
# define WINDOW_POINTER(addr) ((struct window *)(addr))


extern si_t desktop_default_callback(addr_t w, addr_t m);

/**
 * @brief 重绘窗口
 **/
extern void desktop_repaint(struct window* w);

/**
 * @brief 重绘桌面任务栏
 **/
extern void desktop_bar_repaint(struct window* w);

/**
 * @brief 显示窗口
 **/
extern void desktop_show(struct window* w);

/**
 * @brief 重新显示窗口
 **/
extern void desktop_reshow(struct window* w);

/**
 * @brief 使用前设置窗口部件结构体
 *
 * @param title 窗口标题
 *
 * @return 0
 **/
extern struct window* desktop_init(char* title);

/**
 * @brief 使用后清理窗口部件结构体
 *
 * @param w 窗口部件结构体指针
 *
 * @return 0
 **/
extern si_t desktop_exit(struct window * w);

/**
 * @brief 设置控件区域
 * 参数若小于0则对应的位置不发生改变
 *
 * @param window 组件
 * @param x  组件左上角 x 
 * @param y  组件左上角 y
 * @param width  组件宽度
 * @param height 组件高度
 *
 **/ 
extern void desktop_set_bounds(struct window* w, si_t x, si_t y, si_t width , si_t height);

/**
 * @brief 设置基本颜色
 * 参数若为空则对应的位置不发生改变
 *
 * @param w 控件
 * @param fcolor 前景色
 * @param bcolor 背景色
 **/
extern void desktop_set_color(struct window* w, struct color* fcolor, struct color* bcolor);

# endif

