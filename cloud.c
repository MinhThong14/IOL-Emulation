#include "fifo.h"

#define _GNU_SOURCE
static int running = 1;
int main(int argc, char *argv[])
{
	char msg[256];
	printf("%d: - Cloud started.\n",getpid());
	while(running)
	{
		read_FIFO(msg,256);
		printf("%d: - MSG: %s",getpid(),msg);
	}
}