#include <cstdio>
#include <cstdlib>
int main()
{
	system("chcp 437");
	for(int i=0;i<128;i++)
		printf("0x%x: %c\n",i,i);
}
