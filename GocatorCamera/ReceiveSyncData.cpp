// ReceiveSyncData.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "ReceiveSyncData.h"
#include "afxdialogex.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define NUM_PROFILES            5
#define RECEIVE_TIMEOUT         200000 
#define SENSOR_IP               "192.168.1.10" 

kAssembly api = kNULL;
extern GoSystem Gosystem = kNULL;
GoSensor sensor = kNULL;
kStatus status;
unsigned int i, j, n;
GoControl goControl = kNULL;
GoDataSet dataset = kNULL;
kIpAddress ipAddress;

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)
#define INVALID_RANGE_16BIT     ((signed short)0x8000)   // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.

typedef struct
{
	double x;   // x-coordinate in engineering units (mm) - position along laser line
	double y;   // y-coordinate in engineering units (mm) - position along the direction of travel
	double z;   // z-coordinate in engineering units (mm) - height (at the given x position)
	unsigned char intensity;
}ProfilePoint;

short int* Syn_height_map_memory = NULL;
unsigned char* Syn_intensity_image_memory = NULL;
ProfilePoint** Syn_surfaceBuffer = NULL;
//unsigned int rowIdx, colIdx;
unsigned int Syn_Length, Syn_Width;
k32u Syn_surfaceBufferHeight = 0;

#pragma warning(disable:4002)
#pragma warning(disable:4244)
#pragma warning(disable:26495)
// ReceiveSyncData 对话框

IMPLEMENT_DYNAMIC(ReceiveSyncData, CDialogEx)

ReceiveSyncData::ReceiveSyncData(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RECEIVESYSDATA, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

ReceiveSyncData::~ReceiveSyncData()
{
}

void ReceiveSyncData::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RECEIVEEDIT, m_syncEdit);
}


BEGIN_MESSAGE_MAP(ReceiveSyncData, CDialogEx)
	ON_BN_CLICKED(IDCANCEL, &ReceiveSyncData::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &ReceiveSyncData::OnBnClickedOk)
	ON_EN_CHANGE(IDC_RECEIVEEDIT, &ReceiveSyncData::OnEnChangeReceiveedit)
	ON_BN_CLICKED(IDC_EXPORTCSV_BTN, &ReceiveSyncData::OnBnClickedExportcsvBtn)
	ON_BN_CLICKED(IDC_SYNC_REFLESH, &ReceiveSyncData::OnBnClickedSyncReflesh)
END_MESSAGE_MAP()


// ReceiveSyncData 消息处理程序
void ReceiveSyncData::SetUseReceiveSync()  //int argc, char **argv
{
	
	// construct Gocator API Library
	SyncStrOut = "初始化进入同步程序\r\n";
	SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);
	m_syncEdit.SetSel(-1, 0, FALSE);
	m_syncEdit.SetFocus();

	m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		SyncStrOut.Format(_T("Error: GoSdk_Construct:%d\n"), status);
		SetDlgItemText(IDC_SYSINFOSHOW, SyncStrOut);
		return;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&Gosystem, kNULL)) != kOK)
	{
		SyncStrOut.Format(_T("Error: GoSdk_Construct:%d\n"), status);
		SetDlgItemText(IDC_SYSINFOSHOW, SyncStrOut);
		return;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress, &sensor)) != kOK)
	{
		SyncStrOut.Format(_T("Error: GoSystem_FindSensor:%d\n"), status);
		SetDlgItemText(IDC_SYSINFOSHOW, SyncStrOut);
		//m_logdlg->WARN(_T("Error: GoSystem_FindSensor:%d\n", status));
		return;
	}

	// create connection to GoSensor object
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		SyncStrOut.Format(_T("Error: GoSensor_Connect:%d\n"), status);
		SetDlgItemText(IDC_SYSINFOSHOW, SyncStrOut);
		//m_logdlg->WARN(_T("Error: GoSensor_Connect:%d\n", status));
		return;
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(Gosystem, kTRUE)) != kOK)
	{
		SyncStrOut.Format(_T("Error: GoSensor_EnableData:%d\n"), status);
		SetDlgItemText(IDC_SYSINFOSHOW, SyncStrOut);
		return;
	}

	// start Gocator sensor
	if ((status = GoSystem_Start(Gosystem)) != kOK)
	{
		SyncStrOut.Format(_T("Error: GoSystem_Start:%d\n"), status);
		SetDlgItemText(IDC_SYSINFOSHOW, SyncStrOut);
		return;
	}

	// loop until 10 results are received
	n = 0;
	while (n < NUM_PROFILES)
	{
		if (GoSystem_ReceiveData(Gosystem, &dataset, RECEIVE_TIMEOUT) == kOK)
		{
			SyncStrOut += (_T("进入同步收取数据中（同步事件信号中）:\r\n"));
			SyncStrTemp.Format(_T("Data message received:\r\n"));
			SyncStrOut += SyncStrTemp;
			SyncStrTemp.Format(_T("Dataset count: %u\r\n"), GoDataSet_Count(dataset));
			SyncStrOut += SyncStrTemp;
			SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);

			// each result can have multiple data items
			// loop through all items in result message
			for (i = 0; i < GoDataSet_Count(dataset); ++i)
			{
				GoDataMsg dataObj = GoDataSet_At(dataset, i);
				// retrieve GoStamp message
				switch (GoDataMsg_Type(dataObj))
				{
				case GO_DATA_MESSAGE_TYPE_STAMP:
				{
					GoStampMsg stampMsg = dataObj;
					//printf("  Stamp Message batch count: %u\n", GoStampMsg_Count(stampMsg));
					SyncStrTemp.Format(_T("  Stamp Message batch count: %u\r\n"), GoStampMsg_Count(stampMsg));
					SyncStrOut += SyncStrTemp;
					SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);
					//SetDlgItemText(IDC_RECEIVESYS, (_T("  Stamp Message batch count: %u\n", GoStampMsg_Count(stampMsg))));
					
					for (j = 0; j < GoStampMsg_Count(stampMsg); ++j)
					{
						GoStamp *stamp = GoStampMsg_At(stampMsg, j);
						SyncStrTemp.Format(_T("  Timestamp: %11u\r\n"), stamp->timestamp);
						SyncStrOut += SyncStrTemp;
						SyncStrTemp.Format(_T("  Encoder: %11d\r\n"), stamp->encoder);
						SyncStrOut += SyncStrTemp;;
						SyncStrTemp.Format(_T("  Frame index: %11u\r\n"), stamp->frameIndex);
						SyncStrOut += SyncStrTemp;
						SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);
					}
				}
				break;
				case GO_DATA_MESSAGE_TYPE_UNIFORM_SURFACE:
				{
					GoSurfaceMsg surfaceMsg = dataObj;
					unsigned int rowIdx, colIdx;

					double XResolution = NM_TO_MM(GoSurfaceMsg_XResolution(surfaceMsg));
					double YResolution = NM_TO_MM(GoSurfaceMsg_YResolution(surfaceMsg));
					double ZResolution = NM_TO_MM(GoSurfaceMsg_ZResolution(surfaceMsg));
					double XOffset = UM_TO_MM(GoSurfaceMsg_XOffset(surfaceMsg));
					double YOffset = UM_TO_MM(GoSurfaceMsg_YOffset(surfaceMsg));
					double ZOffset = UM_TO_MM(GoSurfaceMsg_ZOffset(surfaceMsg));

				
					SyncStrTemp.Format(_T("  Surface data width: %ld\r\n"), GoSurfaceMsg_Width(surfaceMsg));
					SyncStrOut += SyncStrTemp;
					SyncStrTemp.Format(_T("  Surface data length: %ld\r\n\r\n"), GoSurfaceMsg_Length(surfaceMsg));
					SyncStrOut += SyncStrTemp;
					SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);

					//allocate memory if needed
					if (Syn_surfaceBuffer == NULL)
					{
						Syn_surfaceBuffer = (ProfilePoint**)malloc(GoSurfaceMsg_Length(surfaceMsg) * sizeof(ProfilePoint*));

						for (j = 0; j < GoSurfaceMsg_Length(surfaceMsg); j++)
						{
							Syn_surfaceBuffer[j] = (ProfilePoint*)malloc(GoSurfaceMsg_Width(surfaceMsg) * sizeof(ProfilePoint));
						}

						Syn_surfaceBufferHeight = GoSurfaceMsg_Length(surfaceMsg);
					}

					for (rowIdx = 0; rowIdx < GoSurfaceMsg_Length(surfaceMsg); rowIdx++)
					{
						k16s* data = GoSurfaceMsg_RowAt(surfaceMsg, rowIdx);

						for (colIdx = 0; colIdx < GoSurfaceMsg_Width(surfaceMsg); colIdx++)
						{
							// gocator transmits range data as 16-bit signed integers
							// to translate 16-bit range data to engineering units, the calculation for each point is: 
							//          X: XOffset + columnIndex * XResolution 
							//          Y: YOffset + rowIndex * YResolution
							//          Z: ZOffset + height_map[rowIndex][columnIndex] * ZResolution

							Syn_surfaceBuffer[rowIdx][colIdx].x = XOffset + XResolution * colIdx;
							Syn_surfaceBuffer[rowIdx][colIdx].y = YOffset + YResolution * rowIdx;


							if (data[colIdx] != INVALID_RANGE_16BIT)
							{
								Syn_surfaceBuffer[rowIdx][colIdx].z = ZOffset + ZResolution * data[colIdx];
							}
							else
							{
								Syn_surfaceBuffer[rowIdx][colIdx].z = -9999;
							}
						}
						Syn_Length = GoSurfaceMsg_Length(surfaceMsg);
						Syn_Width = GoSurfaceMsg_Width(surfaceMsg);
					}
				}
				break;
				// Refer to example ReceiveRange, ReceiveProfile, ReceiveMeasurement and ReceiveWholePart on how to receive data                
				}
			}
			m_syncEdit.SetSel(-1, 0, FALSE);
			m_syncEdit.SetFocus();

			GoDestroy(dataset);
			n++;
		}
	}

	/*for (int num = 0; num <= 100; num++)
	{
		strOut += (_T("Callback:\r\n"));
		strOut += (_T("Data message received:\r\n"));
		strOut += (_T("Dataset count: %d\r\n", num));
		strOut += (_T("  Timestamp: %llu\r\n", j, stamp->timestamp));
		strOut += (_T("  Encoder: %lld\r\n", j, stamp->encoder));
		strOut += (_T("  Frame index: %llu\r\n", j, stamp->frameIndex));
		SetDlgItemText(IDC_RECEIVEEDIT, strOut);
	}*/

	// stop Gocator sensor
	if ((status = GoSystem_Stop(Gosystem)) != kOK)
	{
		SetDlgItemText(IDC_RECEIVESYS, (LPSTR)(_T("Error: GoSystem_Stop:%d\n"), status));
		return;
	}

	m_syncEdit.SetSel(-1, 0, FALSE);
	m_syncEdit.SetFocus();
	m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	// destroy handles
	GoDestroy(Gosystem);
	GoDestroy(api);

	//MessageBox("相机拍摄完成5次\r\n相机缓存已释放\r\n相机已停止工作");
	MessageBox(_T("相机拍摄完成5次\r\n相机缓存已释放\r\n相机已停止工作"),
		_T("Gocator操作提示"), MB_ICONEXCLAMATION | MB_OK);

	return;

}

void ReceiveSyncData::SetStopCamera()
{
	SetDlgItemText(IDC_SYSINFOSHOW, (_T("开始进入关闭相机程序")));
	// stop Gocator sensor
	if ((status = GoSystem_Stop(Gosystem)) != kOK)
	{
		SetDlgItemText(IDC_RECEIVESYS, (LPSTR)(_T("Error: GoSystem_Stop:%d\n"), status));
		return;
	}

	m_syncEdit.SetSel(-1, 0, FALSE);
	m_syncEdit.SetFocus();
	m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	// destroy handles
	GoDestroy(Gosystem);
	GoDestroy(api);
}

void ReceiveSyncData::TestFor()
{
	//TODO 函数废弃
}

void ReceiveSyncData::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void ReceiveSyncData::OnEnChangeReceiveedit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
	UpdateData(false);
	//m_deitbar.LineScroll(m_deitbar.GetLineCount() - 1, 0);
	//m_deitbar.LineScroll(m_deitbar.GetLineCount(), 0);
	m_syncEdit.SetSel(-1, 0, TRUE);
	m_syncEdit.SetFocus();
	m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
}

int ReceiveSyncData::SaveCsvFromSaveBtnClicked(CString strfilePath)
{
	CString strOut = "0";
	CString strTemp = "0";

	// 设置过滤器   
	TCHAR szFilter[] = _T("CSV文件(*.csv)|*.csv | 所有文件(*.*)|*.*||");
	// 设置按时间保存文件名
	CTime m_Time;
	m_Time = CTime::GetCurrentTime();//获取当前系统时间
	CString str = (CString)m_Time.Format(_T("%Y-%m-%d_%H-%M-%S"));
	// 构造保存文件对话框   
	CFileDialog fileDlg(FALSE, _T("csv"), str, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString strFilePath, setFileName, outputInfo;

	// 显示保存文件对话框   
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里   
		strFilePath = fileDlg.GetPathName();
		setFileName = fileDlg.GetFileName();

		if (SaveCsvFromSaveBtnClicked(strFilePath))			//类型强转发生截断错误
		{
			GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(setFileName);
			UpdateData(FALSE);
			
			strOut = (_T("保存到-点云-文件成功,文件路径为: " + strFilePath + "--INFO消息"));
			SetDlgItemText(IDC_RECEIVEEDIT, strOut);

			//m_logdlg->INFO(_T("保存到文件成功,文件路径为: " + strFilePath + "--INFO消息"));
		}
		else
		{
			strTemp.Format(_T("存储文件失败，请重新操作---WARN消息"));
			strOut += strTemp;
			SetDlgItemText(IDC_RECEIVEEDIT, strOut);

			//m_logdlg->WARN(_T("存储文件失败，请重新操作---WARN消息"));
		}
	}
	else
	{
		//m_logdlg->WARN(_T("无效的目录，请重新选择---WARN消息"));
		MessageBox(_T("无效的目录，请重新选择---WARN消息"),_T("3D相机管理系统-GUI"), MB_OK);
	}
		

	return 0;
}

int ReceiveSyncData::SaveCsvFromSaveBtnClicked()
{
	CString strOut = "0";
	CString strTemp = "0";

	TCHAR szFilter[] = _T("CSV文件(*.csv)|*.csv | 所有文件(*.*)|*.*||");
	// 设置按时间保存文件名
	CTime m_Time;
	m_Time = CTime::GetCurrentTime();//获取当前系统时间
	CString str = (CString)m_Time.Format(_T("%Y-%m-%d_%H-%M-%S"));
	// 构造保存文件对话框   
	CFileDialog fileDlg(FALSE, _T("csv"), str, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
	CString strFilePath, setFileName, outputInfo;

	// 显示保存文件对话框   
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里   
		strFilePath = fileDlg.GetPathName();
		setFileName = fileDlg.GetFileName();

		
		GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(setFileName);
		UpdateData(FALSE);

		strOut = (_T("保存到文件成功,文件路径为: " + strFilePath + "--INFO消息"));
		SetDlgItemText(IDC_RECEIVEEDIT, strOut);

		//m_logdlg->INFO(_T("保存到文件成功,文件路径为: " + strFilePath + "--INFO消息"));
		
	}
	else
	{
		//m_logdlg->WARN(_T("无效的目录，请重新选择---WARN消息"));
		MessageBox(_T("无效的目录，请重新选择---WARN消息"), _T("3D相机管理系统-GUI"), MB_OK);
	}
	return 0;
}

void ReceiveSyncData::ToDoSaveCsv()
{
	//废弃的函数操作
}

void ReceiveSyncData::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}

void ReceiveSyncData::OnBnClickedSyncReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK);

	SyncStrTemp.Format(_T("等待相机进行重连操作.....\r\n"));
	SyncStrOut += SyncStrTemp;
	SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);

	m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	SetUseReceiveSync();
	SetStopCamera();
}

void ReceiveSyncData::ToSavaPointCloudData()
{
	if (Syn_surfaceBuffer != NULL)
	{
		m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

		errno_t err;
		FILE* fpWrite;

		TCHAR szFilter[] = _T("文本文件(*.txt)|*.txt | 所有文件(*.*)|*.*||");
		// 设置按时间保存文件名
		CTime m_Time;
		m_Time = CTime::GetCurrentTime();//获取当前系统时间
		CString str = (CString)m_Time.Format(_T("%Y-%m-%d_%H-%M-%S"));
		// 构造保存文件对话框   
		CFileDialog fileDlg(FALSE, _T("txt"), str, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);


		// 显示保存文件对话框   
		if (IDOK == fileDlg.DoModal())
		{
			// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里   
			strFilePath = fileDlg.GetPathName();
			setFileName = fileDlg.GetFileName();

			//GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(setFileName);

			//TODO 默认先将已有文件名删除一遍
			remove(strFilePath);
			//fpWrite = fopen(setFileName, "w+");
			err = fopen_s(&fpWrite, strFilePath, "w");
			if (err == 0)
			{
				for (unsigned int rowIdx = 0; rowIdx < Syn_Length; rowIdx++)
				{
					for (unsigned int colIdx = 0; colIdx < Syn_Width; colIdx++)
					{
						fprintf(fpWrite, "|%-4.3f||%-4.3f||%-4.3f|\n",
							Syn_surfaceBuffer[rowIdx][colIdx].x, Syn_surfaceBuffer[rowIdx][colIdx].y, Syn_surfaceBuffer[rowIdx][colIdx].z);
					}
				}
			}
			else
			{
				SetDlgItemText(IDC_SYSINFOSHOW, "错误文件名\n");
			}
			fclose(fpWrite);
			SetDlgItemText(IDC_SYSINFOSHOW, "输出数据已完成\n");

			SyncStrTemp.Format(_T("---保存到文件成功,文件路径为: " + strFilePath + "---\r\n\r\n"));
			SyncStrOut += SyncStrTemp;
			SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);
		}
		else
		{
			//m_logdlg->WARN(_T("无效的目录，请重新选择---WARN消息"));
			MessageBox(_T("无效的目录，请重新选择"), _T("3D相机管理系统-GUI"), MB_OK);
		}

		//m_logdlg->INFO(_T("保存到文件成功,文件路径为: " + strFilePath + "--INFO消息"));
	}
	else
	{
		MessageBox(_T("无效的保存文件\r\n数据还未生成"), _T("3D相机管理系统-GUI"), MB_OK | MB_ICONEXCLAMATION);
	}
}

//TODO 保存bmp位图格式
int ReceiveSyncData::ToSaveBMP(CString strFilePath, CString setFileName)
{

	m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	kAssembly assembly = kNULL;      //assembly handle 
	kImage image = kNULL;            //image handle
	kSize width = 32;
	kSize height = 64;
	kSize i, j;
	//get an initialized reference to zen's core assembly (kApiLib)
	kCheck(kApiLib_Construct(&assembly));
	//construct a new image object
	kCheck(kImage_Construct(&image, kTypeOf(k8u), width, height, kNULL));
	//set the image pixels to a test pattern
	for (i = 0; i < height; ++i)
	{
		//get a pointer to the first element in the current row
		k8u* row = kImage_RowAtT(image, i, k8u);
		for (j = 0; j < width; ++j)
		{
			row[j] = (k8u)(i + j);
		}
	}
	//save the image using bmp format
	kCheck(kImage_Export(image, strFilePath));
	//destroy the image object
	kCheck(kObject_Destroy(image));
	//destroy the kApiLib assembly
	kCheck(kObject_Destroy(assembly));


	//m_UseLog.SetINFO("保存位图正常");

	//m_logdlg.INFO("保存位图正常");

	return kOK;

}

void ReceiveSyncData::OnBnClickedExportcsvBtn()
{
	// TODO: 在此添加控件通知处理程序代码

	m_syncEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	errno_t err;
	FILE* fpWrite;

	TCHAR szFilter[] = _T("位图文件(*.bmp)|*.bmp | 所有文件(*.*)|*.*||");
	// 设置按时间保存文件名
	CTime m_Time;
	m_Time = CTime::GetCurrentTime();//获取当前系统时间
	CString str = (CString)m_Time.Format(_T("%Y-%m-%d_%H-%M-%S"));
	// 构造保存文件对话框   
	CFileDialog fileDlg(FALSE, _T("bmp"), str, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);

	// 显示保存文件对话框   
	if (IDOK == fileDlg.DoModal())
	{
		// 如果点击了文件对话框上的“保存”按钮，则将选择的文件路径显示到编辑框里   
		strFilePath = fileDlg.GetPathName();
		setFileName = fileDlg.GetFileName();

		//TODO 默认先将已有文件名删除一遍
		remove(strFilePath);
		//fpWrite = fopen(setFileName, "w+");
		/*err = fopen_s(&fpWrite, strFilePath, "w");
		if (err == 0)
		{
			ToSaveBMP(strFilePath, setFileName);
		}
		else
		{
			SetDlgItemText(IDC_SYSINFOSHOW, "错误文件名\n");
		}*/

		fpWrite = fopen(setFileName, "w+");
		ToSaveBMP(strFilePath, setFileName);
		fclose(fpWrite);
		SetDlgItemText(IDC_SYSINFOSHOW, "输出数据已完成\n");

		SyncStrTemp.Format(_T("---保存到-位图-文件成功,文件路径为: " + strFilePath + "---\r\n\r\n"));
		SyncStrOut += SyncStrTemp;
		SetDlgItemText(IDC_RECEIVEEDIT, SyncStrOut);
	}
	else
	{
		//m_logdlg->WARN(_T("无效的目录，请重新选择---WARN消息"));
		MessageBox(_T("无效的目录，请重新选择---WARN消息"), _T("3D相机管理系统-GUI"), MB_OK);
	}


	return;
}

