#include "msg_queue.h"

#define _GNU_SOURCE
static int device_running = 1;
static device_msg_t my_msg;

int init(char *name);
int wait_for_ack();

int main(int argc, char *argv[])
{
	int opt;
	char command[25];
	char name[256]= "unnamed";
	struct option longopts[]={
		{"name", 1, NULL, 'n'},
	};
	while ((opt=getopt_long(argc,argv,"n:",longopts,NULL)) != -1)
	{
		switch(opt)
		{
			case 'n':
				strncpy(name,optarg,sizeof(name));
				break;
		}
	}
	//Init the sensor
	if (init(name)){
		printf("%d: - A actuator named %s created.\n",getpid(),name);
	}
	else
	{
		printf("%d: - Cannot init the actuator.\n",getpid());
		exit(EXIT_FAILURE);
	}
	//Wait for acknowledge from the controller
	if (!wait_for_ack()) exit(EXIT_FAILURE);

	//Start the operation
	while (device_running)
	{
		//check if there is msg from the controller
		device_receive_msg(command);
		if (strcmp(command,"on")==0)
		{
			printf("%d: - Alarm!!!: Actuator ON.\n",getpid());

		}
		if (strcmp(command,"stop")==0) 
		{
			printf("%d: - Actuator stop.\n",getpid());
			exit(EXIT_SUCCESS);
		}
	}
}
/*
** This method initialize the sensor
**
*/
int init(char *name)
{
	//construct the msg for this device
	my_msg.pid = getpid();
	strncpy(my_msg.name, name, sizeof(my_msg.name));
	my_msg.device_type = 'a';
	my_msg.threshold = -1000;
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
		printf("%d: - Actuator has been registered with controller.\n",getpid());
	}
	if (strcmp(command,"stop")==0) return 0;
	else return 1;
}
