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
#ifndef DEBUG
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cjson.h"

 /**  添加数组元素**/
void add_arr_2_obj(cjson* obj, const char *name, cjson *arr) 
{
	if(!obj || !arr)
		return ;

	cJSON_AddItemToObject(obj,name,arr);

	return ;
}

 /**  添加子元素**/
void add_obj_2_obj(cjson *obj,char *name,cjson *item)
{
	if(!obj || !item)
		return ;

	cJSON_AddItemToObject(obj, name, item);

	return ;
}

/** 打印节点**/
void printf_cjson(cjson *root)
{
	char *out  = cJSON_Print(root);

	printf("stdout=%s\n", out);

	if(out)
		free(out);

	return ;
}

/** 添加整形到数组**/
void add_int_2_arr(cjson *arr, int num)
{
	cjson* json_num= cJSON_CreateNumber(num);

	cJSON_AddItemToArray(arr,json_num);

	return ;
}

/** 添加字符串到数组**/
void add_str_2_arr(cjson*arr, const char *str)
{
	cjson* json_str = cJSON_CreateString(str);

	cJSON_AddItemToArray(arr,json_str);

	return ;
}

 /**  添加数组到数组**/
void add_arr_2_arr(cjson *arr, cjson *obj)
{
	if(!obj || !arr)
		return ;

	cJSON_AddItemToArray(arr, obj);

	return ;
}

 /** 添加节点到数组元素 **/
void add_obj_2_arr(cjson *arr, cjson *obj)
{
	if(!obj || !arr)
		return ;

	cJSON_AddItemToArray(arr, obj);

	return ;
}
#else

#include "cjson.h"

void test_json()
{
	cjson *root = create_obj();

	add_str_2_obj(root, "pid", "10010");
	add_int_2_obj(root, "int", 123);

		cjson *arr = create_arr();
		add_int_2_arr(arr, 1);
		add_int_2_arr(arr, 2);
		add_str_2_arr(arr, "3");
		add_str_2_arr(arr, "4");

		cjson *arrarr = create_arr();
			add_int_2_arr(arrarr, 5);
			add_str_2_arr(arrarr, "6");
			add_arr_2_arr(arr, arrarr);


		cjson *arrobj = create_obj();
			add_int_2_obj(arrobj, "arr_obj_int", 5);
			add_str_2_obj(arrobj, "arr_obj_str", "6");
			add_obj_2_arr(arr, arrobj);
	

	add_arr_2_obj(root, "arr", arr);
	
	cjson *obj = create_obj();
		add_int_2_obj(obj, "arr_obj_int", 7);
		add_str_2_obj(obj, "arr_obj_str", "8");
		add_obj_2_obj(root, "child", obj);


	printf_cjson(root);

	free_obj(root);

	return ;
}

int main()
{
	test_json();
	return 0;
}
#endif
