
// MfcNotepadView.cpp: CMfcNotepadView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MfcNotepad.h"
#endif

#include "MfcNotepadDoc.h"
#include "MfcNotepadView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMfcNotepadView

IMPLEMENT_DYNCREATE(CMfcNotepadView, CScrollView)

BEGIN_MESSAGE_MAP(CMfcNotepadView, CScrollView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CScrollView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CScrollView::OnFilePrintPreview)
	ON_WM_CHAR()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
    ON_COMMAND(ID_VIEW_THEME, &CMfcNotepadView::OnViewTheme)
    ON_COMMAND(ID_EDIT_UNDO, &CMfcNotepadView::OnEditUndo)
    ON_COMMAND(ID_EDIT_REDO, &CMfcNotepadView::OnEditRedo)
END_MESSAGE_MAP()

// CMfcNotepadView 构造/析构

CMfcNotepadView::CMfcNotepadView() noexcept
{
	// TODO: 在此处添加构造代码

}

CMfcNotepadView::~CMfcNotepadView()
{
}

BOOL CMfcNotepadView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CScrollView::PreCreateWindow(cs);
}

// CMfcNotepadView 绘图

// MfcNotepadView.cpp -> OnDraw (修复版)

void CMfcNotepadView::OnDraw(CDC* pDC)
{
    CMfcNotepadDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);
    if (!pDoc) return;

    // --- 1. 准备颜色 ---
    COLORREF clrText, clrBk, clrSideBarBk, clrSideBarText;

    if (pDoc->m_bDarkTheme)
    {
        clrText = RGB(0, 255, 0);       // 亮绿色文字
        clrBk = RGB(30, 30, 30);        // 深灰色背景
        clrSideBarBk = RGB(50, 50, 50);
        clrSideBarText = RGB(150, 150, 150);
    }
    else
    {
        clrText = RGB(0, 0, 0);         // 黑色文字
        clrBk = RGB(255, 255, 255);     // 白色背景
        clrSideBarBk = RGB(240, 240, 240);
        clrSideBarText = RGB(128, 128, 128);
    }

    // --- 2. 填充背景 ---
    CRect rectClient;
    GetClientRect(&rectClient);
    pDC->FillSolidRect(&rectClient, clrBk);

    // --- 3. 设置字体和边距 ---
    CFont font;
    font.CreatePointFont(120, _T("Consolas"));
    CFont* pOldFont = pDC->SelectObject(&font);

    TEXTMETRIC tm;
    pDC->GetTextMetrics(&tm);
    int nLineHeight = tm.tmHeight + tm.tmExternalLeading;
    int nSideBarWidth = 50;
    int nMargin = 5;

    // 画行号栏背景
    CRect rectSideBar = rectClient;
    rectSideBar.right = nSideBarWidth;
    pDC->FillSolidRect(&rectSideBar, clrSideBarBk);

    // --- 4. 逐行绘制 ---
    CString strContent = pDoc->m_strContent;
    int nLen = strContent.GetLength();
    int nStart = 0;
    int nLineNum = 1;
    int nY = rectClient.top;

    while (nStart < nLen || (nStart == nLen && nLen == 0))
    {
        int nEnd = strContent.Find('\n', nStart);
        if (nEnd == -1) nEnd = nLen;

        CString strLine = strContent.Mid(nStart, nEnd - nStart);
        strLine.Remove('\r');

        // 算出这一行的高度
        CRect rectText(nSideBarWidth + nMargin, nY, rectClient.right - nMargin, nY + 10000);
        int nDrawHeight = 0;

        if (strLine.IsEmpty()) {
            nDrawHeight = nLineHeight;
        }
        else {
            nDrawHeight = pDC->DrawText(strLine, &rectText, DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX | DT_CALCRECT);
        }

        // 画行号
        CRect rectNum(0, nY, nSideBarWidth - 5, nY + nDrawHeight);
        pDC->SetTextColor(clrSideBarText);
        pDC->SetBkMode(TRANSPARENT);
        CString strNum;
        strNum.Format(_T("%d"), nLineNum);
        pDC->DrawText(strNum, &rectNum, DT_RIGHT | DT_SINGLELINE | DT_TOP);

        // 画正文
        pDC->SetTextColor(clrText);
        rectText.bottom = nY + nDrawHeight;
        pDC->DrawText(strLine, &rectText, DT_LEFT | DT_WORDBREAK | DT_EXPANDTABS | DT_NOPREFIX);

        nY += nDrawHeight;
        nStart = nEnd + 1;
        nLineNum++;

        if (nStart > nLen) break;
    }

    // --- 5. 光标处理 (修复版) ---
    if (this == GetFocus())
    {
        CPoint ptCaret;

        // 重新找最后一行
        int nLastEnter = strContent.ReverseFind('\n');
        CString strLastLine = (nLastEnter == -1) ? strContent : strContent.Mid(nLastEnter + 1);
        strLastLine.Remove('\r');

        // 算出X坐标
        CSize sizeLast = pDC->GetTabbedTextExtent(strLastLine, 0, NULL);
        ptCaret.x = nSideBarWidth + nMargin + sizeLast.cx;

        // 算出Y坐标 (利用刚才循环结束后的 nY)
        // 如果最后一行刚按了回车，光标在 nY (新的一行)
        // 否则光标在上一行的位置
        if (!strContent.IsEmpty() && strContent.Right(1) == "\n") {
            ptCaret.y = nY; // 新起一行
        }
        else {
            // 这里做一个简单的估算：总高度 - 单行高
            // 这是一个简化的妥协方案，防止计算过于复杂报错
            ptCaret.y = nY - nLineHeight;
            if (ptCaret.y < rectClient.top) ptCaret.y = rectClient.top;
        }

        // 空文件特判
        if (strContent.IsEmpty()) {
            ptCaret.x = nSideBarWidth + nMargin;
            ptCaret.y = rectClient.top;
        }

        SetCaretPos(ptCaret);
        ShowCaret();
    }

    pDC->SelectObject(pOldFont);
}

void CMfcNotepadView::OnInitialUpdate()
{
	CScrollView::OnInitialUpdate();

	CSize sizeTotal;
	// TODO: 计算此视图的合计大小
	sizeTotal.cx = sizeTotal.cy = 100;
	SetScrollSizes(MM_TEXT, sizeTotal);
}


// CMfcNotepadView 打印

BOOL CMfcNotepadView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMfcNotepadView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CMfcNotepadView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}


// CMfcNotepadView 诊断

#ifdef _DEBUG
void CMfcNotepadView::AssertValid() const
{
	CScrollView::AssertValid();
}

void CMfcNotepadView::Dump(CDumpContext& dc) const
{
	CScrollView::Dump(dc);
}

CMfcNotepadDoc* CMfcNotepadView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMfcNotepadDoc)));
	return (CMfcNotepadDoc*)m_pDocument;
}
#endif //_DEBUG


// CMfcNotepadView 消息处理程序

// MfcNotepadView.cpp -> OnChar

void CMfcNotepadView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CMfcNotepadDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc) return;
    // 只有当是有效输入（不是 Ctrl键组合）时才记录
    if ((nChar >= 32) || (nChar == VK_BACK) || (nChar == VK_RETURN))
    {
        pDoc->RecordSnapshot();
    }

	// 1. 处理退格键 (删除最后一个字)
	if (nChar == VK_BACK)
	{
		if (!pDoc->m_strContent.IsEmpty())
		{
			pDoc->m_strContent.Delete(pDoc->m_strContent.GetLength() - 1);
		}
	}
	// 2. 处理回车键 (关键修改！)
	else if (nChar == VK_RETURN)
	{
		// 换行在 Windows 里通常是 \r\n
		pDoc->m_strContent += _T("\r\n");
	}
	// 3. 处理普通字符 (排除控制字符，但允许 TAB)
	else if (nChar >= 32 || nChar == VK_TAB)
	{
		pDoc->m_strContent += (TCHAR)nChar;
	}

	// 4. 刷新屏幕
	Invalidate();
}


void CMfcNotepadView::OnSetFocus(CWnd* pOldWnd)
{
	CScrollView::OnSetFocus(pOldWnd); // 保持基类调用

	// 创建一个实心光标：宽2像素，高20像素
	CreateSolidCaret(2, 20);
	// 让光标显示出来
	ShowCaret();

	// 注意：这里我们暂时还没算光标位置，它默认会出现在左上角(0,0)
	// 下一步我们会解决位置问题
}
// MfcNotepadView.cpp -> OnKillFocus

void CMfcNotepadView::OnKillFocus(CWnd* pNewWnd)
{
	CScrollView::OnKillFocus(pNewWnd);

	// 隐藏并销毁光标，防止切换窗口时残留
	HideCaret();
	DestroyCaret();
}
void CMfcNotepadView::OnViewTheme()
{
    // 1. 拿到文档指针
    CMfcNotepadDoc* pDoc = GetDocument();
    ASSERT_VALID(pDoc);

    // 2. 切换开关状态 (如果是黑就变白，是白就变黑)
    pDoc->m_bDarkTheme = !pDoc->m_bDarkTheme;

    // 3. 告诉系统：界面变了，赶紧重画！
    Invalidate();
}



// 撤销按钮被点击
void CMfcNotepadView::OnEditUndo()
{
    CMfcNotepadDoc* pDoc = GetDocument();
    pDoc->PerformUndo();
    Invalidate(); // 重新画图，显示旧内容
}

// 重做按钮被点击
void CMfcNotepadView::OnEditRedo()
{
    CMfcNotepadDoc* pDoc = GetDocument();
    pDoc->PerformRedo();
    Invalidate(); // 重新画图，显示新内容
}
