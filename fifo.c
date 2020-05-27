#include "fifo.h"
/*
** This method create a FIFO
** The method is called by a parent process of controller at start-up
**
*/
int create_FIFO()
{
	int ret;
	if (access(FIFO_NAME, F_OK) == -1)
	{
		ret = mkfifo(FIFO_NAME,0777);
		if (ret != 0)
		{
			printf("%d: - ERROR: Cannot create a FIFO.\n",getpid());
			return 0;
		}
	}
#ifdef DEBUG
	printf("%d: - A FIFO has been created.\n",getpid());
#endif
	return 1;
}
/*
**
** This method write message to the FIFO
** The method will be called by the parent process of controller
** The method uses blocking write FIFO
**
*/
int write_FIFO(char *msg, int msg_length)
{
	int pipe_fd;
#ifdef DEBUG
	printf("%d: - Writing msg to FIFO.\n",getpid());
#endif
	pipe_fd = open(FIFO_NAME, write_mode);
	if (pipe_fd != -1)
	{
		if (write(pipe_fd, msg, msg_length) == -1)
		{
			printf("%d: - ERROR: Write error on fifo pipe.\n",getpid());
			return 0;
		}
		(void)close(pipe_fd);

	}
	else
	{
		printf("%d: - ERROR: Cannot open FIFO for writting.\n",getpid());
		return 0;
	}
	return 1;
}
/*
** This method read message from the FIFO
** The method will be called by the cloud process
** The method uses blocking read FIFO
**
*/
int read_FIFO(char *msg, int msg_length)
{
	int pipe_fd;
#ifdef DEBUG
	printf("%d: - Reading msg from FIFO.\n",getpid());
#endif
	pipe_fd = open(FIFO_NAME, read_mode);
	if (pipe_fd != -1)
	{
		if (read(pipe_fd,msg,msg_length) == -1)
		{
			printf("%d: - ERROR: Read error on fifo pipe.\n",getpid());
			return 0;
		}
		(void)close(pipe_fd);
	}
	else
	{
		printf("%d: - ERROR: Cannot open FIFO for reading.\n",getpid());
		return 0;
	}
	return 1;
}