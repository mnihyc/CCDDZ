#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <windows.h>
int main()
{
	while(1)
	{
		srand(clock());
		int a[3]={0,1,2};
		std::random_shuffle(&a[0],&a[3]);
		std::random_shuffle(&a[0],&a[3]);
		std::random_shuffle(&a[0],&a[3]);
		printf("%d",a[0]);
		Sleep(100);
	}
}
