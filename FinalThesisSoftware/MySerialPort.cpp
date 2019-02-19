/******************************************************************************************************
Author: Michael Jacob Mathew

table for serial commands:

order: clockwise, counter clockwise, stop
M1: 1,0,a
M2: 3,2,b
M3: 5,4,c
M4: 7,6,d
M5: 9,8,e

*******************************************************************************************************/
#pragma once
#include"MySerialPort.h"



CSerialPort::CSerialPort(HANDLE comPort)
{
	hSerial = comPort;
}

CSerialPort::~CSerialPort()
{
	//if(hSerial)
	//CloseHandle(hSerial);
}

void CSerialPort::finderror()
{

	char lastError[1024];
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				   NULL,
				   GetLastError(),
				   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				   lastError,
				   1024,
				   NULL);
	return;

}

void CSerialPort::PortInitialize(const char* comPortName)
{
	hSerial = CreateFile(comPortName, //name of the device to be opened; 
						  GENERIC_READ | GENERIC_WRITE, //requested access of the device
						  0, //sharemode off; prevents other devices from accessing it
						  0, //security attribute: 0 means that it cannot be inherited by any of the child processes
						  OPEN_EXISTING, //opens the file only if it exists
						  FILE_ATTRIBUTE_NORMAL,//The file does not have other attributes set
						  0); //template file handle
	if(hSerial==INVALID_HANDLE_VALUE)
	{
		if(GetLastError()==ERROR_FILE_NOT_FOUND)
			{
				//serial port does not exist. Inform user.
				MessageBox(0,"Port does not exist","Error",MB_OK);
			}
	//some other error occurred. Inform user.
		MessageBox(0,"Some kinda error occured","Error",MB_OK );
	}
	else
		MessageBox(0, "Port Successfully Opened","Motor Serial Driver",MB_OK);
		
	DCB dcbSerialParams={0}; //object of type DCB struct
	dcbSerialParams.DCBlength=sizeof(dcbSerialParams); //has to be done in Windows before Getting or setting

	if(!GetCommState(hSerial,&dcbSerialParams))
	{
		//error if unable to Get
		MessageBox(0,"Error Getting the State of Serial",0,MB_OK);
	}
	
	dcbSerialParams.BaudRate=CBR_9600; //setting the baudrate
	dcbSerialParams.ByteSize=8; // the size of bytes
	dcbSerialParams.StopBits=ONESTOPBIT; //number of stop bits
	dcbSerialParams.Parity=NOPARITY; //no parity checking

	if((!SetCommState(hSerial,&dcbSerialParams)))
	{
		//error if unable to Set
		MessageBox(0,"Error Setting the Serial Port",0,MB_OK);
	}

	COMMTIMEOUTS timeouts={0};
	timeouts.ReadIntervalTimeout=50;//how long to wait between receiving characters
	timeouts.ReadTotalTimeoutConstant=50;//how long to wait before returning
	timeouts.ReadTotalTimeoutMultiplier=10;//how much additional time to wait before returing for each byte was requested
	timeouts.WriteTotalTimeoutConstant=50;//how long to wait between writing characters
	timeouts.WriteTotalTimeoutMultiplier=10;//how much additional time to wait before returing for each byte is written
	if(!SetCommTimeouts(hSerial,&timeouts))
	{
		//error if unable to set
		MessageBox(0,"Error Setting the Timeouts",0,MB_OK);
	}
	
	return;

}

void CSerialPort::PortWrite(unsigned char data )
{

	char wrBuff[2]={0};
	wrBuff[0]=data;
	DWORD dwBytesWritten=0;
	if(!WriteFile(hSerial,wrBuff,1,&dwBytesWritten,NULL))
	{
		//unable to write
		MessageBox(0,"Error While Writing Serial Port",0,MB_OK);
	}
	else
		cout<<"\n Written File \n"<<wrBuff[0];
	return;
}

void CSerialPort::PortRead()
{

	char reBuff[31]={0};
	DWORD dwBytesRead=0;
	if(!ReadFile(hSerial,reBuff,30,&dwBytesRead,NULL))
	{
		//unable to write
		MessageBox(0,"Error While Reading Serial Port",0,MB_OK);
	}
	else
		cout<<"\n Read File \n"<<reBuff;
	return;

}

void CSerialPort::CloseSerial()
{

	CloseHandle(hSerial);
	return;
}

BOOL CSerialPort::maestroGetPosition(HANDLE port, unsigned char channel, unsigned short * position)
{
	if(port == NULL) return 1;

	unsigned char command[2];
	unsigned char response[2];
	BOOL success;
	DWORD bytesTransferred;

	// Compose the command.
	command[0] = 0x90;
	command[1] = channel;

	// Send the command to the device.
	success = WriteFile(port, command, sizeof(command), &bytesTransferred, NULL);
	if (!success)
	{
		fprintf(stderr, "Error: Unable to write Get Position command to serial port.  Error code 0x%x.", GetLastError());
		return 0;
	}
	if (sizeof(command) != bytesTransferred)
	{
		fprintf(stderr, "Error: Expected to write %d bytes but only wrote %d.", sizeof(command), bytesTransferred);
		return 0;
	}

	// Read the response from the device.
	success = ReadFile(port, response, sizeof(response), &bytesTransferred, NULL);
	if (!success)
	{
		fprintf(stderr, "Error: Unable to read Get Position response from serial port.  Error code 0x%x.", GetLastError());
		return 0;
	}
	if (sizeof(response) != bytesTransferred)
	{
		fprintf(stderr, "Error: Expected to read %d bytes but only read %d (timeout). ", sizeof(command), bytesTransferred);
		return 0;
	}

	// Convert the bytes received in to a position.
	*position = response[0] + 256*response[1];

	return 1;
}


BOOL CSerialPort::maestroSetTarget(HANDLE port, unsigned char channel, unsigned short target)
{
	if(port == NULL) return 1;
	unsigned char command[4];
	DWORD bytesTransferred;
	BOOL success;

	// Compose the command.
	command[0] = 0x84;
	command[1] = channel;
	command[2] = target & 0x7F;
	command[3] = (target >> 7) & 0x7F;

	//cout<<"\n"<<target<<"\t";printf("%x",command[2]);printf("%x",command[3]);//(target & 0x7F)<<((target >> 7) & 0x7F);
	// Send the command to the device.
	success = WriteFile(port, command, sizeof(command), &bytesTransferred, NULL);
	if (!success)
	{
		fprintf(stderr, "Error: Unable to write Set Target command to serial port.  Error code 0x%x.", GetLastError());
		return 0;
	}
	if (sizeof(command) != bytesTransferred)
	{
		fprintf(stderr, "Error: Expected to write %d bytes but only wrote %d.", sizeof(command), bytesTransferred);
		return 0;
	}

	return 1;
}

void CSerialPort::delay_ms(int n) //  ms delay function.
{
	long int before = GetTickCount();
	long int after = GetTickCount();
	while((after-before)<n) // n ms
	{
	after = GetTickCount();
	}
	before=0; after=0;
}

void CSerialPort::ManipulatorControl(CStereoImage Stereo, bool saveImage) 
{
	int Parent=1,target,channel, counter=0;


	for(Parent;Parent<=1;Parent++)
	{
		int DOF=2;

		for(DOF;DOF<=4;DOF++)
		{
			//delay_ms(1000);
			channel=DOF-1;
			if(DOF==2)
			{
				target=5000;
			}
			else
			{
				target=4000;
			}
			
			while(target<=8000)
			{

				if(!maestroSetTarget(hSerial, channel, target)) {return ;}

				delay_ms(10000);


				if(Stereo.ImgRight != NULL && Stereo.ImgLeft != NULL && saveImage == true)
				{
				++counter;
				string tempC=NameMyRightImage(Parent,DOF,target);
				const char *RightName = tempC.c_str();
				string tempD=NameMyLeftImage(Parent,DOF,target);
				const char *LeftName =tempD.c_str();

				cvSaveImage(RightName,Stereo.imageR_Rectify);
				cvSaveImage(LeftName,Stereo.imageL_Rectify);

				}
				delay_ms(100);
				target+=100;
			}
			maestroSetTarget(hSerial, channel, 6000);
			delay_ms(100);
			if(DOF!=4)
			{
				maestroSetTarget(hSerial, channel+1, 4000);
			}
		}
	}
}

string CSerialPort::NameMyLeftImage(int Parent, int DOF, int target )
{
	string Path="C:\\Users\\SR-MiKE\\Documents\\Visual Studio 2010\\Projects\\Kinect_10_Experiment_with_ARM\\Kinect_SIFT\\Trial";
	string Path2="\\DOF";
	string Path3="\\Left\\";
	long double c = Parent;
	string FileName =to_string(c);
	Path.append(FileName);
	Path.append(Path2);
	c= DOF;
	FileName=to_string(c);
	Path.append(FileName);
	Path.append(Path3);
	c=target;
	FileName=to_string(c);
	Path.append(FileName);
	string str="_Left.jpg";
	Path.append(str);
	return Path;
}

string CSerialPort::NameMyRightImage(int Parent, int DOF, int target )
{
	string Path="C:\\Users\\SR-MiKE\\Documents\\Visual Studio 2010\\Projects\\Kinect_10_Experiment_with_ARM\\Kinect_SIFT\\Trial";
	string Path2="\\DOF";
	string Path3="\\Right\\";
	long double c = Parent;
	string FileName =to_string(c);
	Path.append(FileName);
	Path.append(Path2);
	c= DOF;
	FileName=to_string(c);
	Path.append(FileName);
	Path.append(Path3);
	c=target;
	FileName=to_string(c);
	Path.append(FileName);
	string str="_Right.jpg";
	Path.append(str);
	return Path;
}
