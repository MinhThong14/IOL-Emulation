/* msg_queue.h
**
**
** This file implement the named MSG QUEUE interface to use between the controller 
** and the devices
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

#define DEVICE_MSG 1
#define CONTROLLER_MSG 2
#define MQUEUE_PATH "/tmp/MQUEUE"

/* Define msg struct that a device uses to pass to the queue */
typedef struct {
	pid_t pid; // process id of a device
	char name[256]; // name of a device
	char device_type; // type of a device (s:sensor, a:actuator)
	int threshold; // threshold for sensor; -1000 for actuator
	int current_value; // current value of sensor; -1000 for actuator
} device_msg_t;

/* Define msg struct that the controller's child process send to parent process */
typedef struct{
	char name[256];
	pid_t pid;
	int sensing_data;
	char action[256];
} child_msg_t;

/* provide method to open and close the message queue */
int open_msg_queue(void);
int close_msg_queue(void);
/* provide methods for device (sensors, actuators) to send and to recieve message */
int device_send_msg(device_msg_t msg);
int device_receive_msg(char *command);
/* provide methods for controller to send and recieve msg */
int controller_send_msg(pid_t device_pid, char *command);
int controller_send_msg_to_parent(child_msg_t child_msg);
int controller_receive_msg(device_msg_t *device_msg);
int controller_receive_msg_from_child(child_msg_t *child_msg);
