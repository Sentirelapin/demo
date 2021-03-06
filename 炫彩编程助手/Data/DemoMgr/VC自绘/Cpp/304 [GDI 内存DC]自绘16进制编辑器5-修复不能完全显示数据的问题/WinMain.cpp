#define  USE_GDIPACK
#define USE_GDIPLUS
#include "Common.h"
#include "DrawPack.h"

class XFile
{
public:
	BOOL Open(const wchar_t* lpFileName)
	{
		m_hFile = CreateFileW(
			lpFileName,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
			return FALSE;

		return TRUE;
	}
	VOID Close()
	{
		CloseHandle(m_hFile);
	}
	BOOL SetPoint(int nPos)
	{
		return SetFilePointer(m_hFile,nPos,NULL,FILE_BEGIN) == INVALID_SET_FILE_POINTER ? FALSE:FALSE;
	}

	BOOL Read(VOID* pBuffer,DWORD dwBufferSize,DWORD* pdwRealReadSize)
	{
		return ReadFile(m_hFile,pBuffer,dwBufferSize,pdwRealReadSize,NULL);
	}

	BOOL GetSize(PLARGE_INTEGER fileSize)
	{
		return GetFileSizeEx(m_hFile,fileSize);
	}

private:
	HANDLE m_hFile;
};


class XBuffer
{
public:
	XBuffer()
	{
		m_pBuffer = NULL;
	}
	~XBuffer()
	{
		if (m_pBuffer)
			delete [] m_pBuffer;
	}

	void ReSize(int size)
	{
		if (m_pBuffer)
			delete [] m_pBuffer;

		m_pBuffer = new char[size];
		m_MaxShowCount = size;
	}
public:
	char* m_pBuffer;
	int m_MaxShowCount;
};





class DataFormat
{
public:
    DataFormat()
    {
        ZeroMemory(pDicTable, 256);
        pDicTable['0'] = 0;
        pDicTable['1'] = 1;
        pDicTable['2'] = 2;
        pDicTable['3'] = 3;
        pDicTable['4'] = 4;
        pDicTable['5'] = 5;
        pDicTable['6'] = 6;
        pDicTable['7'] = 7;
        pDicTable['8'] = 8;
        pDicTable['9'] = 9;
        pDicTable['A'] = 10;
        pDicTable['B'] = 11;
        pDicTable['C'] = 12;
        pDicTable['D'] = 13;
        pDicTable['E'] = 14;
        pDicTable['F'] = 15;
    }
    wchar_t HexStrToChar(const wchar_t* pBuf)
    {
        wchar_t result = 0;
        result |= ((pDicTable[pBuf[0]] << 4) & 0xF0);
        result |= pDicTable[pBuf[1]];
        return result;
    }

    wchar_t* CharToHexStr(wchar_t ch)
    {
        wchar_t pHexToDecTable[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
        buffer[0] = pHexToDecTable[(ch & 0xf000) >> 12];
        buffer[1] = pHexToDecTable[(ch & 0xf00) >> 8];
        buffer[2] = pHexToDecTable[(ch & 0xf0) >> 4];
        buffer[3] = pHexToDecTable[(ch & 0xf)];
        buffer[4] = 0;
        return buffer;
    }
private:
    wchar_t pDicTable[256];
	
    wchar_t buffer[5];
};


#define HEX_TEXT_HEIGHT 14

class XHexEdit
{
public:
	XHexEdit()
	{
		m_pTopLeftText = L"offset";
		m_pTopMidText  = L"0123456789ABCDEF";

		m_nMidTextBlockWidth = 20;
		m_nMidTextBlockSpace = 6;

		m_nRowSpace = 6;
		m_nRowHeight = HEX_TEXT_HEIGHT;

		m_nScorBarVPos = 0;
	}
	HELE Create(int x,int y,int cx,int cy,HXCGUI hParent)
	{
		m_x = x, m_y = y,m_cx = cx, m_cy = cy;
        m_hScrollView=XSView_Create(m_x,m_y,m_cx,m_cy,hParent);
		XEle_RegEventCPP(m_hScrollView,XE_SCROLLVIEW_SCROLL_H,&XHexEdit::OnScrollViewScrollH);
		XEle_RegEventCPP(m_hScrollView,XE_SCROLLVIEW_SCROLL_V,&XHexEdit::OnScrollViewScrollV);
		XEle_RegEventCPP(m_hScrollView,XE_SIZE,&XHexEdit::OnSize);
		XEle_RegEventCPP(m_hScrollView,XE_PAINT_SCROLLVIEW,&XHexEdit::OnDrawScrollView);
		m_MemDC.Create(XEle_GetHWND(m_hScrollView));


		

		XEle_PostEvent(m_hScrollView,m_hScrollView,XE_SIZE,NULL,NULL);
//		m_MemDC.Resize(m_cx,m_cy);

		return m_hScrollView;
	}

	BOOL LoadFile(const wchar_t* lpFileName)
	{
		if (!m_File.Open(lpFileName))
			return FALSE;

		XEle_PostEvent(m_hScrollView,m_hScrollView,XE_SIZE,NULL,NULL);

		m_nScorBarVPos = 0;
		return TRUE;
	}


//事件接口
public:
	int OnSize(BOOL *pbHandled)
	{

		m_RowCountIndexTextWidth = m_MemDC.GetTextWidth(L"00000000",8);


		LARGE_INTEGER fileSize;
		m_File.GetSize(&fileSize);
		int nRowCount = fileSize.QuadPart / 16  +20;
		int nRowHeight = nRowCount*m_nRowHeight;
		XSView_SetTotalSize(m_hScrollView,500,nRowHeight);
		XSView_SetLineSize(m_hScrollView,m_nRowHeight,m_nRowHeight);


		m_cx = XEle_GetWidth(m_hScrollView);
		m_cy = XEle_GetHeight(m_hScrollView);
		m_MemDC.Resize(500,m_cy);

		m_Buffer.ReSize(CalcHexMaxShowCount());


		DWORD dwRealReadSize = 0;
		m_File.SetPoint(m_nScorBarVPos*16);
		m_File.Read(m_Buffer.m_pBuffer,m_Buffer.m_MaxShowCount,&dwRealReadSize);


		m_nRealShowSize = dwRealReadSize;

		return 0;
	}
    int  OnScrollViewScrollH(int pos,BOOL *pbHandled)
    {
        XTRACE("XE_SCROLLVIEW_SCROLL_H %d \n",pos);
        return 0;
    }
    int  OnScrollViewScrollV(int pos,BOOL *pbHandled)
    {
		DWORD dwRealReadSize = 0;
		m_File.SetPoint(pos*16);
		m_File.Read(m_Buffer.m_pBuffer,m_Buffer.m_MaxShowCount,&dwRealReadSize);
		m_nRealShowSize = dwRealReadSize;
		m_nScorBarVPos = pos;

        XTRACE("XE_SCROLLVIEW_SCROLL_V %d \n",pos);
        return 0;
    }

	int OnDrawScrollView(HDRAW hDraw,BOOL *pbHandled)
	{
		m_MemDC.FillRect(0,0,m_MemDC.GetBitMapWidth(),m_MemDC.GetBitMapHeight());
#define SPLIT1_WIDTH  10

		m_MemDC.SetTextColor(RGB(0,0,128));

		int leftSize = 5;
		int offsetX = leftSize;
		int offsetY = 0;
		// draw offset 
		m_MemDC.TextOutW(offsetX,offsetY,m_pTopLeftText,wcslen(m_pTopLeftText));

		offsetX += m_RowCountIndexTextWidth;
		offsetX += SPLIT1_WIDTH;
		// draw header tag
		
		for (int i = 0; i < wcslen(m_pTopMidText); i++)
		{
			m_MemDC.TextOutW(offsetX,offsetY,m_pTopMidText+i,1);
			offsetX += m_nMidTextBlockWidth;
			offsetX += m_nMidTextBlockSpace;
		}

		offsetY += m_nRowHeight;
		offsetX = leftSize;
		m_MemDC.SetTextColor(0);

		int nRowCounter = 0;
		wchar_t buffer[10] = {0};
		for (int j = 0,k = 1; j < m_nRealShowSize; j++,k++)
		{
			if ( (j % 16) == 0)
			{
				wsprintfW(buffer,L"%08X",j+m_nScorBarVPos*16);
				m_MemDC.TextOutW(offsetX,offsetY,buffer,8);
				offsetX += m_RowCountIndexTextWidth;
				offsetX += SPLIT1_WIDTH;
				nRowCounter++;
			}
			
			wchar_t* pStrHex = m_Format.CharToHexStr((wchar_t)(m_Buffer.m_pBuffer[j]));
			m_MemDC.TextOutW(offsetX,offsetY,pStrHex+2,2);
			
			offsetX += m_nMidTextBlockWidth;
			offsetX += m_nMidTextBlockSpace;

			if ( (k%16) == 0)
			{
				offsetY += HEX_TEXT_HEIGHT;
				offsetX = leftSize;
			}
		}


		int x,y;
		XDraw_GetOffset(hDraw,&x,&y);
		m_MemDC.BitBlt3_(hDraw,0,y);
		return 0;
	}

private:
	//计算当前元素最大能显示多少个字符
	int CalcHexMaxShowCount()
	{
		int cy = m_cy - m_nRowHeight;
		return (m_cy / m_nRowHeight + ((m_cy % m_nRowHeight)?1:0) )*16;
	}

private:
	int m_x;
	int m_y;
	int m_cx;
	int m_cy;

	HELE    m_hScrollView;
		
	XFile   m_File;
	XBuffer m_Buffer;
	DataFormat m_Format;
	XGDIMEMDC m_MemDC;


	//行数索引文本的宽度
	int m_RowCountIndexTextWidth;

	int m_nMidTextBlockWidth; //中间16进制文本 单个宽度
	int m_nMidTextBlockSpace; //中间16进制文本 间距

	int m_nRowHeight;
	int m_nRowSpace;

	wchar_t* m_pTopLeftText;
	wchar_t* m_pTopMidText;

	int  m_nScorBarVPos;

	//为什么会有这个，因为读取的实际数据可能大于编辑框可显示的数据大小
	int m_nRealShowSize;
};

class CMyWindowScrollView
{
public:
    HWINDOW m_hWindow;

	XHexEdit m_HexEdit;

    CMyWindowScrollView()
    {
        Init();
    }
    void Init()
    {
        m_hWindow = XWnd_Create(0, 0, 300, 300, L"炫彩界面库窗口", NULL, xc_window_style_default);
        XBtn_SetType(XBtn_Create(5, 3, 60, 20, L"Close", m_hWindow),button_type_close);
		

		HELE hEle = m_HexEdit.Create(20,50,200,200,m_hWindow);
		m_HexEdit.LoadFile(L"g:\\WindowsXP-SP3-VOLMSDN.rar");

		

		HXCGUI hLayoutBody = XLayout_Create();
		XWnd_BindLayoutObject(m_hWindow,window_position_body,hLayoutBody);
		XLayout_SetLayoutHeight(hLayoutBody,layout_size_type_fill,1);
		XLayout_SetLayoutWidth(hLayoutBody,layout_size_type_fill,1);
		
		XLayout_AddEle(hLayoutBody,hEle);
		XEle_SetLayoutWidth(hEle,layout_size_type_fill,1);
		XEle_SetLayoutHeight(hEle,layout_size_type_fill,1);

		XWnd_AdjustLayout(m_hWindow);
        XWnd_ShowWindow(m_hWindow,SW_SHOW);
    }


};

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    XInitXCGUI();
    CMyWindowScrollView  MyWindow;
    XRunXCGUI();
    XExitXCGUI();
    return TRUE;
}