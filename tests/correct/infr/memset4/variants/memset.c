#include <stdio.h>
#include <string.h>

typedef struct my_struct_s {
	struct my_struct_s *prev;
	struct my_struct_s *next;
} my_struct;

typedef struct {
	int x;
	int y;
} my_struct_1;

int main()
{
	my_struct s = {0};
	my_struct_1 s1 = {0};
	return 0;
}
