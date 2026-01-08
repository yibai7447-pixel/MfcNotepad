
// MfcNotepadDoc.h: CMfcNotepadDoc 类的接口
//


#pragma once
#include <vector> //我们要用向量容器来存历史
using namespace std;

class CMfcNotepadDoc : public CDocument
{
protected: // 仅从序列化创建
	CMfcNotepadDoc() noexcept;
	DECLARE_DYNCREATE(CMfcNotepadDoc)

// 特性
public:
	// 用来存放记事本的所有文字
	CString m_strContent;
	CString m_strStudentID; // 存学号
	CString m_strSecretKey; // 【新增】加密密钥
	CString m_strIV;        // 【新增】偏移量
	// 【新增】是否开启黑夜/暗色模式 (F-04)
	BOOL m_bDarkTheme;
// 操作
public:
	// MfcNotepadDoc.h -> public 区域

	// --- [F-03 撤销/重做 核心数据] ---
	vector<CString> m_undoStack; // 历史记录仓库
	int m_nUndoPos;              // 当前我们在历史的第几步

	// 记录一次快照 (在修改前调用)
	void RecordSnapshot();

	// 执行撤销
	void PerformUndo();

	// 执行重做
	void PerformRedo();
// 重写
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS
	virtual void SetPathName(LPCTSTR lpszPathName, BOOL bAddToMRU = TRUE);

// 实现
public:
	virtual ~CMfcNotepadDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 用于为搜索处理程序设置搜索内容的 Helper 函数
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
