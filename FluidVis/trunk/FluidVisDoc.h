
// FluidVisDoc.h : CFluidVisDoc Ŭ������ �������̽�
//


#pragma once


class CFluidVisDoc : public CDocument
{
protected: // serialization������ ��������ϴ�.
	CFluidVisDoc();
	DECLARE_DYNCREATE(CFluidVisDoc)

// Ư���Դϴ�.
public:

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// �����Դϴ�.
public:
	virtual ~CFluidVisDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	DECLARE_MESSAGE_MAP()
};


