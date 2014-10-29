
// FluidVisView.cpp : CFluidVisView 클래스의 구현
//

#include "stdafx.h"
#include "FluidVis.h"

#include "FluidVisDoc.h"
#include "FluidVisView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFluidVisView

IMPLEMENT_DYNCREATE(CFluidVisView, CView)

BEGIN_MESSAGE_MAP(CFluidVisView, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_TERRAIN_CREATE, &CFluidVisView::OnTerrainCreate)
	ON_COMMAND(ID_TERRAIN_TEXLOAD, &CFluidVisView::OnTerrainTexload)
	ON_COMMAND(ID_TERRAIN_CONTOUR, &CFluidVisView::OnTerrainContour)
	ON_COMMAND(ID_MODEL_LOAD, &CFluidVisView::OnModelLoad)
	ON_COMMAND(ID_MODEL_IMPORT, &CFluidVisView::OnModelImport)
	ON_COMMAND(ID_MODEL_VELOCITY, &CFluidVisView::OnModelVelocity)
	ON_COMMAND(ID_MODEL_DEPTH, &CFluidVisView::OnModelDepth)
	ON_COMMAND(ID_MODEL_ELEVATION, &CFluidVisView::OnModelElevation)
	ON_COMMAND(ID_MODEL_STREAMLINE, &CFluidVisView::OnModelStreamline)
	ON_COMMAND(ID_MODEL_VECTOR, &CFluidVisView::OnModelVector)
	ON_COMMAND(ID_FILES_NEW, &CFluidVisView::OnFilesNew)
	ON_COMMAND(ID_FILES_SAVE, &CFluidVisView::OnFilesSave)
	ON_COMMAND(ID_FILES_LOAD, &CFluidVisView::OnFilesLoad)
	ON_COMMAND(ID_OBJFILES_LOAD, &CFluidVisView::OnObjfilesLoad)
	ON_COMMAND(ID_OBJFILES_UNLOAD, &CFluidVisView::OnObjfilesUnload)
	ON_COMMAND(ID_FILES_EXIT, &CFluidVisView::OnFilesExit)
	ON_COMMAND(ID_CAMERA_SAVEPOINT, &CFluidVisView::OnCameraSavepoint)
	ON_COMMAND(ID_CAMERA_SAVECLEAR, &CFluidVisView::OnCameraSaveclear)
	ON_COMMAND(ID_CAMERA_PLAY, &CFluidVisView::OnCameraPlay)
	ON_COMMAND(ID_CAMERA_SAVE, &CFluidVisView::OnCameraSave)
	ON_COMMAND(ID_CAMERA_LOAD, &CFluidVisView::OnCameraLoad)
	ON_COMMAND(ID_IMAGE_START, &CFluidVisView::OnImageStart)
	ON_COMMAND(ID_IMAGE_END, &CFluidVisView::OnImageEnd)
	ON_COMMAND(ID_VIDEO_START, &CFluidVisView::OnVideoStart)
	ON_COMMAND(ID_VIDEO_END, &CFluidVisView::OnVideoEnd)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CFluidVisView 생성/소멸


CFluidVisView::CFluidVisView()
{
}

CFluidVisView::~CFluidVisView()
{
	if(AVISaveOK)
		avisaver.EndCapture();//AVI파일 캡쳐완료
}

void CFluidVisView::BMPSaveCall()
{
	unsigned long lImageSize;   // Size in bytes of image
    unsigned char*pBits = NULL;      // Pointer to bits
    GLint iViewport[4];         // Viewport in pixels
    GLint lastBuffer;          // Storage for the current read buffer setting
    
	// Get the viewport dimensions
	glGetIntegerv(GL_VIEWPORT, iViewport);

    // How big is the image going to be (targas are tightly packed)
	lImageSize = iViewport[2] * 3 * iViewport[3];	

    // Allocate block. If this doesn't work, go home
    pBits = (unsigned char *)malloc(lImageSize);

    // Read bits from color buffer
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ROW_LENGTH, 0);
	glPixelStorei(GL_PACK_SKIP_ROWS, 0);
	glPixelStorei(GL_PACK_SKIP_PIXELS, 0);
    
    glGetIntegerv(GL_READ_BUFFER, &lastBuffer);
    glReadBuffer(GL_FRONT);
    glReadPixels(0, 0, iViewport[2], iViewport[3], GL_RGB, GL_UNSIGNED_BYTE, pBits);
    glReadBuffer(lastBuffer);

	if(bmpsavecnt > 0)
	{
		char bmpfilename[200];
		sprintf(bmpfilename, "bmpsave\\res%d.bmp",bmpsavecnt);
		WriteBMP(bmpfilename, iViewport[2], iViewport[3], pBits);
	}
	bmpsavecnt++;
	BMPSaveOK = true;
}

BOOL CFluidVisView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

// CFluidVisView 그리기

void CFluidVisView::OnDraw(CDC* /*pDC*/)
{
	CFluidVisDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	DrawAllScene();
//	DrawGLScene();
}

void CFluidVisView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFluidVisView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}


// CFluidVisView 진단

#ifdef _DEBUG
void CFluidVisView::AssertValid() const
{
	CView::AssertValid();
}

void CFluidVisView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CFluidVisDoc* CFluidVisView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CFluidVisDoc)));
	return (CFluidVisDoc*)m_pDocument;
}
#endif //_DEBUG


BOOL CFluidVisView::SetPixelformat(HDC hdc)
{
	int pixelformat;

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR), 
		1,                           	// default version
		PFD_DRAW_TO_WINDOW |	// support window
		PFD_SUPPORT_OPENGL |	// support OpenGL
		PFD_GENERIC_FORMAT |	
		PFD_DOUBLEBUFFER, 	// double buffered
		PFD_TYPE_RGBA,		// RGBA type
		32,				// 32-bit color depth
		0, 0, 0, 0, 0, 0, 	    	// color bits ignored
		8,				// no alpha buffer
		0,				// shift bit ignored
		8,				// no accumulation buffer
		0, 0, 0, 0,			// accum bits ignored
		16,				// 16-bit z-buffer
		0,				// no stencil buffer
		0,				// no auxiliary buffer
		PFD_MAIN_PLANE,		// main layer
		0,				// reserved
		0, 0, 0				// layer masks ignored
	};

/*
	PIXELFORMATDESCRIPTOR pfd;

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
*/


	if ( (pixelformat = ChoosePixelFormat(hdc, &pfd))==FALSE)
		return FALSE;


	if ( SetPixelFormat(hdc, pixelformat, &pfd) == FALSE)
		return TRUE;

}


int CFluidVisView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{//남중 오픈지엘 생성
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  여기에 특수화된 작성 코드를 추가합니다.
	m_hDC = GetDC()->m_hDC;
	if( !SetPixelformat(m_hDC))
		return -1; 

	// create rendering context and make it current
	m_hglRC = wglCreateContext(m_hDC);
	wglMakeCurrent(m_hDC, m_hglRC);


	InitGL();

	return 0;
}


void CFluidVisView::OnDestroy()
{//남중 오픈지엘 소멸자
	CView::OnDestroy();

	OpenGLRenderer.Destroy();

	wglMakeCurrent(m_hDC, NULL);
	wglDeleteContext(m_hglRC);
}


void CFluidVisView::InitGL(void)
{//남중 오픈지엘 초기화
	bmpsavecnt = 0;//bmp저장용
	BMPSaveOK = false;//bmp저장용
	AVISaveOK = false;//avi저장용

	OpenGLRenderer.AllInit();//텍스쳐, GPU등 연결설정
}


void CFluidVisView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	OpenGLRenderer.Resize(cx, cy);
//	ReSizeGLScene(cx, cy); 
}

void CFluidVisView::ReSizeGLScene(GLsizei width, GLsizei height)
{//남중 오픈지엘 리사이즈
	if (height == 0 ) 	
		height = 1;

	// reset the viewport to new dimentions
	glViewport( 0, 0, width, height); 
	glMatrixMode( GL_PROJECTION); 
	glLoadIdentity( );
	
	// calculate aspect ratio of the window
	gluPerspective (45.f, (GLfloat)width/height, 0.1f, 100000.f );
	
	//set modelview matrix
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity( );
}

void CFluidVisView::DrawAllScene()
{
	static DWORD LastFPSTime = GetTickCount(), LastFrameTime = LastFPSTime, FPS = 0;
	static DWORD DWFPS = 0;

	DWORD Time = GetTickCount();

	float FrameTime = (Time - LastFrameTime) * 0.001f;

	LastFrameTime = Time;

	if(Time - LastFPSTime > 1000)
	{
		DWFPS = FPS;

		LastFPSTime = Time;
		FPS = 0;
	}
	else
	{
		FPS++;
	}

	BYTE Keys = 0x00;

	if(GetKeyState('W') & 0x80) Keys |= 0x01;
	if(GetKeyState('S') & 0x80) Keys |= 0x02;
	if(GetKeyState('A') & 0x80) Keys |= 0x04;
	if(GetKeyState('D') & 0x80) Keys |= 0x08;
	if(GetKeyState('Q') & 0x80) Keys |= 0x10;
	if(GetKeyState('E') & 0x80) Keys |= 0x20;

	if(GetKeyState(VK_SHIFT) & 0x80) Keys |= 0x40;
	if(GetKeyState(VK_CONTROL) & 0x80) Keys |= 0x80;

	if(Keys & 0x3F)
	{
		OpenGLRenderer.Camera.Move(OpenGLRenderer.Camera.OnKeys(Keys, FrameTime));
	}

	OpenGLRenderer.Render(FrameTime);//물,환경,모델등을 그리는 함수호출

	char temp[100];
	sprintf(temp,"FrameTime %.2f, FPS: %d, ElapsedTime: %.2f", FrameTime, DWFPS, OpenGLRenderer.AnalElapsedTime);
	OpenGLRenderer.DisplayElapsedTime( temp, WIN_WIDTH, WIN_HEIGHT );//화면에 fps출력하기

	if(BMPSaveOK==true)
	{
		SetCurrentDirectory(OpenGLRenderer.WinInitDir);
		BMPSaveCall();//시뮬레이션을 BMP파일로 저장
	}

	if(AVISaveOK)
		avisaver.Snap();//시뮬레이션을 AVI파일로 저장

	SwapBuffers( m_hDC );
	InvalidateRect(NULL, FALSE);
}

void CFluidVisView::DrawGLScene(void)
{//남중 오픈지엘 그리기
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
	glLoadIdentity( ); 

	// camera view configuration 
	gluLookAt( 0.f, 0.f, 3.f,  0.f, 0.f, 0.f,  0.f, 1.f, 0.f ); 

	OpenGLRenderer.DrawEnvironmentBox();

	glColor3f( 1.f, 0.f, 0.f ); 
	glBegin(GL_TRIANGLES); 
		glVertex3f( 0.5f, 0.f, 0.f ); 
		glVertex3f( 0.f, 0.5f, 0.f ); 
		glVertex3f( -0.5f, 0.f, 0.f ); 
	glEnd(); 

	// swap buffer
	SwapBuffers( m_hDC );

	InvalidateRect(NULL, FALSE);
}

void CFluidVisView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	int i;
	switch(nChar)
	{
	case VK_ESCAPE:
		exit(1);
		break;
	case VK_RETURN:
		OpenGLRenderer.Camera.CameraAnimPause = !OpenGLRenderer.Camera.CameraAnimPause;
		break;
	case VK_SPACE://Material Parameter변경
		if(OpenGLRenderer.AlreadyAllPlayed == true)
			OpenGLRenderer.ImportAnalData();
		else
			OpenGLRenderer.AnalAnimPause = !OpenGLRenderer.AnalAnimPause;
		break;
	case VK_F1://GRID로 된 WATER를 WIREFRAME으로 표현
			OpenGLRenderer.WireFrame = !OpenGLRenderer.WireFrame;
			break;
	case VK_F2://해석데이터를 초반부터 다시재생
		if(OpenGLRenderer.AnalysisDataImported == true)
		{
			OpenGLRenderer.AnalElapsedTime = 0.0;
			OpenGLRenderer.AnalAnimCnt = 0;
		}
			break;
	case VK_F3://0:VIEW MODE, 1:OBJECT MODE
		OpenGLRenderer.ViewObjMode = !OpenGLRenderer.ViewObjMode;
			break;
	case VK_F4://안티에일리어싱 적용 여부
		OpenGLRenderer.m_Antialias = !OpenGLRenderer.m_Antialias;
			break;
	case VK_F7://카메라 경로 저장 시작
		OpenGLRenderer.Camera.GetSavingViewMatrix();
		OpenGLRenderer.Camera.CameraPathNum++;
		break;
	case VK_F8://카메라 경로 시뮬레이션
		OpenGLRenderer.Camera.CurCameraPath = 0;
		OpenGLRenderer.Camera.AnimatingParam = 0.0;
		OpenGLRenderer.Camera.CameraPathAnimationOn = true;
			break;
	case VK_F9://카메라 경로 초기화
		OpenGLRenderer.Camera.CurCameraPath = 0;
		OpenGLRenderer.Camera.CameraPathNum = 0;
			break;
	case VK_ADD:
		SetCurrentDirectory(OpenGLRenderer.WinInitDir);
		avisaver.PrepareForCapture();
		avisaver.StartCapture("avisave\\capture.avi", 800, 600, 24);
		AVISaveOK = true;
		break;
	case VK_SUBTRACT:
		AVISaveOK = false;
		avisaver.EndCapture();//AVI파일 캡쳐완료
		break;

				case VK_LEFT:
		OpenGLRenderer.Camera.RotateAboutYAxis(-2.0);
			break;
				case VK_RIGHT:
		OpenGLRenderer.Camera.RotateAboutYAxis(2.0);
			break;
				case VK_UP:
		OpenGLRenderer.Camera.RotateAboutXAxis(-2.0);
			break;
				case VK_DOWN:
		OpenGLRenderer.Camera.RotateAboutXAxis(2.0);
			break;

	case VK_NUMPAD1://XZ면으로 카메라 회전
		OpenGLRenderer.Camera.RotateToXZPlane();
			break;
	case VK_NUMPAD3://YZ면으로 카메라 회전
		OpenGLRenderer.Camera.RotateToYZPlane();
			break;
	case VK_NUMPAD7://XY면으로 카메라 회전
		OpenGLRenderer.Camera.RotateToXYPlane();
			break;
	case VK_NUMPAD5:
		OpenGLRenderer.Camera.SetPersOrtho();
			break;
	case VK_NUMPAD2:
		OpenGLRenderer.Camera.RotateAboutLeftRight(-MINDELTA);
			break;
	case VK_NUMPAD4:
		OpenGLRenderer.Camera.RotateAboutUpDown(MINDELTA);
			break;
	case VK_NUMPAD6:
		OpenGLRenderer.Camera.RotateAboutUpDown(-MINDELTA);
			break;
	case VK_NUMPAD8:
		OpenGLRenderer.Camera.RotateAboutLeftRight(MINDELTA);
			break;
	}

	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CFluidVisView::OnLButtonDown(UINT nFlags, CPoint point) 
{
	LastX = point.x;
	LastY = point.y;

	CView::OnLButtonDown(nFlags, point);
}

void CFluidVisView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	LastX = point.x;
	LastY = point.y;

	CView::OnMButtonDown(nFlags, point);
}

void CFluidVisView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	LastX = point.x;
	LastY = point.y;

	CView::OnRButtonDown(nFlags, point);
}

BOOL CFluidVisView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	OpenGLRenderer.Camera.OnMouseWheel(zDelta);

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}

void CFluidVisView::OnMouseMove(UINT nFlags, CPoint point) 
{
	int rx, ry;
	if(GetKeyState(VK_RBUTTON) & 0x80)
	{
		if(OpenGLRenderer.ViewObjMode == true)//객체모드이면
		{//회전 수행
			rx = LastX - point.x;
			ry = LastY - point.y;

			OpenGLRenderer.yRot -= rx/2.0;
			OpenGLRenderer.xRot -= ry/2.0;

			LastX = point.x;
			LastY = point.y;
		}
	}
	else if(GetKeyState(VK_LBUTTON) & 0x80)
	{

		if(OpenGLRenderer.ViewObjMode == true)//객체모드이면
		{//확대축소 수행
			if(LastX > point.x || LastY < point.y)
				OpenGLRenderer.ScaleFactor *= 1.05;
			else
				OpenGLRenderer.ScaleFactor /= 1.05;

			LastX = point.x;
			LastY = point.y;
		}
	}
	else if(GetKeyState(VK_MBUTTON) & 0x80)
	{//카메라 이동 수행
		if(GetKeyState(VK_SHIFT) & 0x80)
		{//Shift키+마우스중간버튼 드래그하면 카메라회전
			OpenGLRenderer.Camera.OnMouseMove(LastX - point.x, LastY - point.y);

		LastX = point.x;
		LastY = point.y;
		}
		else
		{
		OpenGLRenderer.Camera.Move(OpenGLRenderer.Camera.MoveByMouseMButton(LastX - point.x, LastY - point.y));
		LastX = point.x;
		LastY = point.y;
		}
	}


	CView::OnMouseMove(nFlags, point);
}

void CFluidVisView::OnTerrainCreate()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.TerrainHeightMapFileOpen();
}

void CFluidVisView::OnTerrainTexload()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.TextureFileLoad();
}

void CFluidVisView::OnTerrainContour()
{
	OpenGLRenderer.TerrainColorCodingOn = !OpenGLRenderer.TerrainColorCodingOn;
}

void CFluidVisView::OnModelLoad()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.ModelFileOpen();
}

void CFluidVisView::OnModelImport()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.WaterLevelFileOpen();
	OpenGLRenderer.WaterSpeedFileOpen();

	OpenGLRenderer.ImportAnalData();
}

void CFluidVisView::OnModelVelocity()
{
	if(OpenGLRenderer.AnalDataColorCodingOn == ANAL_VELOCITYSTYLE)
		OpenGLRenderer.AnalDataColorCodingOn = 0;
	else
		OpenGLRenderer.AnalDataColorCodingOn = ANAL_VELOCITYSTYLE;
}

void CFluidVisView::OnModelDepth()
{
	if(OpenGLRenderer.AnalDataColorCodingOn == ANAL_DEPTH)
		OpenGLRenderer.AnalDataColorCodingOn = 0;
	else
		OpenGLRenderer.AnalDataColorCodingOn = ANAL_DEPTH;
}

void CFluidVisView::OnModelElevation()
{
	if(OpenGLRenderer.AnalDataColorCodingOn == ANAL_ELEVATION)
		OpenGLRenderer.AnalDataColorCodingOn = 0;
	else
		OpenGLRenderer.AnalDataColorCodingOn = ANAL_ELEVATION;
}

void CFluidVisView::OnModelStreamline()
{
	if(OpenGLRenderer.AnalDataVectorsOn == ANAL_STREAMLINE)
		OpenGLRenderer.AnalDataVectorsOn = 0;
	else
		OpenGLRenderer.AnalDataVectorsOn = ANAL_STREAMLINE;
}

void CFluidVisView::OnModelVector()
{
	if(OpenGLRenderer.AnalDataVectorsOn == ANAL_VECTOR)
		OpenGLRenderer.AnalDataVectorsOn = 0;
	else
		OpenGLRenderer.AnalDataVectorsOn = ANAL_VECTOR;
}

void CFluidVisView::OnFilesNew()
{
}

void CFluidVisView::OnFilesSave()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.ProjectFileSave();
}

void CFluidVisView::OnFilesLoad()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.ProjectFileOpen();
}

void CFluidVisView::OnObjfilesLoad()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.OBJFileLoad();
}

void CFluidVisView::OnObjfilesUnload()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.OBJFileUnLoad();
}

void CFluidVisView::OnFilesExit()
{
	exit(0);
}

void CFluidVisView::OnCameraSavepoint()
{
	OpenGLRenderer.Camera.GetSavingViewMatrix();
	OpenGLRenderer.Camera.CameraPathNum++;
}

void CFluidVisView::OnCameraSaveclear()
{
	OpenGLRenderer.Camera.CurCameraPath = 0;
	OpenGLRenderer.Camera.CameraPathNum = 0;
}

void CFluidVisView::OnCameraPlay()
{
	OpenGLRenderer.Camera.CurCameraPath = 0;
	OpenGLRenderer.Camera.AnimatingParam = 0.0;
	OpenGLRenderer.Camera.CameraPathAnimationOn = true;
}

void CFluidVisView::OnCameraSave()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.CameraFileSave();
}

void CFluidVisView::OnCameraLoad()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	OpenGLRenderer.CameraFileOpen();
}

void CFluidVisView::OnImageStart()
{
	bmpsavecnt = 0;
	BMPSaveOK = true;
}

void CFluidVisView::OnImageEnd()
{
	BMPSaveOK = false;
}

void CFluidVisView::OnVideoStart()
{
	SetCurrentDirectory(OpenGLRenderer.WinInitDir);
	avisaver.PrepareForCapture();
	avisaver.StartCapture("avisave\\capture.avi", 1920, 1280, 24);
	AVISaveOK = true;
}

void CFluidVisView::OnVideoEnd()
{
	AVISaveOK = false;
	avisaver.EndCapture();//AVI파일 캡쳐완료
}
