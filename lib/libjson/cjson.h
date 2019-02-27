/**
 * * @file  cjson.c
 * * @brief 根据cJSON封装接口
 * * @author    yu
 * * @date     2018-5-28
 * * @version  A001 
 * * @copyright yu
 * * @par History: 
 *  update:yu 2018-5-28\n
 *  接口说明
 *  创建节点：		cjson *obj = create_obj();\n
 *  创建数组节点：	cjson *arr = create_arr();\n
 *                     */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cJSON.h"

typedef cJSON cjson;

 /**  创建节点 **/
#define create_obj cJSON_CreateObject

 /**  创建数组节点 **/
#define create_arr cJSON_CreateArray

 /**  创建数组并初始化 **/
#define init_str_arr(s,n) cJSON_CreateStringArray(s,n)

 /**  创建数组并初始化 需设置步长**/
#define init_str_arr_ex(s,l,n) cJSON_CreateStringArray_ex((char *)s,l,n)

 /**  添加整形元素**/
#define add_int_2_obj(obj,name,num) do{if(obj)cJSON_AddNumberToObject(obj,name,num);}while(0)

 /**  添加字符串元素**/
#define add_str_2_obj(obj,name,str) do{if(obj)cJSON_AddStringToObject(obj,name,str);}while(0)

/** 释放节点 **/
#define free_obj  cJSON_Delete

 /**  添加数组元素**/
void add_arr_2_obj(cjson* obj, const char *name, cjson *arr);

 /**  添加子元素**/
void add_obj_2_obj(cjson *obj,char *name,cjson *item);

/** 打印节点**/
void printf_cjson(cjson *root);

/** 添加整形到数组**/
void add_int_2_arr(cjson *arr, int num);

/** 添加字符串到数组**/
void add_str_2_arr(cjson*arr, const char *str);

 /**  添加数组到数组**/
void add_arr_2_arr(cjson *arr, cjson *obj);

 /** 添加节点到数组元素 **/
void add_obj_2_arr(cjson *arr, cjson *obj);

