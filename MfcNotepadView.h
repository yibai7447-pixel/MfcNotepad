
// MfcNotepadView.h: CMfcNotepadView 类的接口
//

#pragma once


class CMfcNotepadView : public CScrollView
{
protected: // 仅从序列化创建
	CMfcNotepadView() noexcept;
	DECLARE_DYNCREATE(CMfcNotepadView)

// 特性
public:
	CMfcNotepadDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual void OnDraw(CDC* pDC);  // 重写以绘制该视图
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 构造后第一次调用
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// 实现
public:
	virtual ~CMfcNotepadView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnViewTheme();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnEditUndo();
	afx_msg void OnEditRedo();
};

#ifndef _DEBUG  // MfcNotepadView.cpp 中的调试版本
inline CMfcNotepadDoc* CMfcNotepadView::GetDocument() const
   { return reinterpret_cast<CMfcNotepadDoc*>(m_pDocument); }
#endif

