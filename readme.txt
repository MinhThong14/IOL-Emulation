## How to compile
	>> make clean
	>> make
## How to use
Each step should be run on different terminals
Step 1: run the controller process
	>> ./controller
Step 2: run the cloud process
	>> ./cloud
Step 3: run the sensor process
	>> ./sensor -n "sensor_name" -t <sensor_threshold>
Step 4: run the actuator process
	>> ./actuator -n "actuator_name"
<< repeat step 3 and 4 to create more sensors and actuators >>
Step 5: Cltr + C on controller terminal to close all processes except the cloud