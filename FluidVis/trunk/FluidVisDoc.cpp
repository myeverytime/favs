
// FluidVisDoc.cpp : CFluidVisDoc Ŭ������ ����
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


// CFluidVisDoc ����/�Ҹ�

CFluidVisDoc::CFluidVisDoc()
{
	// TODO: ���⿡ ��ȸ�� ���� �ڵ带 �߰��մϴ�.

}

CFluidVisDoc::~CFluidVisDoc()
{
}

BOOL CFluidVisDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: ���⿡ ���ʱ�ȭ �ڵ带 �߰��մϴ�.
	// SDI ������ �� ������ �ٽ� ����մϴ�.

	return TRUE;
}




// CFluidVisDoc serialization

void CFluidVisDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: ���⿡ ���� �ڵ带 �߰��մϴ�.
	}
	else
	{
		// TODO: ���⿡ �ε� �ڵ带 �߰��մϴ�.
	}
}


// CFluidVisDoc ����

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


// CFluidVisDoc ���
