
// FluidVisView.h : CFluidVisView Ŭ������ �������̽�
//


#pragma once

#include "mainfluidvis.h"
#include "bmp.h"//bmp�����
#include "MovieMaker.h"//avi�����

class CFluidVisView : public CView
{
protected: // serialization������ ��������ϴ�.
	CFluidVisView();
	DECLARE_DYNCREATE(CFluidVisView)

// Ư���Դϴ�.
public:
	CFluidVisDoc* GetDocument() const;

// �۾��Դϴ�.
public:
	void DrawAllScene();
	void BMPSaveCall();

	COpenGLRenderer OpenGLRenderer;

	int bmpsavecnt;//bmp�����
	bool BMPSaveOK;//bmp�����
	MovieMaker avisaver;//avi�����
	bool AVISaveOK;//avi�����
	int LastX, LastY;


	HDC		m_hDC; 			// GDI Device Context
	HGLRC	m_hglRC;		// Rendering Context



// �������Դϴ�.
public:
	virtual void OnDraw(CDC* pDC);  // �� �並 �׸��� ���� �����ǵǾ����ϴ�.
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// �����Դϴ�.
public:
	virtual ~CFluidVisView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
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

#ifndef _DEBUG  // FluidVisView.cpp�� ����� ����
inline CFluidVisDoc* CFluidVisView::GetDocument() const
   { return reinterpret_cast<CFluidVisDoc*>(m_pDocument); }
#endif

