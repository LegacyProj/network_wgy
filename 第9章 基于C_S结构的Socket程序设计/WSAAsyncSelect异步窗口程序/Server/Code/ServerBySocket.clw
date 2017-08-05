; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CServerBySocketDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "ServerBySocket.h"

ClassCount=3
Class1=CServerBySocketApp
Class2=CServerBySocketDlg
Class3=CAboutDlg

ResourceCount=3
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_SERVERBYSOCKET_DIALOG

[CLS:CServerBySocketApp]
Type=0
HeaderFile=ServerBySocket.h
ImplementationFile=ServerBySocket.cpp
Filter=N

[CLS:CServerBySocketDlg]
Type=0
HeaderFile=ServerBySocketDlg.h
ImplementationFile=ServerBySocketDlg.cpp
Filter=D
LastObject=IDC_LOGSTAT
BaseClass=CDialog
VirtualFilter=dWC

[CLS:CAboutDlg]
Type=0
HeaderFile=ServerBySocketDlg.h
ImplementationFile=ServerBySocketDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_SERVERBYSOCKET_DIALOG]
Type=1
Class=CServerBySocketDlg
ControlCount=11
Control1=IDC_IPlist,combobox,1344339970
Control2=IDC_PORT,edit,1350631552
Control3=IDC_STARTSERV,button,1342242817
Control4=IDC_STOPSERV,button,1476460544
Control5=IDCLOSE,button,1342242816
Control6=IDC_CATLOG,edit,1352734916
Control7=IDC_STATIC,static,1342308352
Control8=IDC_STATIC,static,1476526593
Control9=IDC_STATIC,static,1342308352
Control10=IDC_LOGSTAT,static,1476526592
Control11=IDC_CONNSTAT,static,1476526593

