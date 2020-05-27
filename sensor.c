/* Sensor.c
**
** This file create a sensor process emulating the behaviour of a sensor
**
** In the terminal: ./sensor -n "sensor_name" -t <sensor threshold>
**
**
*/
#include "msg_queue.h"
#include <time.h>

#define _GNU_SOURCE
static int device_running = 1;
static device_msg_t my_msg;

int init(char *name, int threshold);
int wait_for_ack(void);
int generate_data(void);

int main(int argc, char *argv[])
{
	int opt,threshold;
	char command[25];
	char name[256]="unnamed";

	struct option longopts[]={
		{"name", 1, NULL, 'n'},
		{"threshold", 1, NULL, 't'}
	};
	while ((opt=getopt_long(argc,argv,"n:t:",longopts,NULL)) != -1)
	{
		switch(opt)
		{
			case 'n':
				strncpy(name, optarg, sizeof(name));
				break;
			case 't':
				threshold = atoi(optarg);
				break;
		}
	}
	//Init the sensor
	if (init(name,threshold)){
		printf("%d: - A sensor named %s with threshold %d created.\n",getpid(),name,threshold);
	}
	else
	{
		printf("%d: - Cannot init the sensor.\n",getpid());
		exit(EXIT_FAILURE);
	}
	//Wait for acknowledge from the controller
	if (!wait_for_ack()) exit(EXIT_FAILURE);

	//Start the operation
	while (device_running)
	{
		//generate sensor data
		my_msg.current_value = generate_data();
		printf("%d: Current value:%d.\n",getpid(),my_msg.current_value);
		//send data to msg queue
		device_send_msg(my_msg);
		//check if there is msg from the controller
		device_receive_msg(command);
		if (strcmp(command,"stop")==0) 
		{
			printf("%d: - Sensor stop.\n",getpid());
			exit(EXIT_SUCCESS);
		}
		//sleep for 10 seconds
		sleep(10);
	}
}
/*
** This method generate random sensor data 
**
*/
int generate_data(void)
{
	int sign, value;

	sign = rand() % 2; // generate sign; 0 : negative; 1: positive
	value = (rand() % 101); // data return from 0 - 100

	if (sign) return value;
	else return -value;
}
/*
** This method initialize the sensor
**
*/
int init(char *name, int threshold)
{
	//seed the random generator
	srand(time(NULL));
	//construct the msg for this device
	my_msg.pid = getpid();
	strncpy(my_msg.name, name, sizeof(my_msg.name));
	my_msg.device_type = 's';
	my_msg.threshold = threshold;
	my_msg.current_value = -1000;
	//Open the msg queue
	if (!open_msg_queue()) return 0;
	//Send first message
	return device_send_msg(my_msg);
}
/*
** This method wait for an acknowledgement msg from the controller
**
**/
int wait_for_ack()
{
	char command[25];
	//wait for msg acknowledge from the controller
	while (!device_receive_msg(command)) {};
	if (strcmp(command,"ack")==0)
	{
		printf("%d: - Sensor has been registered with controller.\n",getpid());
	}
	if (strcmp(command,"stop")==0) return 0;
	else return 1;
}
