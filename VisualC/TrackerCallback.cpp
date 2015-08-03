


//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


// Parts of codes by : Sensics, Inc.
// 

// Alvin Ng

// Internal Includes
#include <osvr/ClientKit/Context.h>
#include <osvr/ClientKit/Interface.h>
#include <Windows.h>
#include <time.h>
#include <sstream>


// Library/third-party includes
// - none

#include "SerialSend.h"

#include "quaternion.h"



// Standard includes
#include <iostream>



//global var hacks so that callbacks will work
clock_t now_t, last_t;
SerialSend ssend;
bool serial_setup_success = FALSE;

Quaternion qtn;

float startangle = 9999;

void myTrackerCallback(void * /*userdata*/,
                       const OSVR_TimeValue * /*timestamp*/,
                       const OSVR_PoseReport *report) {
    std::cout << "Got POSE report: Position = ("
              << report->pose.translation.data[0] << ", "
              << report->pose.translation.data[1] << ", "
              << report->pose.translation.data[2] << "), orientation = ("
              << osvrQuatGetW(&(report->pose.rotation)) << ", ("
              << osvrQuatGetX(&(report->pose.rotation)) << ", "
              << osvrQuatGetY(&(report->pose.rotation)) << ", "
              << osvrQuatGetZ(&(report->pose.rotation)) << "))" << std::endl;
}

void myOrientationCallback(void * /*userdata*/,
                           const OSVR_TimeValue * /*timestamp*/,
                           const OSVR_OrientationReport *report) {

	//Get updated values here
	now_t = clock();
	float diff_t = now_t - last_t;
	// read values every 200ms
	if (diff_t > 200)
	{
		std::cout << diff_t << std::endl;
		last_t = now_t;

		std::ostringstream  strserial;

		
		qtn.setQuaternion((float)osvrQuatGetW(&(report->rotation)), (float)osvrQuatGetX(&(report->rotation)),
			(float)osvrQuatGetY(&(report->rotation)), (float)osvrQuatGetZ(&(report->rotation)));

		/*
		std::cout << osvrQuatGetW(&(report->rotation)) << "w"
			<< osvrQuatGetX(&(report->rotation)) << "x"
			<< osvrQuatGetY(&(report->rotation)) << "y"
			<< osvrQuatGetZ(&(report->rotation)) << "z" << std::endl;
			*/
		//qtn.Normalize();
		EulerAnglesStruct eaStruct = qtn.getEulerAngles();
		
		float norm_yaw = eaStruct.yaw;
		if (eaStruct.yaw < 0)
		{
			norm_yaw = 180 + eaStruct.yaw + 180;

		}
		else
		{
			norm_yaw = eaStruct.yaw;
		}

		if (startangle == 9999)
		{
			//don't move camera, just record this as the point of origin

			startangle = eaStruct.yaw;

		
		}
		else
		{

			//sets up angle diff
			float diff_angle = 0;
			//
			diff_angle = eaStruct.yaw - startangle;

			diff_angle = (int)diff_angle % 360;

			if (diff_angle > 0)
			{
				if (diff_angle > 90)
				{
					diff_angle = 0;
				}
				else
				{
					diff_angle = diff_angle > 180 ? 360 - diff_angle : diff_angle;
				}
			}
			else
			{
				diff_angle = diff_angle < -180 ? +360 + diff_angle : diff_angle;
			}
			//sends serial info to arduino to get parsed to gimbal action
			strserial << ceil(eaStruct.pitch) << "x" << diff_angle << "g";
			
			const std::string tmp = strserial.str();
			const char* cstr = tmp.c_str();//converts C++ string to c style string for sending via serial lib
			if (serial_setup_success)
			{
				ssend.sendStuff(cstr);
			}
			else
				std::cout << "com fail "<<std::endl;

			
			std::cout <<strserial.str() << std::endl;

		}

	}
}


float constrainAngle(float x){
	x = fmod(x + 180, 360);
	if (x < 0)
		x += 360;
	return x - 180;
}


void myPositionCallback(void * /*userdata*/,
                        const OSVR_TimeValue * /*timestamp*/,
                        const OSVR_PositionReport *report) {
    std::cout << "Got POSITION report: Position = (" << report->xyz.data[0]
              << ", " << report->xyz.data[1] << ", " << report->xyz.data[2]
              << ")" << std::endl;
}


/*
to launch program :

<program.exe>  COMPORT

Node Windows XP and above : com port above 9 should be named as e.g \\.\COM10 or \\.\COM11
*/


int main(int argc, char *argv[]) {
    osvr::clientkit::ClientContext context(
        "com.osvr.exampleclients.TrackerCallback");

	now_t = clock();
	last_t = clock();
	

	std::cout << "********COM PORT*********" << argc << std::endl;
	if (argc == 2)
	{
		std::cout << argv[1];
	}

	//gets head tracking
    osvr::clientkit::Interface lefthand =
        context.getInterface("/me/head");


    
    lefthand.registerCallback(&myOrientationCallback, NULL);
	

	
	if (argc == 2)
	{
		serial_setup_success = ssend.setup(argv[1]);
		//set up serial port
	}

	//Update loop, hit ESC to exit
	while (1)
	{
        context.update();

		if (GetAsyncKeyState(VK_ESCAPE))
			break;
		//Sleep(500);
    }


    std::cout << "Library shut down, exiting." << std::endl;
    return 0;
}
