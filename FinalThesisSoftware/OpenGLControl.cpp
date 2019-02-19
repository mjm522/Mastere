#pragma once

#include "OpenGLControl.h"

// COpenGLControl

COpenGLControl::COpenGLControl()
{
	Xrot=0.0f; // variables that denote the rotation about each axis
	Yrot=0.0f;
	Zrot=0.0f;
	L1Angle = 0; L2Angle = 0; L3Angle = 0; L4Angle = 0; L1Angle2 = 360; L2Angle2 = 360; L3Angle2 = 360; L4Angle2 = 360;
	L1AngleMin = 360; L1AngleMax = 630; L2AngleMin = 360; L2AngleMax = 420; L3AngleMin = 360; L3AngleMax = 520; L4AngleMin = 360; L4AngleMax = 580;
	L1length = 0.5; L2length = 0.5; L3length = 0.5; L4length = 0.5;
	alpha1 = -90; alpha2 = 0; alpha3 = 0; alpha4 = 0;
	d1 = 0; d2 = 0; d3 = 0.0; d4 = 0;
}

COpenGLControl::~COpenGLControl()
{}


/////////////////////////////////////////////////////////////////////////////
// Handlers for messages COpenGLControl

void COpenGLControl::DrawGLScene()
{
	
	glClear(GL_COLOR_BUFFER_BIT |  GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	//***************************
	// DRAWING CODE
	//***************************
	
	drawManipulator(); 
	glFlush(); // ensures that all the commands are flushed
}

void COpenGLControl::InitGL()
{

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);												
	glDepthFunc(GL_LEQUAL);	
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	Joint0<< 0,0,0;
	Joint1<<L1length,0,0+d1;		//distance between x0 and x1 along z0
	Joint2<<L2length,0,0+d2;		//distance between x1 and x2 along z1
	Joint3<<L3length,0,0+d3;		//distance between x2 and x3 along z2
	EndEffector<<L4length, 0 , 0+d4; //distance between x3 and x4 along z3

	L1location<<0,0,0;			
	L2location = L1location + Joint1;
	L3location = L2location + Joint2;
	L4location = L3location + Joint3;

	// setting up the window

	glMatrixMode(GL_PROJECTION);						
	glLoadIdentity();						

	glViewport(0, 0, WIDTH, HEIGHT);

	gluPerspective(45.0f,1.0f*WIDTH/HEIGHT,1.0f,500.0f);

	glMatrixMode(GL_MODELVIEW);						
	glLoadIdentity();

}

void COpenGLControl::drawArrow(Vector3f joint, char axis)
{
	GLdouble base = 0.02; GLdouble top = 0; GLdouble height = 0.05; GLint slices = 10; GLint stacks = 5;

	glPushMatrix();
		glTranslatef(joint[0], joint[1], joint[2]);
		switch(axis)
		{
		case 'x': glRotatef(90,0,1,0); break; // for x axis arrow turn wrt to Y
		case 'y': glRotatef(270,1,0,0); break; // for y axis arrow turn wrt to X
		case 'z': break; // for z axis arrow turn wrt to Z
		}
		GLUquadricObj *quadObj = gluNewQuadric();
		gluCylinder(quadObj, base, top, height, slices, stacks);
	glPopMatrix();

	gluDeleteQuadric(quadObj);
}

void COpenGLControl::drawAxes(Vector3f Translate, float lengthAxis, double Angle, int link)
{
	Vector3f X,Y,Z;
	X<<lengthAxis,0,0;
	Y<<0,lengthAxis,0;
	Z<<0,0,lengthAxis;
	
	glPushMatrix();
		glTranslatef(Translate[0],Translate[1],Translate[2]);
		Angle=fmod(Angle,360);
		if(link==1)
		{
		glRotatef(-Angle,0,1,0);
		}
		else if(link==2)
		{
		glRotatef(Angle,0,0,1);
		}

		glBegin(GL_LINES); // X axis	
			glColor3f(1.0f,0.0f,0.0f);
			glVertex3f(0, 0,0);
			glVertex3f(X[0], X[1], X[2]);
		glEnd();
		drawArrow(X,'x');

		glBegin(GL_LINES); //Y axis	
			glColor3f(0.0f,1.0f,0.0f);
			glVertex3f(0, 0, 0);
			glVertex3f(Y[0], Y[1], Y[2]);
		glEnd();
		drawArrow(Y,'y');

		glBegin(GL_LINES); //Z axis
			glColor3f(0.0f,0.0f,1.0f);
			glVertex3f(0, 0, 0);
			glVertex3f(Z[0], Z[1], Z[2]);
		glEnd();
		drawArrow(Z,'z');
	glPopMatrix();
}

/*Drawing cylinder help: ht_tp://lifeofaprogrammergeek.blogspot.in/2008/07/rendering-cylinder-between-two-points.html */

void COpenGLControl::renderCylinder(Vector3f StartVector, Vector3f EndVector, float radius,int subdivisions,GLUquadricObj *quadric)
{
	float x1=StartVector[0], y1=StartVector[1], z1=StartVector[2], x2=EndVector[0], y2=EndVector[1], z2=EndVector[2];
	float vx = x2-x1;
	float vy = y2-y1;
	float vz = z2-z1;

	//handle the degenerate case of z1 == z2 with an approximation
	if(vz == 0)
		vz = .0001;

	float v = sqrt( vx*vx + vy*vy + vz*vz );
	float ax = 57.2957795*acos( vz/v );
	if ( vz < 0.0 )
		ax = -ax;
	float rx = -vy*vz;
	float ry = vx*vz;
	glPushMatrix();

	//draw the cylinder body
		glTranslatef( x1,y1,z1 );
		glRotatef(ax, rx, ry, 0.0);
		gluQuadricOrientation(quadric,GLU_OUTSIDE);
		gluCylinder(quadric, radius, radius, v, subdivisions, 1);

		//draw the first cap
		gluQuadricOrientation(quadric,GLU_INSIDE);
		gluDisk( quadric, 0.0, radius, subdivisions, 1);
		glTranslatef( 0,0,v );

		//draw the second cap
		gluQuadricOrientation(quadric,GLU_OUTSIDE);
		gluDisk( quadric, 0.0, radius, subdivisions, 1);
	glPopMatrix();

}


void COpenGLControl::renderCylinder_convenient (Vector3f StartVector, Vector3f EndVector, float radius,int subdivisions)
{
//the same quadric can be re-used for drawing many cylinders
	GLUquadricObj *quadric=gluNewQuadric();
	gluQuadricNormals(quadric, GLU_SMOOTH);
	renderCylinder(StartVector, EndVector,radius,subdivisions,quadric);
	gluDeleteQuadric(quadric);
}


void COpenGLControl::drawManipulator() 
{
	GLfloat width=1000;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);//Switch to the model prespective
	glLoadIdentity();//reset the drawing perspective

	gluLookAt(
		4.0f, 4.0f, 4.0f, //where the camera is placed
		0.0f, 0.0f, 0.0f, // where the camera is pointing
		0.0f, 1.0f, 0.0f); // normalized vector 

	Vector3f origin; origin<<0,0,0;

	glPushMatrix();
		glRotatef(alpha1,1,0,0); //alpha1= -90 // Angle from z0 to z1 about x1
		drawAxes(origin,1,0,0);
	glPopMatrix();

	glPushMatrix();
		glColor3f(1,1,0);
		renderCylinder_convenient(origin, Joint1, 0.06 , 10);
	glPopMatrix();
	
	glPushMatrix(); //draw Axis for link 1
		glRotatef(alpha2,1,0,0); // Angle from z1 to z2 about x1
		drawAxes(Joint1,0.2,L1Angle2,1);
	glPopMatrix();

	glPushMatrix();												// link 2
		glTranslatef(L2location(0),L2location(1),L2location(2));
		glColor3f(0.0f,1.0f,1.0f);
		renderCylinder_convenient(origin, Joint2, 0.06 , 10);
	glPopMatrix();

	glPushMatrix(); //draw Axis for link 2
		glRotatef(alpha3,1,0,0); // Angle from z2 to z3 about x2
		drawAxes(L2location+Joint2,0.2,L2Angle2,2);
	glPopMatrix();

	glPushMatrix();												// link 3
		glTranslatef(L3location(0),L3location(1),L3location(2));
		glColor3f(1.0f,0.0f,1.0f);
		renderCylinder_convenient(origin, Joint3, 0.06 , 10);
	glPopMatrix();

	glPushMatrix(); //draw Axis for link 3
		glRotatef(alpha4,1,0,0); // Angle from z3 to z4 about x3
		drawAxes(L3location+Joint3,0.2,L3Angle2,2);
	glPopMatrix();

	glPushMatrix();												// link 4
		glTranslatef(L4location(0),L4location(1),L4location(2));
		glColor3f(1.0f,1.0f,1.0f);
		renderCylinder_convenient(origin, EndEffector, 0.06 , 10);
	glPopMatrix();

}

void COpenGLControl::KeyHook(int x)
{
	switch(x)
	{
	case 'a' :
		if( L4Angle2 <= L4AngleMax )
		{
			L4Angle2 += 3;
			L4Angle = 3* PI/180;
		}else
			L4Angle = 0;
		Rz<<cos(L4Angle),-sin(L4Angle),0,sin(L4Angle),cos(L4Angle),0,0,0,1;
		EndEffector = Rz * EndEffector;
		break;

	case 'q' :
		if( L4Angle2 >= L4AngleMin )
		{
			cout<<L4Angle2;
			L4Angle2 -= 3;
			L4Angle = -3* PI/180;
		}else
			L4Angle = 0;
		Rz<<cos(L4Angle),-sin(L4Angle),0,sin(L4Angle),cos(L4Angle),0,0,0,1;
		EndEffector = Rz * EndEffector;
		break;

	case 's' :
		if( L3Angle2 <= L3AngleMax )
		{
			L3Angle2 += 3;
			L3Angle = 3.0f * PI/180;
		}else
			L3Angle=0;
		Rz<<cos(L3Angle),-sin(L3Angle),0,sin(L3Angle),cos(L3Angle),0,0,0,1;
		Joint3 = Rz * Joint3;
		L4location = L3location + Joint3; // L4location is the location of the origin of link 4

		break;

	case 'w' :
		if( L3Angle2 >= L3AngleMin )
		{
			L3Angle2 -= 3;
			L3Angle = -3.0f * PI/180;
		}else
			L3Angle=0;
		Rz<<cos(L3Angle),-sin(L3Angle),0,sin(L3Angle),cos(L3Angle),0,0,0,1;
		Joint3 = Rz * Joint3;
		L4location = L3location + Joint3; // L4location is the location of the origin of link 4

		break;

	case 'd':
		if( L2Angle2 <= L2AngleMax )
		{
			L2Angle2 += 3.0f;
			L2Angle = 3.0f*PI/180;
		}else
			L2Angle=0;
		Rz<<cos(L2Angle),-sin(L2Angle),0,sin(L2Angle),cos(L2Angle),0,0,0,1;
		Joint2 = Rz * Joint2;
		L3location = L2location + Joint2; // L3location is the location of the origin of link 3 
		L4location = L3location + Joint3; 

		break;

	case 'e':
		if( L2Angle2 >= L2AngleMin )
		{
			L2Angle2 -= 3.0f;
			L2Angle = -3.0f*PI/180;
		}else
			L2Angle=0;
		Rz<<cos(L2Angle),-sin(L2Angle),0,sin(L2Angle),cos(L2Angle),0,0,0,1;
		Joint2 = Rz * Joint2;
		L3location = L2location + Joint2; // L3location is the location of the origin of link 3 
		L4location = L3location + Joint3; 

		break;

	case 'f':
		if( L1Angle2 <= L1AngleMax )
		{
			L1Angle2 += 3;
			L1Angle = 3.0f* PI/180;
		}else
			L1Angle=0;
		Rz<<cos(L1Angle),0,-sin(L1Angle),0,1,0,sin(L1Angle),0,cos(L1Angle);
		Joint1 = Rz * Joint1;

		L2location = L1location + Joint1; // L2 location is the location of the origin of link 2
		L3location = L2location + Joint2;
		L4location = L3location + Joint3;

		break;

	case 'r':
		if( L1Angle2 >= L1AngleMin )
		{
			L1Angle2 -= 3;
			L1Angle = -3.0f* PI/180;
		}else
			L1Angle=0;
		Rz<<cos(L1Angle),0,-sin(L1Angle),0,1,0,sin(L1Angle),0,cos(L1Angle);
		Joint1 = Rz * Joint1;

		L2location = L1location + Joint1; // L2 location is the location of the origin of link 2
		L3location = L2location + Joint2;
		L4location = L3location + Joint3;

		break;
		
	}

}


