#pragma once

#include<Windows.h>

#include <cv.h>
#include <highgui.h>

#include "triclops.h"
#include "pgrflycapture.h"
#include "pgrflycapturestereo.h"
#include "pnmutils.h"

#include<assert.h>

#include <stdio.h>
#include <time.h>

using namespace std;
class CStereoImage 
{
public:
	CStereoImage(void);
	~CStereoImage(void);

	bool GetData();

	void Initial(int W, int H);
	bool Close();
	bool Open();

	void convertFlyCaptureImagesToIplImages(void *, void *, void *); 
	TriclopsContext   triclops;
	TriclopsImage     disparityImage;
	TriclopsImage     refImage;
	TriclopsInput     triclopsInput;
	
	FlyCaptureContext	   flycapture;
	FlyCaptureImage	   flycaptureImage;
	FlyCaptureInfoEx	   pInfo;
	FlyCapturePixelFormat   pixelFormat;
	TriclopsImage16     depthImage16;
	
	TriclopsInput       colorDataL; ///left packed data
	TriclopsInput       colorDataR; // right packed data
	TriclopsColorImage  colorImage;

	int imageCols;
	int imageRows;
	int imageRowInc;
	int iSideBySideImages;
	unsigned long timeStampSeconds;
	unsigned long timeStampMicroSeconds;

	int Width;
	int Height;
	
	IplImage * ImgLeft;
	IplImage * ImgRight;

	IplImage * imageL_Rectify;
	IplImage * imageR_Rectify;

	int pixelinc;

	TriclopsError     te;
	FlyCaptureError   fe;
	
	int iMaxCols;
	int iMaxRows;

	char* szCalFile;

	unsigned char* rowIntMono;	
	unsigned char* rowIntColor;
	
	FlyCaptureImage tempImage;
	FlyCaptureImage tempColorImage;
	
    float baseline;//!< Camera baseline
    float focalLength;//!< Camera focal length
	float centerCol, centerRow;//!< Camera center coordinates

	// Pointers to positions in the mono buffer that correspond to the beginning
	// of the red, green and blue sections
	unsigned char* redMono;
	unsigned char* greenMono;
	unsigned char* blueMono;
	
	unsigned char* redColor;
	unsigned char* greenColor;
	unsigned char* blueColor;

};

