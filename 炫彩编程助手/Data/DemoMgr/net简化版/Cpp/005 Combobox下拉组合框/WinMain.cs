﻿using System;
using System.Collections.Generic;
//using System.Linq;
using System.Text;
//using System.Threading.Tasks;
using System.Runtime.InteropServices;
using XCGUI;


class CWindow
{
    unsafe public void Create()
    {
        xc.XInitXCGUI("");
        int hWindow = XWnd.Create(0, 0, 500, 300, "xcgui", 0, 15);

        int hComboBox = XComboBox.Create(0, 0, 300, 20, hWindow);
        XComboBox.SetItemTemplateXML(hComboBox, "..//..//ComboBox_ListBox_Item.xml");
      
        int hAdapterTable = XAdapterTable.Create();
        XComboBox.BindApapter(hComboBox,hAdapterTable);
        XAdapterTable.AddColumn(hAdapterTable, "name");

        int nItem = XAdapterTable.AddItemText(hAdapterTable, "1.1");

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
