CC = gcc
DEBUG= NODEBUG
CFLAGS = -Wall -c -D$(DEBUG)
LFLAGS = -Wall -D$(DEBUG)

all: sensor actuator controller cloud
sensor: sensor.o msg_queue.o
	$(CC) $(LFLAGS) sensor.o msg_queue.o -o sensor
actuator: actuator.o msg_queue.o
	$(CC) $(LFLAGS) actuator.o msg_queue.o -o actuator
controller: controller.o msg_queue.o fifo.o
	$(CC) $(LFLAGS) controller.o msg_queue.o fifo.o -o controller
cloud: cloud.o fifo.o
	$(CC) $(LFLAGS) cloud.o fifo.o -o cloud
controller.o: controller.c msg_queue.h fifo.h
	$(CC) $(CFLAGS) controller.c
sensor.o: sensor.c msg_queue.h
	$(CC) $(CFLAGS) sensor.c
actuator.o: actuator.c msg_queue.h
	$(CC) $(CFLAGS) actuator.c
cloud.o: cloud.c fifo.h
	$(CC) $(CFLAGS) cloud.c
msg_queue.o: msg_queue.c msg_queue.h
	$(CC) $(CFLAGS) msg_queue.c
fifo.o: fifo.c fifo.h
	$(CC) $(CFLAGS) fifo.c
clean:
	rm -rf *.o