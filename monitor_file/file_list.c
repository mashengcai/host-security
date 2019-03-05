/**
 * @file  file_list.c 
 * @brief 防篡改:链表以及hash各种操作
 * @author    yu
 * @date     2017-10-27
 * @version  A001 
 * @copyright yu                                                              
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <assert.h>

#include "file_list.h"

static file_list_s * gsp_file_list = NULL;
static file_hash_s** gspp_hash_table  = NULL;

static file_list_s * create_node(void);
static int read_file_to_list(const char * path);

int malloc_num;

/**  
 * list_malloc申请内存空间
 * @param[in]   size 内存大小
 * @retval  OK  内存地址 
 * @retval  ERROR   NULL  
 * @par 修改日志 
 *      yu于2017-05-08创建 
 */ 
void * list_malloc(size_t size)
{
	malloc_num++;
	return malloc(size);
}
/**  
 * list_free释放内存空间
 * @param[in]   ptr 内存地址 
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */
void list_free(void * ptr)
{
	malloc_num--;
	free(ptr);
	return ;
}

/**  
 * push_list压入节点到链表头
 * @param[in]   list 链表节点 
 * @param[out]  head 链表头  
 * @retval  OK  	0 
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */
int push_list(file_list_s **head ,file_list_s *list)
{
	assert(head != NULL || list != NULL);
	
	if(*head == NULL)
	{
		*head = list;
		return 0;
	}

	list->next = *head;
	*head = list;

	return 0;
}
/**  
 * push_file	路径压入链表
 * @param[in]   path 路径名  
 * @retval  OK  	0 
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */
int push_file(const char * path)
{
	file_list_s * node = NULL;
	
	assert(path != NULL);
	
	if(path == NULL || path[0] == '\0')
	{
		
		return -1;	
	}
	node = create_node();
	if( NULL == node)
	{
		return -2;	
	}

	memcpy(node->path, path, sizeof(node->path));

	push_list( &gsp_file_list, node );	

	return 0;
}
/**  
 * create_node创建链表节点 
 * @retval  OK  节点地址
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */
static file_list_s * create_node(void)
{

	file_list_s * node = list_malloc( sizeof(file_list_s) );
	if( node <= 0 )
	{
		return NULL;
	}
	memset(node,0,sizeof(file_list_s));

	return node;
}

/**  
 * free_node释放链表节点
 * @param[in]   node 链表节点 
 * @retval  OK  	0 
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */
int free_node(file_list_s *node)
{
	assert( node != NULL );
	list_free(node);
	return 0;
}

/**  
 * free_all_node释放链表头下所有节点
 * @param[in]   list 链表节点 
 * @retval  OK  	0 
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */
int free_file_list(file_list_s **list)
{
	assert( list != NULL );
	if(*list == NULL)
	{
		return -1;
	}

	file_list_s *next = *list;
	file_list_s *node = NULL;

	gsp_file_list = NULL;

	while(next != NULL)
	{	
		node = next;
		next = next->next;
		list_free(node);
	}
	*list = gsp_file_list;
	return 0;
}

/**  
 * get_file_list 对外接口，获取文件结构体
 * @param[in]   path 路径
 * @param[out]  list 输出结构体
 * @retval  OK  内存地址 
 * @retval  ERROR   NULL  
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */ 
int get_file_list(const char *path, file_list_s **list)
{
	struct stat ft = {0};
	int ret = 0;
	
	assert( path != NULL || list != NULL);
	
	if( lstat(path, &ft)!=0)
	{
		return -2;
	}
	if( S_ISDIR(ft.st_mode) )   ///dir
	{
		ret = push_file(path) ;

		if( 0 != ret)
		{
			return -3;
		}

		ret = read_file_to_list(path);
		if(ret != 0)
		{
			list = NULL;
			printf("get_file_list error\n");
			return -1;
		}
	}

	*list = gsp_file_list;
	
	return 0;
}
/**  
 * read_file_to_list 读取目录路径并放入链表中
 * @param[in]   path 路径
 * @retval  ERROR   NULL  
 * @par 修改日志 
 *      yu于2017-10-27创建  
 */ 
static int read_file_to_list(const char * path)
{
	DIR *dir;
	struct dirent *ptr;
	struct stat ft = {0};
	int ret = 0;

	char base[1024];
	assert( path != NULL );
	
	if(path == NULL || path[0] == '\0')
	{
		return -1;
	}

	if ((dir = opendir(path)) == NULL)
	{
		perror("Open dir error...");
		return -2;
	}

	while ((ptr = readdir(dir)) != NULL)
	{
		if( strcmp(ptr->d_name,".") == 0 || strcmp(ptr->d_name, "..") == 0 )    ///current dir OR parrent dir
		{
			continue;
		}		

		memset(base, 0, sizeof(base));

		if(path[strlen(path) - 1] == '/')
			sprintf(base, "%s%s",path, ptr->d_name);
		else
			sprintf(base, "%s/%s",path, ptr->d_name);

		if( lstat(base, &ft)!=0)
		{
			continue;
		}
		if( S_ISDIR(ft.st_mode) )   ///dir
		{
			ret = push_file(base) ;

			if( 0 != ret)
			{
				return -3;
			}

			read_file_to_list(base);
		}
	}
	closedir(dir);
	return 0;
}



//file_hash_s

/*static int hashfunc(unsigned int wd)
  {
  unsigned int value;
  int hash;

  value = wd;
  hash = 0;
  do {
  hash ^= value;
  } while ((value >>= HASH_LOG));

  return hash & (HASH_SIZE - 1);
  }*/
 /**  
 * hashfunc 	哈希函数
 * @param[in]   wd	文件描述符
 * @retval  	key key值  
 * @par 修改日志 
 *      yu于2017-10-27创建  
 */ 
static int hashfunc(unsigned int wd)
{
	unsigned int value;

	value = wd % HASH_TABLE_SIZE;

	return value;
}
 /**  
 * hashfunc 	哈希函数
 * @param[in]   wd	文件描述符
 * @retval  	key key值  
 * @par 修改日志 
 *      yu于2017-10-27创建  
 */ 
file_hash_s * hash_free_node(file_hash_s * node)
{
	assert( node != NULL );
	
	list_free(node);

	return node;
}
 /**  
 * hash_init_node 	初始化一个hash结构体
 * @param[in]   wd		 文件描述符
 * @param[in] 	dir_path 文件路径  
 * @par
 *      yu于2017-10-27创建 
 */ 
file_hash_s *hash_init_node(const int wd, const char * dir_path)
{
	assert( dir_path != NULL );
	
	file_hash_s * node = list_malloc( sizeof(file_hash_s) );
	if( node <= 0 )
	{
		return NULL;
	}
	
	memset(node, 0, sizeof(file_hash_s));

	node->wd = wd;
	memcpy(node->dir_path, dir_path, DIR_PATH_LEN);
	node->next_hash = NULL;

	return node;
}
 /**  
 * hash_push_table 	存储一个hash结构体
 * @param[in] 	key key值  
 * @param[in] 	value 值  
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */ 
int hash_push_table(unsigned int key, file_hash_s * value)
{

	assert( gspp_hash_table != NULL || value != NULL );
	
	if(gspp_hash_table == NULL || value == NULL)
	{
		return -1;
	}

	file_hash_s **head = &gspp_hash_table[key];

	//更新节点
	for(; *head != NULL; head = &((*head)->next_hash) )
	{
		if( (*head)->wd == value->wd )
		{
			memcpy((*head)->dir_path, value->dir_path, DIR_PATH_LEN);
			hash_free_node(value);
			return 0;
		}
	}
	//放入
	if( *head == NULL )
	{
		*head = value;
		return 0;
	}
	return -2;

}
 /**  
 * hash_get_value 	根据key值获取hash结构体
 * @param[in]   gspp_hash_table	哈希表
 * @param[in] 	key key值  
 * @param[in] 	value 值  
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */ 
char* hash_get_value(unsigned int wd)
{
	assert( gspp_hash_table != NULL );
	
	if(gspp_hash_table == NULL)
	{
		return NULL;
	}
	file_hash_s *head = gspp_hash_table[hashfunc(wd)];

	for(; head != NULL; head = head->next_hash )
	{
		if( head->wd == wd )
		{
			return head->dir_path;
		}
	}
	return NULL;
}
int hash_get_key(const char * path)
{
	assert( gspp_hash_table != NULL );

	int i = 0;
	
	if(gspp_hash_table == NULL)
	{
		return -1;
	}
	for(i = 0;i < HASH_TABLE_SIZE; i++)
	{
		file_hash_s *head = gspp_hash_table[hashfunc(i)];

		for(; head != NULL; head = head->next_hash )
		{
			if( strcmp(path, head->dir_path) == 0)
			{
				return head->wd;
			}
		}
	}
	return -2;
}
 /**  
 * hash_init_table 	初始化一个hash表
 * @param[in]   hash_size	大小
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */
int hash_init_table(unsigned int hash_size)
{
	gspp_hash_table = (file_hash_s** )list_malloc( sizeof(file_hash_s*) * hash_size);
	if( gspp_hash_table <= 0 )
	{
		return -1;
	}
	memset(gspp_hash_table, 0, sizeof(file_hash_s*) * hash_size);

	return 0;
}
 /**  
 * hash_push 	根据key值获取hash结构体
 * @param[in]   gspp_hash_table	哈希表
 * @param[in] 	key key值  
 * @param[in] 	value 值  
 * @par 修改日志 
 *      yu于2017-10-27创建 
 */ 
int hash_push(const int wd, const char * dir_path)
{
	
	assert( gspp_hash_table != NULL || dir_path != NULL);

	file_hash_s * hash_node = hash_init_node(wd, dir_path);
	if(hash_node == NULL)
	{
	  return -1;
	}
	if( 0 != hash_push_table(hashfunc(wd), hash_node) )
	{
	  hash_free_node(hash_node);
	  return -2;
	}
	return 0;
}

int hash_pop(const int wd)
{
	int key = 0;
	key = hashfunc(wd);
	
	file_hash_s **head = &gspp_hash_table[key];

	//更新节点
	for(; *head != NULL; head = &((*head)->next_hash) )
	{
		if( (*head)->wd == wd )
		{
			memset((*head)->dir_path, 0, DIR_PATH_LEN);
			return 0;
		}
	}
	return -1;
}
#ifdef TEST
//test
void print_list(file_list_s * head)
{
	int n = 1;
	while(head != NULL)
	{
		printf("list %d == ip %s \n",n++,head->path);
		fflush(stdout);
		head = head->next;
	}
}
void test_hash()
{
	// file_hash_s ** tables = hash_init_table(HASH_TABLE_SIZE);

	// char *buf = "/var/www/user";
	// char buff[128] = {0};

	// int i = 0 ;
	// for(i = 0; i< 100;i++)
	// {
		// sprintf(buff, "%s/id=%d",buf, i);	
		// file_hash_s * node = hash_init_node(i, buff);
		
		// hash_push_table(tables, hashfunc(i), node);
	// }

	// for(i = 0; i< 100;i++)
	// {
		// file_hash_s * node = hash_get_value(tables, i);
		// printf("i = %d, wd = %d, path = %s\n", i , node->wd, node->dir_path);
	// }
	
	// for(i = 0; i< 100;i++)
	// {
		// sprintf(buff, "%s/ex_ex_ex_ex=%d",buf, i);	
		// file_hash_s * node = hash_init_node(i, buff);
		
		// hash_push_table(tables, hashfunc(i), node);
	// }
	// for(i = 0; i< 100;i++)
	// {
		// file_hash_s * node = hash_get_value(tables, i);
		// printf("i = %d, wd = %d, path = %s\n", i , node->wd, node->dir_path);
	// }
	return ;
}
void test1(char * path)
{
	//后期有更改，可能测试例子需要调整，17.3.8
	//1.1 链表测试 log_list_s
	file_list_s * test_list = NULL;

	get_file_list(path, &test_list);

	print_list(test_list);
	free_file_list(&test_list);
	
	printf("%d\n",malloc_num);
}


int main(int argn, char ** argv)
{
	char *path = "/mnt/hgfs/share/test/inotify/wordpress/123";
	test1(path);
	test1(path);
	
	//test_hash();
	return 0;
}
#endif
