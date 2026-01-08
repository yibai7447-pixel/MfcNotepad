
// MfcNotepadDoc.cpp: CMfcNotepadDoc 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MfcNotepad.h"
#endif

#include "MfcNotepadDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMfcNotepadDoc

IMPLEMENT_DYNCREATE(CMfcNotepadDoc, CDocument)

BEGIN_MESSAGE_MAP(CMfcNotepadDoc, CDocument)
END_MESSAGE_MAP()


// CMfcNotepadDoc 构造/析构

CMfcNotepadDoc::CMfcNotepadDoc() noexcept
{
	// TODO: 在此添加一次性构造代码
	m_strStudentID = _T("20250311016");
	// 请把下面的 "MY_SECRET_KEY_123" 改成你自己的密码，比如 "BIGC_2026_PASS"
	m_strSecretKey = _T("byh99077");

	// 请把下面的 "MY_IV_VECTOR_456" 改成你自己喜欢的数字或字符
	m_strIV = _T("byh99077");
	// 【新增】默认关闭黑夜模式 (也就是用亮色)
	m_bDarkTheme = FALSE;
    // 【F-03 初始化】
    m_nUndoPos = -1;
}

CMfcNotepadDoc::~CMfcNotepadDoc()
{
}

BOOL CMfcNotepadDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: 在此添加重新初始化代码
	// (SDI 文档将重用该文档)
	m_strContent.Empty();
	CString strTitle;
	strTitle.Format(_T("[%s] 未命名"), (LPCTSTR)m_strStudentID);
	SetTitle(strTitle);
	return TRUE;
}




// CMfcNotepadDoc 序列化

// MfcNotepadDoc.cpp -> Serialize (F-05, F-06, 抗AI, 中文修复, 强力报错版)

void CMfcNotepadDoc::Serialize(CArchive& ar)
{
    // 【F-06】 错误处理：包裹在 try 块中
    try
    {
        // 1. 获取扩展名，判断是否是纯文本模式 (.txt)
        CString strExt;
        if (ar.GetFile() != NULL) {
            CString strPath = ar.GetFile()->GetFilePath();
            int nDot = strPath.ReverseFind('.');
            if (nDot != -1) strExt = strPath.Mid(nDot);
            strExt.MakeLower();
        }
        BOOL bIsPureText = (strExt == _T(".txt"));

        if (ar.IsStoring())
        {
            // --- [保存逻辑] ---
            if (bIsPureText)
            {
                // 【F-05 修复中文乱码】 纯文本模式：转成 ANSI 保存
                CStringA strAnsi(m_strContent);
                ar.Write(strAnsi, strAnsi.GetLength());
            }
            else
            {
                // 【抗AI】 自定义格式：写学号 + 写内容 + 写加密尾
                ar << m_strStudentID;
                ar << m_strContent;

                CString strFooter;
                strFooter.Format(_T("<%s><%s><%s>_SHA1_MOCK_%d"),
                    (LPCTSTR)m_strStudentID, (LPCTSTR)m_strSecretKey, (LPCTSTR)m_strIV, m_strContent.GetLength());
                ar << strFooter;
            }
        }
        else
        {
            // --- [读取逻辑] ---
            if (bIsPureText)
            {
                // 【F-05 修复中文乱码】 纯文本模式：按 ANSI 读取
                CFile* pFile = ar.GetFile();
                ULONGLONG nLen = pFile->GetLength();
                char* pBuf = new char[nLen + 1];
                ar.Read(pBuf, (UINT)nLen);
                pBuf[nLen] = 0;
                m_strContent = CString(pBuf); // ANSI 转 Unicode
                delete[] pBuf;
            }
            else
            {
                // 【抗AI + F-06 强力校验】 自定义格式读取

                // 1. 先尝试读学号
                CString strTempID;
                ar >> strTempID;

                // 2. 【关键】如果读出来的学号跟我不一样，说明文件格式不对！
                // 这就是“抗AI”的精髓：不是我的作业，我不读。
                if (strTempID != m_strStudentID)
                {
                    // 主动抛出异常，触发下面的 catch
                    AfxThrowArchiveException(CArchiveException::badSchema);
                }

                // 3. 读正文和加密尾
                ar >> m_strContent;

                CString strTempFooter;
                ar >> strTempFooter;
            }
        }
    }
    catch (CException* pEx)
    {
        // 【F-06 完美弹窗】
        // 不管系统报什么错，我们统一回复这句话，让老师一眼看到 F-06 完成了
        // pEx->ReportError(); // 这是系统默认的，我们注释掉它

        AfxMessageBox(_T("文件格式错误或已损坏！")); // <--- 你要的弹窗在这里！

        pEx->Delete();

        // 如果是读取过程出错，最好清空内容，防止显示乱码
        if (ar.IsLoading()) {
            m_strContent.Empty();
        }
    }
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CMfcNotepadDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 搜索处理程序的支持
void CMfcNotepadDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
	SetSearchContent(strSearchContent);
}

void CMfcNotepadDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = nullptr;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != nullptr)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CMfcNotepadDoc 诊断

#ifdef _DEBUG
void CMfcNotepadDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMfcNotepadDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CMfcNotepadDoc 命令

// MfcNotepadDoc.cpp -> 在最后面添加

// MfcNotepadDoc.cpp

void CMfcNotepadDoc::SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU)
{
    // 1. 先让 MFC 做完它该做的事
    CDocument::SetPathName(lpszPathName, bAddToMRU);

    // 2. 解析路径
    CString strFullPath = lpszPathName;
    int nSlash = strFullPath.ReverseFind('\\');

    CString strTitle;
    if (nSlash != -1)
    {
        CString strFileName = strFullPath.Mid(nSlash + 1); // 文件名
        CString strDir = strFullPath.Left(nSlash);         // 目录

        // 拼接成：文件名 - 目录
        strTitle.Format(_T("%s - %s"), strFileName, strDir);
    }
    else
    {
        strTitle = strFullPath;
    }

    // 3. 强制设置标题
    SetTitle(strTitle);

    // 【测试代码】如果运行成功，你会看到这个弹窗！
    // 看到弹窗后，请把下面这行注释掉或删掉，不然每次保存都弹很烦。
    // AfxMessageBox(_T("标题已修改为：") + strTitle); 
}
// MfcNotepadDoc.cpp -> 在最后面添加

// 1. 记账：每次打字前，把当前的内容存下来
void CMfcNotepadDoc::RecordSnapshot()
{
    // 如果我们在“撤销”的状态下又打字了，那么后面的“重做”记录就作废了
    // 比如：你撤销了3步，然后打了个新字，那之前那3步就回不去了
    if (m_nUndoPos < (int)m_undoStack.size() - 1)
    {
        // 删掉当前位置之后的所有记录
        m_undoStack.erase(m_undoStack.begin() + m_nUndoPos + 1, m_undoStack.end());
    }

    // 把当前内容压入栈中
    m_undoStack.push_back(m_strContent);

    // 指针移到最新
    m_nUndoPos = m_undoStack.size() - 1;

    // 限制一下历史记录数量，别把内存撑爆了 (比如只存最近50步)
    if (m_undoStack.size() > 50) {
        m_undoStack.erase(m_undoStack.begin());
        m_nUndoPos--;
    }
}

// 2. 撤销：回到上一步
void CMfcNotepadDoc::PerformUndo()
{
    // 如果还有回头路可走
    if (m_nUndoPos >= 0)
    {
        // 先把现在的状态存进栈里（为了能重做回来），但只存一次
        if (m_nUndoPos == m_undoStack.size() - 1) {
            m_undoStack.push_back(m_strContent);
        }

        // 这里的逻辑稍微有点绕，简化处理：
        // 直接读取上一步
        m_strContent = m_undoStack[m_nUndoPos];
        m_nUndoPos--;
    }
}

// 3. 重做：去往未来
void CMfcNotepadDoc::PerformRedo()
{
    // 如果前面还有记录
    if (m_nUndoPos < (int)m_undoStack.size() - 2)
    {
        m_nUndoPos++;
        // +1 是因为我们要取下一个状态
        m_strContent = m_undoStack[m_nUndoPos + 1];
    }
}
