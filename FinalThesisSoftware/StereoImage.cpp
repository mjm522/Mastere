#pragma once

#include "StereoImage.h"

CStereoImage::CStereoImage(void)
{
}


CStereoImage::~CStereoImage(void)
{
}

void CStereoImage::Initial(int W, int H)
{		
	Width = W;
	Height = H;

	ImgLeft = cvCreateImage( cvSize(Width, Height), IPL_DEPTH_8U, 3 ); /*BGRU*/
	ImgRight = cvCreateImage( cvSize(Width, Height), IPL_DEPTH_8U, 3 );
	
	// Create output images
	imageL_Rectify = cvCreateImage( cvSize(Width,Height ), IPL_DEPTH_8U, 3);
	imageR_Rectify = cvCreateImage( cvSize(Width,Height ), IPL_DEPTH_8U, 3 );

	iMaxCols = 0;
	iMaxRows = 0;
	rowIntMono=0;
	rowIntColor=0;
	
	// Pointers to positions in the mono buffer that correspond to the beginning
	// of the red, green and blue sections
	redMono = NULL;
	greenMono = NULL;
	blueMono = NULL;
	
	redColor = NULL;
	greenColor = NULL;
	blueColor = NULL; 
}

bool CStereoImage::Open()
{
	// Open the camera
	if( flycaptureCreateContext( &flycapture ) != FLYCAPTURE_OK ) 
		return false;	
	
	// Initialize the Flycapture context
	if( flycaptureInitialize( flycapture, 0 ) != FLYCAPTURE_OK ) 
		return false;	
	
	// Save the camera's calibration file, and return the path 
	if( flycaptureGetCalibrationFileFromCamera( flycapture, &szCalFile ) != FLYCAPTURE_OK ) 
		return false;
	
	// Create a Triclops context from the cameras calibration file
	if( triclopsGetDefaultContextFromFile( &triclops, szCalFile ) != TriclopsErrorOk ) 
		return false;	
	
	// Get camera information
	if( flycaptureGetCameraInfo( flycapture, &pInfo ) != FLYCAPTURE_OK ) 
		return false;
	
	if (pInfo.CameraType == FLYCAPTURE_COLOR)
	{
		pixelFormat = FLYCAPTURE_RAW16;
	} 
	else 
	{
		pixelFormat = FLYCAPTURE_MONO16;
	}
	
	
	if(pInfo.CameraModel==FLYCAPTURE_BUMBLEBEE2)
	{
		unsigned long ulValue;
		flycaptureGetCameraRegister( flycapture, 0x1F28, &ulValue );
			
		if ( ( ulValue & 0x2 ) == 0 )
		{
			// Hi-res BB2
			iMaxCols = 1024; 
			iMaxRows = 768;
		}
		else
		{
			// Low-res BB2
			iMaxCols = 640;
			iMaxRows = 480;
		}
	}
	
	// Start transferring images from the camera to the computer
	if( flycaptureStartCustomImage(flycapture, 3, 0, 0, iMaxCols, iMaxRows, 100, pixelFormat) != FLYCAPTURE_OK ) 
		return false;	
	
	// Set up some stereo parameters:
	if( triclopsSetResolution( triclops, Height, Width ) != TriclopsErrorOk ) 
		return false;

	if( triclopsGetBaseline( triclops, &baseline ) !=TriclopsErrorOk ) // to get the baseline from the calibration file its in meters
		return false;
	if( triclopsGetFocalLength( triclops, &focalLength ) !=TriclopsErrorOk ) // to get the focal length its in pixels
		return false;
	if( triclopsGetImageCenter( triclops, &centerRow, &centerCol ) !=TriclopsErrorOk ) // to get the centre of the images its in pixels
		return false;
	
	// Set disparity range
	if( triclopsSetDisparity( triclops, 0, 100 ) != TriclopsErrorOk ) 
		return false;	

	// Lets turn off all validation except subpixel and surface
	// This works quite well
	if( triclopsSetTextureValidation( triclops, 0 ) != TriclopsErrorOk ) 
		return false;	
	
	if( triclopsSetUniquenessValidation( triclops, 0 ) != TriclopsErrorOk ) 
		return false;
	
	if( triclopsSetSubpixelInterpolation( triclops, 1 ) != TriclopsErrorOk ) 
		return false;
		
	return true;
}

bool CStereoImage::GetData()
{
if( flycaptureGrabImage2( flycapture, &flycaptureImage ) != FLYCAPTURE_OK)
		return false;
	
	// Extract information from the FlycaptureImage
	imageCols = flycaptureImage.iCols;
	imageRows = flycaptureImage.iRows;
	imageRowInc = flycaptureImage.iRowInc;
	iSideBySideImages = flycaptureImage.iNumImages;
	timeStampSeconds = flycaptureImage.timeStamp.ulSeconds;
	timeStampMicroSeconds = flycaptureImage.timeStamp.ulMicroSeconds;
	
	// Create buffers for holding the mono images
	if( rowIntMono == 0)
	{
		rowIntMono = new unsigned char[ imageCols * imageRows * iSideBySideImages ];
		// Create a temporary FlyCaptureImage for preparing the stereo image
		tempImage.pData = rowIntMono;		
	}
	
	if(rowIntColor == 0)
	{
		rowIntColor = new unsigned char[ imageCols * imageRows * iSideBySideImages * 4 * 2]; //ROWS x COLS x NUM_IMGS x 4 COLORS x 2 BYTES
		// Create a temporary FlyCaptureImage for preparing the stereo image
		tempColorImage.pData = rowIntColor;
	}
	
	// Convert the pixel interleaved raw data to row interleaved format
	if(flycapturePrepareStereoImage( flycapture, flycaptureImage, &tempImage, &tempColorImage  ) != FLYCAPTURE_OK)
		return false;
	
	redColor = rowIntColor; // pointer to the right image
	greenColor = redColor + (4*imageCols ); // pointer to the left image
	blueColor = redColor + (4*imageCols );
	
	redMono = rowIntMono;
	greenMono = redMono + imageCols;
	blueMono = redMono + imageCols; 

	// Use the row interleaved images to build up a packed TriclopsInput.
	// A packed triclops input will contain a single image with 32 bpp.
	if ( triclopsBuildPackedTriclopsInput( imageCols, imageRows, imageRowInc * 4, timeStampSeconds, timeStampMicroSeconds, redColor, &colorDataR ) != TriclopsErrorOk) // right Image
		return false;
	
	if ( triclopsBuildPackedTriclopsInput( imageCols, imageRows, imageRowInc * 4, timeStampSeconds, timeStampMicroSeconds, greenColor, &colorDataL ) != TriclopsErrorOk) //left Image
		return false;
	
	// Use the row interleaved images to build up an RGB TriclopsInput.  
	// An RGB triclops input will contain the 3 raw images (1 from each camera).
	if( triclopsBuildRGBTriclopsInput(imageCols, imageRows, imageRowInc,  timeStampSeconds, timeStampMicroSeconds, redMono, greenMono, blueMono, &triclopsInput) != TriclopsErrorOk )
		return false;	

	// Rectify the images/ surely need
	if( triclopsRectify( triclops, &triclopsInput ) != TriclopsErrorOk)
		return false;
	
	if ( triclopsGetImage16( triclops, TriImg16_DISPARITY, TriCam_REFERENCE, &depthImage16 ) != TriclopsErrorOk )
		return false;
	
	// Do stereo processing
	if( triclopsStereo( triclops ) != TriclopsErrorOk )
		return false; 
	
	IplImage *tmpImageL = cvCreateImage( cvSize(Width, Height), IPL_DEPTH_8U, 4 ); // temporary buffer to store the four channel image
	IplImage *tmpImageR = cvCreateImage( cvSize(Width, Height), IPL_DEPTH_8U, 4 ); // left and right image
	TriclopsPackedColorImage	colorImageL, colorImageR;
	
	if ( pixelFormat == FLYCAPTURE_RAW16 )
	{
		if( triclopsRectifyColorImage( triclops, TriCam_REFERENCE, &colorDataR, &colorImage ) != TriclopsErrorOk) // color image is rectified here
			return false;
		
		triclopsRectifyPackedColorImage( triclops, TriCam_LEFT,  &colorDataL, &colorImageL ); // the image is getting rectified here
		triclopsRectifyPackedColorImage( triclops, TriCam_RIGHT, &colorDataR, &colorImageR );	
	}

	if( tmpImageL->imageData!=NULL && tmpImageR->imageData != NULL)
	{
		memcpy( tmpImageL->imageData, colorImageL.data, colorImageL.nrows*colorImageL.rowinc ); // copy the image data to IplImage
		tmpImageL->widthStep = colorImageL.rowinc;
		
		memcpy( tmpImageR->imageData, colorImageR.data, colorImageR.nrows*colorImageR.rowinc );
		tmpImageR->widthStep = colorImageR.rowinc;
		
//		memcpy( ImgReference->imageData, colorImage.blue, colorImageR.nrows*colorImageR.rowinc );
//		ImgReference->widthStep = colorImageR.rowinc;
		
		cvCvtColor( tmpImageL, imageL_Rectify, CV_BGRA2BGR );	// Left image 3 plain image
		imageL_Rectify->origin = tmpImageL->origin;

		cvCvtColor( tmpImageR, imageR_Rectify, CV_BGRA2BGR );	// Right image 3 plain image
		imageR_Rectify->origin = tmpImageR->origin; 
	}

	// Release temp images
	cvReleaseImage( &tmpImageL );
	cvReleaseImage( &tmpImageR );

	if( flycaptureImage.bStippled ) // this means that bumble bee does not have on board processing and sent it as bayer tiled raw images
	{
		FlyCaptureImage colorImage;
		colorImage.pData              = new unsigned char[flycaptureImage.iRows*flycaptureImage.iCols*flycaptureImage.iNumImages*3*2];      // ROWS x COLS x NUM_IMGS x 3 COLORS x 2 BYTES
		colorImage.pixelFormat        = FLYCAPTURE_BGR;                                                                         // Format BGR

		fe = flycaptureConvertImage( flycapture, &flycaptureImage, &colorImage );
                        
		// ColorImage contains the two raw images in format BGR
		if( fe != FLYCAPTURE_OK )
		{
				delete [] colorImage.pData;
				cout << "Error when converting the grabbed image to BGR(U)." << endl;
		}

		IplImage* tmpL = cvCreateImage( cvSize(Width, Height), IPL_DEPTH_8U, 4 );
		IplImage* tmpR = cvCreateImage( cvSize(Width, Height), IPL_DEPTH_8U, 4 );
                      
		convertFlyCaptureImagesToIplImages( &colorImage, tmpL, tmpR );

		// Free allocated memory
		delete [] colorImage.pData;

		// Convert images to BGR (3 channels) and set origins
		cvCvtColor( tmpL, ImgLeft, CV_BGRA2BGR );  // Left raw; image un rectified
		ImgLeft->origin = tmpL->origin;

		cvCvtColor( tmpR, ImgRight, CV_BGRA2BGR );  // Right raw; unrectified
		ImgRight->origin = tmpR->origin;

		cvReleaseImage( &tmpL );
		cvReleaseImage( &tmpR );

	} 		
	return true;
	// Determine the number of pixels spacing per row			
}

void CStereoImage::convertFlyCaptureImagesToIplImages(void * flycapImage, void * dstL,void * dstR)
{
	  FlyCaptureImage *fcImg  = static_cast<FlyCaptureImage*>(flycapImage);
      IplImage *leftImg  = static_cast<IplImage*>(dstL);
      IplImage *rightImg = static_cast<IplImage*>(dstR);

      IplImage *tmpL, *tmpR;
      bool mustResize = false;

      if( leftImg->width != fcImg->iCols/2 || leftImg->height != fcImg->iRows )
            mustResize = true;

      if( mustResize )
      {
            tmpL = cvCreateImage( cvSize(fcImg->iCols/2,fcImg->iRows), IPL_DEPTH_8U, 4 );
            tmpR = cvCreateImage( cvSize(fcImg->iCols/2,fcImg->iRows), IPL_DEPTH_8U, 4 );
      }

      assert( fcImg->iRowInc == 4*fcImg->iCols );          // 2 images of type BRGU
      assert( leftImg->width <= fcImg->iCols/2 && rightImg->width <= fcImg->iCols/2 && leftImg->height <= fcImg->iRows && rightImg->height <= fcImg->iRows );
      assert( leftImg->width == rightImg->width && leftImg->height == rightImg->height );
      assert( leftImg->depth == IPL_DEPTH_8U && rightImg->depth == IPL_DEPTH_8U );

      unsigned char *p1Src, *p2Src;
      char *p1Dst, *p2Dst;
      p1Src = &(fcImg->pData[0]);                                                                     // Source
      p1Dst = mustResize ? &(tmpL->imageData[0]) : &(leftImg->imageData[0]);  // Destination
      p2Src = p1Src + 2*fcImg->iCols;                                                                 // Source
      p2Dst = mustResize ? &(tmpR->imageData[0]) : &(rightImg->imageData[0]); // Destination
      for( int r = 0; r < fcImg->iRows; r++ )
      {
            memcpy( p1Dst, p1Src, 2*fcImg->iCols );
            memcpy( p2Dst, p2Src, 2*fcImg->iCols );
            p1Src += fcImg->iRowInc;
            p2Src += fcImg->iRowInc;
            p1Dst += 2*fcImg->iCols;
            p2Dst += 2*fcImg->iCols;
      }

      if( mustResize )
      {
            // Resize the output images
            cvResize( tmpL, dstL );
            cvResize( tmpR, dstR );

            cvReleaseImage( &tmpL );
            cvReleaseImage( &tmpR );
      }
}

bool CStereoImage::Close()
{
	if(flycapture != 0 )
	{
		// Close the camera
		if( flycaptureStop( flycapture ) != FLYCAPTURE_OK )
			return false;
		
		if( flycaptureDestroyContext( flycapture ) != FLYCAPTURE_OK )
			return false;
		flycapture=0;
	}
	
	if(rowIntMono != 0)
	{
		// Delete the image buffer
		delete rowIntMono;
		redMono = NULL;
		greenMono = NULL;
		blueMono = NULL;
		rowIntMono=0;
	}
	
	if(rowIntColor != 0)
	{
		delete rowIntColor;
		redColor = NULL;
		greenColor = NULL;
		blueColor = NULL;
		rowIntColor = 0;
	}		
	
	if(triclops != 0 )
	{		
		// Destroy the Triclops context
		if( triclopsDestroyContext( triclops ) != FLYCAPTURE_OK )
			return false;
		triclops=0;
	}
	
	if(ImgRight != 0)
	{
		cvReleaseImage(&ImgRight);
		ImgRight=0;
	}

	if(ImgLeft != 0)
	{
		cvReleaseImage(&ImgLeft);
		ImgLeft=0;
	}
	
	return true;
}


