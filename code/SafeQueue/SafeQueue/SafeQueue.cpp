
#include<Windows.h>
#include<processthreadsapi.h>
#include<cstdio>
#include"SafeQueue.h"
#include <ctime>

SpinQueue<LONG64> g_queue(10);

DWORD Queuing(LPVOID param)
{
	LONG64 cnt = 0;
	while (true)
	{
		LONG64 item = cnt;
		if (g_queue.push(item, 20)) {
			printf("Enqueued: %ld\n", item);
		}
		else {
			//printf("Failed to enqueue: %ld\n", item);
		}

		cnt++;
		if (item == 100000) break;
		Sleep(0);
	}
	return 0;
}

DWORD DeQueuing(LPVOID param)
{
	while (true)
	{
		LONG64 item;
		if (g_queue.pop(item, 20)) {
			printf("Dequeued: %ld\n", item);
		}
		else {
			//printf("Failed to dequeue\n");
		}
		if (item == 100000) break;
		Sleep(0);
	}

	return 0;
}

int main()
{
	srand(time(0));

	HANDLE threads[2]; 

	for (size_t i = 0; i < 2; ++i)
	{
		if ((i % 2) == 0)
		{
			threads[i] = CreateThread(NULL, 0, Queuing, NULL, 0, NULL);
		}
		else {
			threads[i] = CreateThread(NULL, 0, DeQueuing, NULL, 0, NULL);
		}

		if (threads[i] == NULL)
		{
			printf("Error creating thread %d: %d\n", i, GetLastError());
			return 1;
		}
	}

	WaitForMultipleObjects(2, threads, TRUE, INFINITE);
	return 0;
}

