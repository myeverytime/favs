
// FluidVisDoc.cpp : CFluidVisDoc 클래스의 구현
//

#include "stdafx.h"
#include "FluidVis.h"

#include "FluidVisDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFluidVisDoc

IMPLEMENT_DYNCREATE(CFluidVisDoc, CDocument)

BEGIN_MESSAGE_MAP(CFluidVisDoc, CDocument)
END_MESSAGE_MAP()


// CFluidVisDoc 생성/소멸

CFluidVisDoc::CFluidVisDoc()
{
	// TODO: 여기에 일회성 생성 코드를 추가합니다.

}

CFluidVisDoc::~CFluidVisDoc()
{
}

BOOL CFluidVisDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 여기에 재초기화 코드를 추가합니다.
	// SDI 문서는 이 문서를 다시 사용합니다.

	return TRUE;
}




// CFluidVisDoc serialization

void CFluidVisDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: 여기에 저장 코드를 추가합니다.
	}
	else
	{
		// TODO: 여기에 로딩 코드를 추가합니다.
	}
}


// CFluidVisDoc 진단

#ifdef _DEBUG
void CFluidVisDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CFluidVisDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CFluidVisDoc 명령
