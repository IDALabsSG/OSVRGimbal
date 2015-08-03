#OSVR
#####Documentation

###### Introduction

This project is designed to use OSVR headtracking to control the camera gimbal on a Quadcopter.

[**OSVR**] ---*connected to*--> [**PC**]---*via USB*-->[**Arduiono Board**]---->[**Radio Transmitter**]--->[**Drone**]

The windows executable in the Visual C folder will take readings from OSVR headtracking and sends it via serial to Arduino(Uno), which will send PWM via the radio transmitter.

###### Folders

- Visual C
        To be compiled with Visual C and OSVR SDK

- Arduino
        Codes need to be deployed to the arduino.

