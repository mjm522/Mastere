#pragma once

#ifndef MYSERIALPORT_H_

//#include<windows.h>
#include<conio.h>
#include<iostream>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include "afxdialogex.h"
#include<string>
#include "StereoImage.h"


using namespace std;

class CSerialPort
{

private:

	HANDLE hSerial;

public:

	CSerialPort(HANDLE comPort);
	virtual ~CSerialPort();
	void finderror();
	void PortInitialize(const char* comPortName);
	void PortWrite(unsigned char data );
	void PortRead();
	void CloseSerial();
	void delay_ms(int n);
	BOOL maestroSetTarget(HANDLE port, unsigned char channel, unsigned short target);
	BOOL maestroGetPosition(HANDLE port, unsigned char channel, unsigned short * position);
	void ManipulatorControl(CStereoImage Stereo, bool saveImage = false);
	string NameMyLeftImage(int Parent, int DOF, int target);
	string NameMyRightImage(int Parent, int DOF, int target);
};

#endif