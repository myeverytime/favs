
// FluidVisView.h : CFluidVisView 클래스의 인터페이스
//


#pragma once

#include "mainfluidvis.h"
#include "bmp.h"//bmp저장용
#include "MovieMaker.h"//avi저장용

class CFluidVisView : public CView
{
protected: // serialization에서만 만들어집니다.
	CFluidVisView();
	DECLARE_DYNCREATE(CFluidVisView)

// 특성입니다.
public:
	CFluidVisDoc* GetDocument() const;

// 작업입니다.
public:
	void DrawAllScene();
	void BMPSaveCall();

	COpenGLRenderer OpenGLRenderer;

	int bmpsavecnt;//bmp저장용
	bool BMPSaveOK;//bmp저장용
	MovieMaker avisaver;//avi저장용
	bool AVISaveOK;//avi저장용
	int LastX, LastY;


	HDC		m_hDC; 			// GDI Device Context
	HGLRC	m_hglRC;		// Rendering Context



// 재정의입니다.
public:
	virtual void OnDraw(CDC* pDC);  // 이 뷰를 그리기 위해 재정의되었습니다.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 구현입니다.
public:
	virtual ~CFluidVisView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()

public:
	BOOL SetPixelformat(HDC hdc);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	void InitGL(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void ReSizeGLScene(GLsizei width, GLsizei height);
	void DrawGLScene(void);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTerrainCreate();
	afx_msg void OnTerrainTexload();
	afx_msg void OnTerrainContour();
	afx_msg void OnModelLoad();
	afx_msg void OnModelImport();
	afx_msg void OnModelVelocity();
	afx_msg void OnModelDepth();
	afx_msg void OnModelElevation();
	afx_msg void OnModelStreamline();
	afx_msg void OnModelVector();
	afx_msg void OnFilesNew();
	afx_msg void OnFilesSave();
	afx_msg void OnFilesLoad();
	afx_msg void OnObjfilesLoad();
	afx_msg void OnObjfilesUnload();
	afx_msg void OnFilesExit();
	afx_msg void OnCameraSavepoint();
	afx_msg void OnCameraSaveclear();
	afx_msg void OnCameraPlay();
	afx_msg void OnCameraSave();
	afx_msg void OnCameraLoad();
	afx_msg void OnImageStart();
	afx_msg void OnImageEnd();
	afx_msg void OnVideoStart();
	afx_msg void OnVideoEnd();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

#ifndef _DEBUG  // FluidVisView.cpp의 디버그 버전
inline CFluidVisDoc* CFluidVisView::GetDocument() const
   { return reinterpret_cast<CFluidVisDoc*>(m_pDocument); }
#endif

