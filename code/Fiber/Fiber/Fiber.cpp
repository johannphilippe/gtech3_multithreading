#include<Windows.h>
#include<cstdio>

int global = 0;

static void FiberFunction(LPVOID param)
{
	while (true) {
		global++;
		printf("Fiber : Global is now %d \n", global);
		Sleep(300);
		SwitchToFiber(param);
	}
}

int main()
{
	LPVOID mainFiber = ConvertThreadToFiber(NULL);

	LPVOID param = mainFiber;
	LPVOID fiber_ptr = CreateFiber(0, FiberFunction, param);
	if (fiber_ptr == NULL)
	{
		printf("Error creating Fiber\n");
		return 1;
	}
	while (true)
	{
		SwitchToFiber(fiber_ptr);
		global--;
		printf("Main : Global is now %d \n", global);
		Sleep(300);
	}
	return 0;
}