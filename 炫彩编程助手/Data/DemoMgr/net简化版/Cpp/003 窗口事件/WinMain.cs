﻿using System;
using System.Collections.Generic;
//using System.Linq;
using System.Text;
//using System.Threading.Tasks;
using System.Runtime.InteropServices;
using XCGUI;

unsafe public delegate int OnBtnClick(bool* pbHandled);
class CWindow
{
 
   unsafe private int OnWndLButtonDown(UInt32 nFlags,IntPtr pPt,bool *pbHandled)
    {
        POINT pt =(POINT) Marshal.PtrToStructure(pPt, typeof(POINT));
//       Marshal.
        xc.MessageBoxA(0, "炫彩，我是大色", "你好", 0);
        return 0;
    }
    unsafe public void Create()
    {
        xc.XInitXCGUI("");

        int hWindow = XWnd.Create(0, 0, 500, 300, "xcgui", 0, 15);
        int hBtn = XBtn.Create(0, 0, 100, 100, "按钮", hWindow);

        //  Marshal.AllocCoTaskMem(Marshal.SizeOf(pBtnClick))
        XWnd.RegEventCPP(hWindow, xcConst.WM_LBUTTONDOWN, new WM_LBUTTONDOWN_CPP(OnWndLButtonDown));

        XWnd.ShowWindow(hWindow, 5);
        xc.XRunXCGUI();
        xc.XExitXCGUI();
    }
}

namespace Project1
{
    class RunMain
    {
        static void Main()
        {
            CWindow pWindow = new CWindow();
            pWindow.Create();
        }

    }
}