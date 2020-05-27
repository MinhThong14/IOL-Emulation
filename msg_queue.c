/* msg_queue.c
**
**
** This file implement the named MSG QUEUE interface to use between the controller 
** and the devices
**
** 
*/
#include "msg_queue.h"

#define CONTROL_CMD_LENGTH 25

static int qid = -1;
/* define msg that is used to passed to the queue */
struct device_msg_passed{
	long int msg_type;
	device_msg_t msg;
};
struct control_msg_passed{
	long int msg_type;
	char command[CONTROL_CMD_LENGTH];
};
struct child_msg_passed{
	long int msg_type;
	child_msg_t child_msg;
};
/*
** Open the message queue 
** Devices need to open the queue before sending/recieving msg
**
*/
int open_msg_queue()
{
#ifdef DEBUG
	printf("%d: - open msg queue.\n", getpid());
#endif
	qid = msgget(ftok(MQUEUE_PATH,1), 0666 | IPC_CREAT);
	if (qid == -1) return(0);
	return (1);
}
/*
** Close the message queue 
** This method can only be done once by the controller
**
*/
int close_msg_queue()
{
	(void)msgctl(qid, IPC_RMID,0);
#ifdef DEBUG
	printf("%d: - close msg queue.\n", getpid());
#endif
	qid = -1;
	return 1;
}
/*
** Devices send msg to the queue
** Sensors/actuators use this method to send msg to the queue
**
*/
int device_send_msg(device_msg_t msg)
{
	struct device_msg_passed my_msg;
	int ret;
#ifdef DEBUG
	printf("%d: - device send msg.\n",getpid());
#endif
	my_msg.msg_type = DEVICE_MSG;
	my_msg.msg = msg;

	ret = msgsnd(qid, (void*)&my_msg, sizeof(msg), 0);
	if (ret == -1){
		printf("%d: - Message send failed.\n", getpid());
		return(0);
	}
	return(1);
}
/*
** Devices receive msg from the queue
** Sensors/actuators use this method to retrieve msg from the queue
*/
int device_receive_msg(char *command)
{
	struct control_msg_passed my_msg;
	int ret;
#ifdef DEBUG
	printf("%d: - devices receives msg.\n", getpid());
#endif
	// Retrieve msg that sent by controller (msg_type = CONTROLLER_MSG)
	ret = msgrcv(qid, (void *)&my_msg, CONTROL_CMD_LENGTH, getpid(), 0);

	if (ret == -1) return(0);

	strncpy(command, my_msg.command, CONTROL_CMD_LENGTH);
	return(1);

}
/*
** Controller sends msg to the queue
** Controlller use this method to send msg to the queue
**
*/
int controller_send_msg(pid_t device_pid, char *command)
{
	struct control_msg_passed my_msg;
	int ret;
#ifdef DEBUG
	printf("%d: - controller sends msg.\n", getpid());
#endif
	my_msg.msg_type = device_pid;
	strncpy(my_msg.command, command, sizeof(my_msg.command));

	ret = msgsnd(qid, (void*)&my_msg, CONTROL_CMD_LENGTH, 0);
	if (ret == -1){
		printf("%d: - Message send failed.\n", getpid());
		return(0);
	}
	return(1);
}
/*
** The child process in the controller use this method to
** ...send mesg to the parent process
**
*/
int controller_send_msg_to_parent(child_msg_t child_msg)
{
	struct child_msg_passed my_msg;
	int ret;
#ifdef DEBUG
	printf("%d: - child process send msg to parent.\n", getpid());
#endif
	my_msg.msg_type = CONTROLLER_MSG;
	my_msg.child_msg = child_msg;

	ret = msgsnd(qid, (void*)&my_msg, sizeof(child_msg), 0);
	if (ret == -1)
	{
		printf("%d: - Message send failed.\n",getpid());
		return(0);
	}
	return(1);
}
/*
** Controller recieve msg from the queue
** Controller use this method to retrieve msg from the queue
** 
*/
int controller_receive_msg(device_msg_t *device_msg)
{
	struct device_msg_passed my_msg;
	int ret;
#ifdef DEBUG
	printf("%d: - controller receives msg.\n",getpid());
#endif
	// Retrieve all msg that sent by devices
	ret = msgrcv(qid, (void *)&my_msg, sizeof(*device_msg), DEVICE_MSG, 0);

	if (ret == -1) return(0);

	*device_msg = my_msg.msg;
	return(1);
}
/* The parent process in the controller use this method to
** .. receive mesg from the chile process
**
*/
int controller_receive_msg_from_child(child_msg_t *child_msg)
{
	struct child_msg_passed my_msg;
	int ret;
#ifdef DEBUG
	printf("%d: - parent process receives msg from child.\n",getpid());
#endif
	//Retrieve msg that send by the child process
	ret = msgrcv(qid, (void *)&my_msg, sizeof(*child_msg), CONTROLLER_MSG, 0);

	if (ret == -1) return(0);

	*child_msg = my_msg.child_msg;
	return(1);
}
