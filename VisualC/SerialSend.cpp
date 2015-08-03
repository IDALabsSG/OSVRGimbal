#include "SerialSend.h"
#include <iostream>

//Prepared by :Alvin Ng
bool SerialSend::setup(LPCSTR commport)
{


	hComm = CreateFile(commport,  // Specify port device: default "COM1"
		GENERIC_READ | GENERIC_WRITE,       // Specify mode that open device.
		0,                                  // the devide isn't shared.
		NULL,                               // the object gets a default security.
		OPEN_EXISTING,                      // Specify which action to take on file.
		0,                                  // default.
		NULL);                              // default.





	if (!GetCommState(hComm, &dcb))
	{
		return FALSE;
	}


	dcb.BaudRate = CBR_57600;
	dcb.ByteSize = 8;
	dcb.Parity = NOPARITY;
	dcb.StopBits = ONESTOPBIT;

	if (!SetCommState(hComm, &dcb))
		return FALSE;


	return TRUE;
}


/*
Contract :

before sending stuff, ensure that setup returns with a non-zero value
*/
bool SerialSend::sendStuff(CString cszData)
{




	for (int i = 0; i < cszData.GetLength(); i++)
	{
		int retVal = WriteFile(hComm, cszData.Mid(i, 1), 1, &dwBytesWritten, NULL);
		//	WriteFile(hComm, cszData, cszData.GetLength(), &dwBytesWritten, NULL);
		std::cout << "bytes writte" << dwBytesWritten << std::endl;
	}

	return FALSE;
}

SerialSend::~SerialSend()
{
	if (hComm)
	{
		CloseHandle(hComm);
	}
}
