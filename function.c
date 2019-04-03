#include "struct.h"


int function_1(file_s *pos)
{
	printf("%s %s\n", __FUNCTION__, pos->file_name);
	return 0;
}
int function_2(file_s *pos)
{
	printf("%s %s\n", __FUNCTION__, pos->file_name);

	return 0;
}
