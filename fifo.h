/* fifo.h
** This file implement the named pipe interface to use between the controller 
** parents and child processes
** 
**
*/
#include <stdlib.h>
#include <stdio.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/msg.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>

#define FIFO_NAME "/tmp/fifo"
#define BUFFER_SIZE PIPE_BUF
#define read_mode O_RDONLY
#define write_mode O_WRONLY

int create_FIFO();
int read_FIFO();
int write_FIFO();