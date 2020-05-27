#include "msg_queue.h"
#include "fifo.h"
#include <signal.h>
#include <string.h>

#define _GNU_SOURCE
#define MAX_DEVICES 100 //maximum number of devices that controller can handle

/* data struct storing device information */
typedef struct {
	pid_t pid; //pid of device
	char name[256]; //name of device
	int threshold;// threshold value if sensor, -1000 if actuator
} device;
/* array of sensors connect to the controller */
static device sensor[MAX_DEVICES];
static int sensor_count=0;
/* array of actuators connect to the controller */
static device actuator[MAX_DEVICES];
static int actuator_count=0;
static int child_pid;
static int device_running = 1;

int init(void);
int device_Handler(void);
int signal_Handler(void);
void child_Handler(int sig);
int is_new_device(pid_t pid);
pid_t find_matching_actuator(pid_t sensor_pid);
void parent_close(int sig);
void child_close_devices(int sig);

int main(int argc, char *argv[])
{
	pid_t pid;

	//Initialize the controller
	if (!init()) parent_close(0);
	//Create a child process
	pid = fork();
	switch(pid)
	{
		case -1:
			printf("%d: fork failed\n.",getpid());
			parent_close(0);
		case 0:
			// The child process handles all connected devices
			device_Handler();
			break;
		default:
			// The parent process handles signals from the child
			child_pid = pid;
			signal_Handler();
			break;
	}
	return 1;
}

/*
** Initialize the controller
**
*/
int init()
{
	/* Open the message queue */	
	if (!open_msg_queue()) return 0;
	//close the message queue to flush all messages;
	while (!close_msg_queue()) {};
	/* Open the message queue again*/	
	if (!open_msg_queue()) return 0;
	if (!create_FIFO()) return 0;
	printf("%d: - Controller has been created.\n",getpid());
	return 1;
}
/*
** This method handle the communicate with devices connected
** ...to the controller
**
** If new device, send acknowledge message and
** ...register device to the device list
**
** If existing device, read and process msg from device
*/
int device_Handler()
{
	device_msg_t device_msg;
	child_msg_t child_msg;
	int matching_actuator_index;
	struct sigaction act;

	/* Handle signal from the parent */
	act.sa_handler = child_close_devices;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	sigaction(SIGINT, &act, 0);
	/* Handle request from devices */
	while (device_running)
	{
		if (controller_receive_msg(&device_msg))
		{
			// if the device is new
			if (is_new_device(device_msg.pid))
			{
#ifdef DEBUG
				printf("%d: - Recieved a msg from a new device with pid %d.\n",getpid(),device_msg.pid);
#endif
				//register a sensor
				if (device_msg.device_type == 's'){
					if (sensor_count < MAX_DEVICES)
					{
						sensor[sensor_count].pid = device_msg.pid;
						strcpy(sensor[sensor_count].name,device_msg.name);
						sensor[sensor_count].threshold = device_msg.threshold;
						sensor_count++;
						//send acknowledge message
						controller_send_msg(device_msg.pid,"ack");
						printf("%d: - Registered a new sensor with pid %d.\n",getpid(),device_msg.pid);
					}
					else
					{
						//if cannot register the sensor
						controller_send_msg(device_msg.pid,"stop");
					}
					
				}
				//register an actuator
				if (device_msg.device_type == 'a'){
					if (actuator_count < MAX_DEVICES)
					{
						actuator[actuator_count].pid = device_msg.pid;
						strcpy(actuator[actuator_count].name,device_msg.name);
						actuator[actuator_count].threshold = -1000;
						actuator_count++;
						//send acknowledge message
						controller_send_msg(device_msg.pid,"ack");
						printf("%d: - Registered a new actuator with pid %d.\n",getpid(),device_msg.pid);
					}
					else
					{
						//if cannot register the actuator
						controller_send_msg(device_msg.pid,"stop");
					}
				}

			}
			// if the device is already registered
			else
			{
				//send receive acknowledgement to device
				controller_send_msg(device_msg.pid,"rev");
#ifdef DEBUG
				printf("%d: - Recieve a msg from an existing device with pid %d.\n",getpid(),device_msg.pid);
#endif
				//check if sensor value exceeds the pre-configured threshold
				if (device_msg.device_type == 's' && device_msg.current_value > device_msg.threshold) 
				{
					//print message to terminal
					printf("%d: - Alarm!!! Sensor named:%s,pid:%d has value:%d exceeds threshold:%d.\n",\
								getpid(),device_msg.name,device_msg.pid,\
										device_msg.current_value,\
										 device_msg.threshold);
					//Raise signal to parent process
					kill(getppid(),SIGALRM);
					//find an actuator that needs to be triggered
					matching_actuator_index = find_matching_actuator(device_msg.pid);
					//printf("%d: - matching actuator_count: %d",getpid(),matching_actuator);
					if (matching_actuator_index!= -1)
					{
						//send message to trigger the actuator
						controller_send_msg(actuator[matching_actuator_index].pid,"on");
						//print message to terminal
						printf("%d: - Turn on actuator named:%s with pid %d.\n",getpid(),\
										actuator[matching_actuator_index].name,\
										actuator[matching_actuator_index].pid);
					}
					//Create a message for parent process in the message queue
					child_msg.pid=device_msg.pid;
					strcpy(child_msg.name,device_msg.name);
					strcpy(child_msg.action,"Turn on matching actuator named:");
					sprintf(child_msg.action, "%s%s",child_msg.action,actuator[matching_actuator_index].name);
					strcat(child_msg.action," with PID:");
					sprintf(child_msg.action, "%s%d.\n",child_msg.action,actuator[matching_actuator_index].pid);
					child_msg.sensing_data = device_msg.current_value;
					controller_send_msg_to_parent(child_msg);
				}
			}
		}
	}
	return 1;
}
/*
** The parent process uses this method to handle all signals
**
**
*/
int signal_Handler()
{
	struct sigaction act1, act2;
	/* Handle signal from the child */
	act1.sa_handler = child_Handler;
	sigemptyset(&act1.sa_mask);
	act1.sa_flags = 0;
	sigaction(SIGALRM, &act1, 0);
	/* Handle the Ctlr+C signal to close all process*/
	act2.sa_handler = parent_close;
	sigemptyset(&act2.sa_mask);
	act2.sa_flags = 0;
	sigaction(SIGINT, &act2, 0);
	while (device_running) {}
	return 1;
}
/*
** This method read message from the child process 
** ...and send it to the Cloud via FIFO
**
*/
void child_Handler(int sig)
{
	int ret;
	child_msg_t child_msg;
	char msg[256]="";
	ret = controller_receive_msg_from_child(&child_msg);
	if (ret)
	{
		strcpy(msg, "Sensor named: ");
		strcat(msg, child_msg.name);
		strcat(msg, ",pid: ");
		sprintf(msg,"%s%d",msg,child_msg.pid);
		strcat(msg," with sensing data: ");
		sprintf(msg,"%s%d",msg,child_msg.sensing_data);
		strcat(msg," exceeds threshold.");
		strcat(msg,child_msg.action);
		// write the msg to the FIFO
		write_FIFO(msg,strlen(msg));
	}
}
/* 
** This method close all related processed and exit the processed
**
**
*/
void parent_close(int sig)
{
	printf("%d: Parent prepares to close.\n",getpid());
	//exit
	exit(EXIT_SUCCESS);
}
/*
** The child process use this method to close stop all connected devices
**
**
*/
void child_close_devices(int sig)
{
	int i;
	printf("%d: Child prepares to close.\n",getpid());
	//close all sensor processes
	for (i=0; i<sensor_count; i++)
	{
		controller_send_msg(sensor[i].pid, "stop");
 	}
	//close all actuator processes
	for (i=0; i<actuator_count; i++)
	{
		controller_send_msg(actuator[i].pid, "stop");
	}
	//exit
	exit(EXIT_SUCCESS);
}
/* 
** Check if the device is new
**
*/
int is_new_device(pid_t pid)
{
	int i;
	for (i=0; i<sensor_count; i++)
	{
		if (pid == sensor[i].pid) return 0;
	}
	for (i=0; i<actuator_count; i++)
	{
		if (pid == actuator[i].pid) return 0;
	}
	return 1;
}
/*
** Find matching actuator
** This functuon return an actuator index that
** .. match to a sensor_pid
*/
pid_t find_matching_actuator(pid_t sensor_pid)
{
	int i;
	for (i=0; i<sensor_count; i++)
	{
		if (sensor[i].pid == sensor_pid) break;
	}
	// if there is no matching actuator
	if (i>(actuator_count -1)) return -1;

	//return the matching actuator pid
	return i;
}



