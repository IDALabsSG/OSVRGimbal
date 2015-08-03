#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <atlstr.h>



/*

Serial Port code for windows,
this class opens and sends characters to a specified comm port with fixed
9600, 1 stop bit , no parity

returns 0x100 if there is an issue
otherwise it will be 0x00

Prepared by :Alvin Ng

*/

class SerialSend
{
public:
	bool setup(LPCSTR);
	bool sendStuff(CString);
	~SerialSend();

private:
	DCB dcb;
	HANDLE hComm;
	BYTE Byte;
	DWORD dwBytesXferred, dwCommModemStatus, dwBytesWritten;


};
