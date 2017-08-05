; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CReceiverDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "Receiver.h"

ClassCount=3
Class1=CReceiverApp
Class2=CReceiverDlg
Class3=CAboutDlg

ResourceCount=4
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_RECEIVER_DIALOG
Resource4=IDR_MENU_POPUP

[CLS:CReceiverApp]
Type=0
HeaderFile=Receiver.h
ImplementationFile=Receiver.cpp
Filter=N

[CLS:CReceiverDlg]
Type=0
HeaderFile=ReceiverDlg.h
ImplementationFile=ReceiverDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=ID_MENU_CLEANUP

[CLS:CAboutDlg]
Type=0
HeaderFile=ReceiverDlg.h
ImplementationFile=ReceiverDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_STATIC,static,1342308352

[DLG:IDD_RECEIVER_DIALOG]
Type=1
Class=CReceiverDlg
ControlCount=29
Control1=ID_START_RECV,button,1342242817
Control2=IDC_STOP_RECV,button,1476460544
Control3=IDCANCEL,button,1342242816
Control4=IDC_STATIC,button,1342308359
Control5=IDC_RECV_WND_SIZE,edit,1350631552
Control6=IDC_RECV_INTERVAL,edit,1350631552
Control7=IDC_AUX_TIMER,edit,1350631552
Control8=IDC_STATIC,static,1342308352
Control9=IDC_STATIC,static,1342308352
Control10=IDC_STATIC,static,1342308352
Control11=IDC_STATIC,button,1342308359
Control12=IDC_RANDOM_ERR,button,1476591625
Control13=IDC_MANUAL_ERR,button,1342242825
Control14=IDC_STATIC,static,1342308352
Control15=IDC_EDIT_ERR_ACK,edit,1484849280
Control16=IDC_STATIC,static,1342308352
Control17=IDC_EDIT_LOST_ACK,edit,1350631552
Control18=IDC_LIST_OUTPUT,listbox,1352728833
Control19=IDC_STATIC,button,1342177287
Control20=IDC_STATIC,static,1342308352
Control21=IDC_STATIC,static,1342308352
Control22=IDC_CUR_RCVD,static,1342308352
Control23=IDC_STATIC,static,1342308352
Control24=IDC_STATIC,static,1342308352
Control25=IDC_BOTTOM,static,1342308352
Control26=IDC_STATIC,static,1342308352
Control27=IDC_TOP,static,1342308352
Control28=IDC_STATIC,static,1342308352
Control29=IDC_CUR_TOKEN,static,1342308352

[MNU:IDR_MENU_POPUP]
Type=1
Class=?
Command1=ID_MENU_CLEANUP
CommandCount=1

