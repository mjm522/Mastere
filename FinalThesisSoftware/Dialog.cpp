//-----------------------------------------------------------------------------------------
#include <afxwin.h>      //MFC core and standard components
#include "resource.h"    //main symbols
#include "StereoImage.h"
#include "OpenGLControl.h"
#include "OpenGLDevice.h"
#include "MySerialPort.h"
//-----------------------------------------------------------------------------------------

//Globals
CDC* pDC1;
CDC* pDC2;
CDC* pDC3;

CRect rect1;
CRect rect2;
HANDLE hSerial;

CStereoImage Stereo;
COpenGLControl openGLControl;
OpenGLDevice openGLDevice;
CSerialPort SerialPort(hSerial);

class Interface : public CDialog
{
    public:
    Interface(CWnd* pParent = NULL): CDialog(Interface::IDD, pParent)
    {    }

    // Dialog Data, name of dialog form
    enum{IDD = IDD_INTERFACE1};

    protected:
		virtual void DoDataExchange(CDataExchange* pDX) {
			CDialog::DoDataExchange(pDX); 	//  DDX_CBString(pDX, IDC_COMPORT, m_MyPort);
			DDX_Control(pDX, IDC_COMPORT, m_MyPort);
			DDX_CBString(pDX, IDC_COMPORT, m_ComPort);
			//  DDX_Control(pDX, IDC_MANIPULATORCNTRL, m_ManipCntrl);
			DDX_Check(pDX, IDC_MANIPULATORCNTRL, m_ManipCntrl);
		}

    //Called right after constructor. Initialize things here.
    virtual BOOL OnInitDialog() 
    { 
        CDialog::OnInitDialog();

		SelectComPort(); // selection of the comport in the window

		Stereo.Initial(640,480); // initialize the bumble bee with resolution 640 by 480	
		Stereo.Open();
		
		pDC1 = GetDlgItem(IDC_LEFT)->GetDC();
		GetDlgItem(IDC_LEFT)->GetClientRect(&rect1); //Copies the dimensions of the bounding rectangle of the CWnd object to the structure pointed to by lpRect

		pDC2 = GetDlgItem(IDC_RIGHT)->GetDC();
		GetDlgItem(IDC_RIGHT)->GetClientRect(&rect2);

		pDC3 = GetDlgItem(IDC_OPENGL)->GetDC(); 

		openGLDevice.create(pDC3->m_hDC);
		openGLControl.InitGL();

		// variables
		m_ManipCntrl = false;
		saveImage= false;
		thresh = false;

        return true; 
    }
public:
	void DisplayIplImg(IplImage* pImgIpl, CDC* pDC, CRect rect)
	{
		 BITMAPINFO bitmapInfo;
		 bitmapInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		 bitmapInfo.bmiHeader.biPlanes=1;
		 bitmapInfo.bmiHeader.biCompression=BI_RGB;
		 bitmapInfo.bmiHeader.biXPelsPerMeter=100;
		 bitmapInfo.bmiHeader.biYPelsPerMeter=100;
		 bitmapInfo.bmiHeader.biClrUsed=0;
		 bitmapInfo.bmiHeader.biClrImportant=0;
		 bitmapInfo.bmiHeader.biSizeImage=0;
		 bitmapInfo.bmiHeader.biWidth=pImgIpl->width;
		 bitmapInfo.bmiHeader.biHeight=-pImgIpl->height;
		 IplImage* tempImage;
		 if(pImgIpl->nChannels == 3)
		 {
		  tempImage = (IplImage*)cvClone(pImgIpl);
		  bitmapInfo.bmiHeader.biBitCount=tempImage->depth * tempImage->nChannels;
		 }
		 else if(pImgIpl->nChannels == 1)
		 {
		  tempImage =  cvCreateImage(cvGetSize(pImgIpl), IPL_DEPTH_8U, 3);
		  cvCvtColor(pImgIpl, tempImage, CV_GRAY2BGR);
		  bitmapInfo.bmiHeader.biBitCount=tempImage->depth * tempImage->nChannels;
		 }
		 pDC->SetStretchBltMode(COLORONCOLOR);
		 ::StretchDIBits(pDC->GetSafeHdc(), rect.left, rect.top, rect.right, rect.bottom,
		  0, 0, tempImage->width, tempImage->height, tempImage->imageData, &bitmapInfo,
		  DIB_RGB_COLORS, SRCCOPY);
		 cvReleaseImage(&tempImage);
	}

afx_msg	void OnPaint()
	{
		Stereo.GetData();
		DisplayIplImg(Stereo.imageL_Rectify, pDC1,rect1);
		DisplayIplImg(Stereo.imageR_Rectify, pDC2,rect2);
		
		openGLControl.DrawGLScene();
		SwapBuffers(pDC3->m_hDC);

		SetTimer(1,30,NULL); //This function creates a timer with the specified time-out value. 1= timer identifier, 30 = timeout in millisecods Null = long pointer to the function
		CDialog::OnPaint();
	}
afx_msg void OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1)
	{

	if(!Stereo.GetData()) KillTimer(1);
	else 
		{
			DisplayIplImg(Stereo.imageL_Rectify, pDC1,rect1);
			DisplayIplImg(Stereo.imageR_Rectify, pDC2,rect2);
			openGLControl.DrawGLScene();
			SwapBuffers(pDC3->m_hDC);
		}
	}

	CDialog::OnTimer(nIDEvent);
}

virtual BOOL PreTranslateMessage(MSG* pMsg)
{
	int x = (int) pMsg->wParam;
	openGLControl.KeyHook(x);
	return CDialog::PreTranslateMessage(pMsg);
}

DECLARE_MESSAGE_MAP()

afx_msg void OnBnClickedCancel()
{

	Stereo.Close();
	CDialog::OnCancel();

}
afx_msg void OnClose()
{
	Stereo.Close();
	CDialog::OnClose();
}

CComboBox m_MyPort;
CString m_ComPort;

bool thresh;
bool saveImage;

void SelectComPort() //added fucntion to find the present serial ports of the system; make sure you added virtual funtion SelectComPort() in Motor_Serial_DriverDlg.h file
{
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE); 
	TCHAR lpTargetPath[5000]; // buffer to store the path of the COMPORTS
	DWORD test;
	bool gotPort=0; // in case the port is not found
	
	for(int i=0; i<255; i++) // checking ports from COM0 to COM255
	{
		CString str;
		str.Format(_T("%d"),i);
		CString ComName=CString("COM") + CString(str); // converting to COM0, COM1, COM2
		
		test = QueryDosDevice(ComName, (LPSTR)lpTargetPath, 5000);

			// Test the return value and error if any
		if(test!=0) //QueryDosDevice returns zero if it didn't find an object
		{
			m_MyPort.AddString((CString)ComName); // add to the ComboBox
			gotPort=1; // found port
		}

		if(::GetLastError()==ERROR_INSUFFICIENT_BUFFER)
		{
			lpTargetPath[10000]; // in case the buffer got filled, increase size of the buffer.
			continue;
		}

	}

	if(!gotPort) // if not port
	m_MyPort.AddString((CString)"No Active Ports Found"); // to display error message incase no ports found

}


afx_msg void OnBnClickedConnect();
afx_msg void OnBnClickedDisconnect();
//CButton m_ManipCntrl;
afx_msg void OnBnClickedManipulatorcntrl();
afx_msg void OnBnClickedRadio1();
BOOL m_ManipCntrl;
afx_msg void OnBnClickedSaveimage();
};




//-----------------------------------------------------------------------------------------

class Application : public CWinApp
{
public:
Application() {  }

public:
virtual BOOL InitInstance()
{
   CWinApp::InitInstance();
   Interface dlg;
   m_pMainWnd = &dlg;
   INT_PTR nResponse = dlg.DoModal();
   
   return FALSE;

} //close function

};

//-----------------------------------------------------------------------------------------
//Need a Message Map Macro for both CDialog and CWinApp

BEGIN_MESSAGE_MAP(Interface, CDialog)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDCANCEL, &Interface::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_CONNECT, &Interface::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &Interface::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_MANIPULATORCNTRL, &Interface::OnBnClickedManipulatorcntrl)
	ON_BN_CLICKED(IDC_SAVEImage, &Interface::OnBnClickedSaveimage)
END_MESSAGE_MAP()

//-----------------------------------------------------------------------------------------

Application theApp;  //Starts the Application


void Interface::OnBnClickedConnect()
{
	UpdateData(TRUE); // to take data from GUI
	CString comPortName;
	if(m_ComPort != "No Active Ports Found" || m_ComPort != "")
	{
		comPortName = CString(CString("\\\\.\\") + CString(m_ComPort));
		GetDlgItem(IDC_COMPORT)->EnableWindow(FALSE); // to disable the com port select box
		GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE); // to disable connect button
		GetDlgItem(IDC_DISCONNECT)->EnableWindow(TRUE); // to enable disconnect button
		SerialPort.PortInitialize(comPortName);
	}
}


void Interface::OnBnClickedDisconnect()
{
	GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE); // to disable connect button
	m_ComPort="";
	UpdateData(FALSE); // to update the GUI
	GetDlgItem(IDC_COMPORT)->EnableWindow(TRUE); // to disable the com port select box
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE); // to enable disconnect button
	SerialPort.CloseSerial();
}


void Interface::OnBnClickedManipulatorcntrl()
{
	UpdateData(TRUE);
	//if(m_ManipCntrl)
	//SerialPort.ManipulatorControl(Stereo, saveImage);
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

				if(!SerialPort.maestroSetTarget(hSerial, channel, target)) {return ;}

				//SerialPort.delay_ms(10000);


				if(Stereo.ImgRight != NULL && Stereo.ImgLeft != NULL && saveImage == true)
				{
				++counter;
				//string tempC=NameMyRightImage(Parent,DOF,target);
				//const char *RightName = tempC.c_str();
				//string tempD=NameMyLeftImage(Parent,DOF,target);
				//const char *LeftName =tempD.c_str();

				//cvSaveImage(RightName,Stereo.imageR_Rectify);
				//cvSaveImage(LeftName,Stereo.imageL_Rectify);
				OnPaint();
				}
				SerialPort.delay_ms(100);
				target+=100;
			}
			SerialPort.maestroSetTarget(hSerial, channel, 6000);
			SerialPort.delay_ms(100);
			if(DOF!=4)
			{
				SerialPort.maestroSetTarget(hSerial, channel+1, 4000);
			}
		}
	}
}




void Interface::OnBnClickedSaveimage()
{
	saveImage= true;
}
