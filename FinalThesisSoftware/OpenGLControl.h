#pragma once

#include<Windows.h>

#include<gl\glew.h>
#include<gl\glut.h>
#include<gl\GLU.h>
#include<gl\GL.h> 

#include <iostream> //for various functions
#include <conio.h>
#include <algorithm>// for STL
#include <functional>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <Eigen/Dense>
#include <Eigen/LU>
#include <math.h>

#define WIDTH 640
#define HEIGHT 480
#define PI 3.14159265

using namespace std;
using namespace Eigen;	


class COpenGLControl
{

private:

	double Xrot,Yrot,Zrot; // variables that denote the rotation about each axis
	double L1Angle, L2Angle, L3Angle, L4Angle, L1Angle2, L2Angle2, L3Angle2, L4Angle2;
	double L1AngleMin, L1AngleMax, L2AngleMin, L2AngleMax, L3AngleMin, L3AngleMax, L4AngleMin, L4AngleMax;
	float L1length, L2length, L3length, L4length;
	float alpha1, alpha2, alpha3, alpha4;
	float d1, d2, d3, d4;
	Vector3f Joint0,Joint1, Joint2, Joint3, EndEffector, L1location,L2location,L3location,L4location;
	Matrix3f Rz;

public:

	COpenGLControl();
	virtual ~COpenGLControl();

	virtual void drawManipulator(); 
	virtual void renderCylinder_convenient (Vector3f StartVector, Vector3f EndVector, float radius,int subdivisions);
	virtual void renderCylinder(Vector3f StartVector, Vector3f EndVector, float radius,int subdivisions,GLUquadricObj *quadric);
	virtual void drawAxes(Vector3f Translate, float lengthAxis, double Angle, int link);
	virtual void drawArrow(Vector3f joint, char axis);
	void KeyHook(int x);

	void InitGL();
	void DrawGLScene();

};


