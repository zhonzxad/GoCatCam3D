// ToDealOutSetCamera.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "ToDealOutSetCamera.h"
#include "afxdialogex.h"
#include "CCLogWnd.h"



#define RECEIVE_TIMEOUT			20000000
#define INVALID_RANGE_16BIT     ((signed short)0x8000)          // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define DOUBLE_MAX              ((k64f)1.7976931348623157e+308) // 64-bit double - largest positive value.  
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.
#define SENSOR_IP               "192.168.1.10"  

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)

#pragma warning(disable:4002)
#pragma warning(disable:4244)

kStatus kCall onToDealData(void* ctx, void* sys, void* dataset);
CLogWnd *m_ToDealLogDlg;
CString ToDealTemp = NULL;
CString ToDealOut = NULL;

typedef struct
{
	k32u count;
}DataContext;

typedef struct
{
	double x;   // x-coordinate in engineering units (mm) - position along laser line
	double y;   // y-coordinate in engineering units (mm) - position along the direction of travel
	double z;   // z-coordinate in engineering units (mm) - height (at the given x position)
	unsigned char intensity;
}ProfilePoint;



// ToDealOutSetCamera 对话框
IMPLEMENT_DYNAMIC(ToDealOutSetCamera, CDialogEx)

ToDealOutSetCamera::ToDealOutSetCamera(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_GOCATORCAMERA_DIALOG, pParent)
{

}

ToDealOutSetCamera::~ToDealOutSetCamera()
{
}


// data callback function
kStatus kCall onToDealData(void* ctx, void* sys, void* dataset)
{
	unsigned int i, j;
	DataContext *context = (DataContext *)ctx;

	ToDealOut = (_T("进入外部触发的异步回调函数中： \r\n"));
	ToDealTemp.Format(_T("onData Callback:\r\n"));
	ToDealOut += ToDealTemp;
	ToDealTemp.Format(_T("Data message received:\r\n"));
	ToDealOut += ToDealTemp;
	ToDealTemp.Format(_T("Dataset message count: %u\r\n"), GoDataSet_Count(dataset));
	ToDealOut += ToDealTemp;
	SetDlgItemText((HWND)IDC_SYSINFOSHOW,1006 ,ToDealOut);

	short int* height_map_memory = NULL;
	unsigned char* intensity_image_memory = NULL;
	ProfilePoint **surfaceBuffer = NULL;
	k32u surfaceBufferHeight = 0;

	ToDealTemp.Format(_T("Dataset count: %u\r\n"), GoDataSet_Count(dataset));
	ToDealOut += ToDealTemp;
	SetDlgItemText((HWND)IDC_SYSINFOSHOW, 1006, ToDealOut);

	// each result can have multiple data items
	// loop through all items in result message
	for (i = 0; i < GoDataSet_Count(dataset); ++i)
	{
		GoDataMsg dataObj = GoDataSet_At(dataset, i);

		switch (GoDataMsg_Type(dataObj))
		{
		case GO_DATA_MESSAGE_TYPE_STAMP:
		{
			GoStampMsg stampMsg = dataObj;

			ToDealTemp.Format(_T("  Stamp Message batch count: %u\r\r\n\n"), GoStampMsg_Count(stampMsg));
			ToDealOut += ToDealTemp;
			SetDlgItemText((HWND)IDC_SYSINFOSHOW, 1006, ToDealOut);

			for (j = 0; j < GoStampMsg_Count(stampMsg); j++)
			{
				GoStamp *stamp = GoStampMsg_At(stampMsg, j);

				ToDealTemp.Format(_T("  Timestamp: %llu\r\n"), stamp->timestamp);
				ToDealOut += ToDealTemp;
				ToDealTemp.Format(_T("  Encoder position at leading edge: %lld\r\n"),stamp->encoder);
				ToDealOut += ToDealTemp;
				ToDealTemp.Format(_T("  Frame index: %llu\r\n"), stamp->frameIndex);
				ToDealOut += ToDealTemp;
				SetDlgItemText((HWND)IDC_SYSINFOSHOW, 1006, ToDealOut);

				context->count++;
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

			/*GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowTextA(_T(
				"Surface data width: %ld\n Surface data length: %ld : %ld",
				GoSurfaceMsg_Width(surfaceMsg), GoSurfaceMsg_Length(surfaceMsg)));*/

			ToDealTemp.Format(_T("  Surface data width: %ld\r\n"), GoSurfaceMsg_Width(surfaceMsg));
			ToDealOut += ToDealTemp;
			ToDealTemp.Format(_T("  Surface data length: %ld\r\n"), GoSurfaceMsg_Length(surfaceMsg));
			ToDealOut += ToDealTemp;
			SetDlgItemText((HWND)IDC_SYSINFOSHOW, 1006, ToDealOut);

			//allocate memory if needed
			if (surfaceBuffer == NULL)
			{
				surfaceBuffer = (ProfilePoint **)malloc(GoSurfaceMsg_Length(surfaceMsg) * sizeof(ProfilePoint *));

				for (j = 0; j < GoSurfaceMsg_Length(surfaceMsg); j++)
				{
					surfaceBuffer[j] = (ProfilePoint *)malloc(GoSurfaceMsg_Width(surfaceMsg) * sizeof(ProfilePoint));
				}

				surfaceBufferHeight = GoSurfaceMsg_Length(surfaceMsg);
			}

			for (rowIdx = 0; rowIdx < GoSurfaceMsg_Length(surfaceMsg); rowIdx++)
			{
				k16s *data = GoSurfaceMsg_RowAt(surfaceMsg, rowIdx);

				for (colIdx = 0; colIdx < GoSurfaceMsg_Width(surfaceMsg); colIdx++)
				{
					// gocator transmits range data as 16-bit signed integers
					// to translate 16-bit range data to engineering units, the calculation for each point is: 
					//          X: XOffset + columnIndex * XResolution 
					//          Y: YOffset + rowIndex * YResolution
					//          Z: ZOffset + height_map[rowIndex][columnIndex] * ZResolution

					surfaceBuffer[rowIdx][colIdx].x = XOffset + XResolution * colIdx;
					surfaceBuffer[rowIdx][colIdx].y = YOffset + YResolution * rowIdx;


					if (data[colIdx] != INVALID_RANGE_16BIT)
					{
						surfaceBuffer[rowIdx][colIdx].z = ZOffset + ZResolution * data[colIdx];
					}
					else
					{
						surfaceBuffer[rowIdx][colIdx].z = INVALID_RANGE_DOUBLE;
					}
				}
			}
			//TODO 保存surfaceBuffer到对应文件目录
			//errno_t err;
			//FILE *fpWrite;

			//err = fopen_s(&fpWrite, "PointCloud.txt", "w+");

			//TODO 计算数据量大小 总共2.2g
			/*auto len = GoSurfaceMsg_Length(surfaceMsg);
			auto wid = GoSurfaceMsg_Width(surfaceMsg);
			char buf[1000] = { 0 };
			sprintf(buf, "|%5.5f||%5.5f||%5.5f|\n",
			surfaceBuffer[0][0].x,
			surfaceBuffer[0][0].y,
			surfaceBuffer[0][0].z);
			auto buf_len = std::strlen(buf);
			std::size_t total_size = len * wid*buf_len;*/

			//计算程序运行时间以及保存文件
			/*clock_t start, finish;
			double duration;

			start = clock();
			if (err == 0)
			{

				for (rowIdx = 0; rowIdx < GoSurfaceMsg_Length(surfaceMsg); rowIdx++)
				{
					for (colIdx = 0; colIdx < GoSurfaceMsg_Width(surfaceMsg); colIdx++)
					{
						fprintf(fpWrite, "|%5.5f||%5.5f||%5.5f|\n",
							surfaceBuffer[rowIdx][colIdx].x,
							surfaceBuffer[rowIdx][colIdx].y,
							surfaceBuffer[rowIdx][colIdx].z);
					}
				}
			}
			else
			{
				m_ToDealLogDlg->INFO(_T("错误文件名\n"));
			}

			fclose(fpWrite);

			finish = clock();
			duration = (double)(finish - start) / CLOCKS_PER_SEC;
			m_ToDealLogDlg->INFO(_T("------程序运行了%f s\n------", duration));

			m_ToDealLogDlg->INFO(_T("输出数据已完成\n\n\n\n"));*/
		}
		break;
		case GO_DATA_MESSAGE_TYPE_SURFACE_POINT_CLOUD:
		{
			GoSurfacePointCloudMsg surfacePointCloudMsg = dataObj;
			unsigned int rowIdx, colIdx;

			double XResolution = NM_TO_MM(GoSurfacePointCloudMsg_XResolution(surfacePointCloudMsg));
			double YResolution = NM_TO_MM(GoSurfacePointCloudMsg_YResolution(surfacePointCloudMsg));
			double ZResolution = NM_TO_MM(GoSurfacePointCloudMsg_ZResolution(surfacePointCloudMsg));
			double XOffset = UM_TO_MM(GoSurfacePointCloudMsg_XOffset(surfacePointCloudMsg));
			double YOffset = UM_TO_MM(GoSurfacePointCloudMsg_YOffset(surfacePointCloudMsg));
			double ZOffset = UM_TO_MM(GoSurfacePointCloudMsg_ZOffset(surfacePointCloudMsg));

			/*GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowTextA(_T(
				"Surface Point Cloud data width: %ld\n  Surface Point Cloud data length : %ld", 
				GoSurfacePointCloudMsg_Width(surfacePointCloudMsg),GoSurfacePointCloudMsg_Length(surfacePointCloudMsg)));*/

			ToDealTemp.Format(_T("  Surface Point Cloud data width: %ld\r\n"),
				GoSurfacePointCloudMsg_Width(surfacePointCloudMsg));
			ToDealOut += ToDealTemp;
			ToDealTemp.Format(_T("  Surface Point Cloud data length: %ld\r\n"),
				GoSurfacePointCloudMsg_Length(surfacePointCloudMsg));
			ToDealOut += ToDealTemp;
			SetDlgItemText((HWND)IDC_SYSINFOSHOW, 1006, ToDealOut);

			//allocate memory if needed
			if (surfaceBuffer == NULL)
			{
				surfaceBuffer = (ProfilePoint **)malloc(GoSurfacePointCloudMsg_Length(surfacePointCloudMsg) * sizeof(ProfilePoint *));

				for (j = 0; j < GoSurfacePointCloudMsg_Length(surfacePointCloudMsg); j++)
				{
					surfaceBuffer[j] = (ProfilePoint *)malloc(GoSurfacePointCloudMsg_Width(surfacePointCloudMsg) * sizeof(ProfilePoint));
				}

				surfaceBufferHeight = GoSurfacePointCloudMsg_Length(surfacePointCloudMsg);
			}

			for (rowIdx = 0; rowIdx < GoSurfacePointCloudMsg_Length(surfacePointCloudMsg); rowIdx++)
			{
				kPoint3d16s *data = GoSurfacePointCloudMsg_RowAt(surfacePointCloudMsg, rowIdx);

				for (colIdx = 0; colIdx < GoSurfacePointCloudMsg_Width(surfacePointCloudMsg); colIdx++)
				{
					surfaceBuffer[rowIdx][colIdx].x = XOffset + XResolution * data[colIdx].x;
					surfaceBuffer[rowIdx][colIdx].y = YOffset + YResolution * data[colIdx].y;

					if (data[colIdx].z != INVALID_RANGE_16BIT)
					{
						surfaceBuffer[rowIdx][colIdx].z = ZOffset + ZResolution * data[colIdx].z;
					}
					else
					{
						surfaceBuffer[rowIdx][colIdx].z = INVALID_RANGE_DOUBLE;
					}
				}
			}
			////TODO 保存surfaceBuffer到对应文件目录
			//errno_t err;
			//FILE *fpWrite;
			//remove("PointCloud.txt");
			////FILE *fpWrite = fopen("PointCloud.txt", "w+");
			//err = fopen_s(&fpWrite, "PointCloud.txt", "w");
			//if (err == 0)
			//{
			//	for (rowIdx = 0; rowIdx < GoSurfacePointCloudMsg_Length(surfacePointCloudMsg); rowIdx++)
			//	{
			//		for (colIdx = 0; colIdx < GoSurfacePointCloudMsg_Width(surfacePointCloudMsg); colIdx++)
			//		{
			//			fprintf(fpWrite, "|%5.5f||%5.5f||%5.5f|\n", surfaceBuffer[rowIdx][colIdx].x, surfaceBuffer[rowIdx][colIdx].y, surfaceBuffer[rowIdx][colIdx].z);
			//		}
			//	}
			//}
			//else
			//	printf("错误文件名\n");
			//fclose(fpWrite);
			//printf("输出数据已完成\n");
		}
		break;
		case GO_DATA_MESSAGE_TYPE_SURFACE_INTENSITY:
		{
			GoSurfaceIntensityMsg surfaceIntMsg = dataObj;
			unsigned int rowIdx, colIdx;
			double XResolution = NM_TO_MM(GoSurfaceIntensityMsg_XResolution(surfaceIntMsg));
			double YResolution = NM_TO_MM(GoSurfaceIntensityMsg_YResolution(surfaceIntMsg));
			double XOffset = UM_TO_MM(GoSurfaceIntensityMsg_XOffset(surfaceIntMsg));
			double YOffset = UM_TO_MM(GoSurfaceIntensityMsg_YOffset(surfaceIntMsg));

			/*GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowTextA(_T(
				"Surface intensity width: %ld\n Surface intensity height: %ld",
				GoSurfaceIntensityMsg_Width(surfaceIntMsg), GoSurfaceIntensityMsg_Length(surfaceIntMsg)));*/

			ToDealTemp.Format(_T("Surface intensity width: %ld\r\n"), GoSurfaceIntensityMsg_Width(surfaceIntMsg));
			ToDealOut += ToDealTemp;
			ToDealTemp.Format(_T("Surface intensity height: %ld\r\n"), GoSurfaceIntensityMsg_Length(surfaceIntMsg));
			ToDealOut += ToDealTemp;
			SetDlgItemText((HWND)IDC_SYSINFOSHOW, 1006, ToDealOut);

			//allocate memory if needed
			if (surfaceBuffer == NULL)
			{
				surfaceBuffer = (ProfilePoint **)malloc(GoSurfaceIntensityMsg_Length(surfaceIntMsg) * sizeof(ProfilePoint *));

				for (j = 0; j < GoSurfaceIntensityMsg_Length(surfaceIntMsg); j++)
				{
					surfaceBuffer[j] = (ProfilePoint *)malloc(GoSurfaceIntensityMsg_Width(surfaceIntMsg) * sizeof(ProfilePoint));
				}

				surfaceBufferHeight = GoSurfaceIntensityMsg_Length(surfaceIntMsg);
			}

			for (rowIdx = 0; rowIdx < GoSurfaceIntensityMsg_Length(surfaceIntMsg); rowIdx++)
			{
				k8u *data = GoSurfaceIntensityMsg_RowAt(surfaceIntMsg, rowIdx);

				// gocator transmits intensity data as an 8-bit grayscale image of identical width and height as the corresponding height map
				for (colIdx = 0; colIdx < GoSurfaceIntensityMsg_Width(surfaceIntMsg); colIdx++)
				{
					surfaceBuffer[rowIdx][colIdx].x = XOffset + XResolution * colIdx;
					surfaceBuffer[rowIdx][colIdx].y = YOffset + YResolution * rowIdx;
					surfaceBuffer[rowIdx][colIdx].intensity = data[colIdx];
				}
			}
			//TODO 保存surfaceBuffer到对应文件目录
			//errno_t err;
			//FILE *fpWrite;
			//remove("PointCloud.txt");
			////FILE *fpWrite = fopen("PointCloud.txt", "w+");
			//err = fopen_s(&fpWrite, "PointCloud.txt", "w");
			//if (err == 0)
			//{
			//	for (rowIdx = 0; rowIdx < GoSurfaceIntensityMsg_Length(surfaceIntMsg); rowIdx++)
			//	{
			//		for (colIdx = 0; colIdx < GoSurfaceIntensityMsg_Width(surfaceIntMsg); colIdx++)
			//		{
			//			fprintf(fpWrite, "|%20.5f||%20.5f||%20.5f|\n", surfaceBuffer[rowIdx][colIdx].x, surfaceBuffer[rowIdx][colIdx].y, surfaceBuffer[rowIdx][colIdx].z);
			//		}
			//	}
			//}
			//else
			//	SetDlgItemText((HWND)IDC_FILENAMEINFOSHOW, 1005,_T("错误文件名\n"));
			//	//GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowTextA(_T("错误文件名\n"));
			//fclose(fpWrite);
			////GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowTextA(_T("输出数据已完成，文件已经保存到本地。"));
			//SetDlgItemText((HWND)IDC_FILENAMEINFOSHOW, 1005, _T("输出数据已完成，文件已经保存到本地。\n"));
		}
		default:
			break;
		}
	}

	GoDestroy(dataset);
	//free memory arrays
	if (surfaceBuffer)
	{
		unsigned int i;
		for (i = 0; i < surfaceBufferHeight; i++)
		{
			free(surfaceBuffer[i]);
		}
	}
	else
	{
		ToDealTemp.Format(_T("Error: No data received during the waiting period\r\n"));
		ToDealOut += ToDealTemp;
		SetDlgItemText((HWND)IDC_SYSINFOSHOW, 1006, ToDealOut);
	}
	return kOK;
}

int ToDealOutSetCamera::SetOutPutStartCamera()	//int argc, char **argv
{
	ToDealOut = (_T("进入外部触发拍摄系统:\r\n"));
	SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);

	kAssembly api = kNULL;
	kStatus status;
	GoSystem system = kNULL;
	GoSensor sensor = kNULL;
	GoDataSet dataset = kNULL;
	kIpAddress ipAddress;

	DataContext contextPointer;

	// construct Gocator API Library
	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		ToDealTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
		ToDealOut += ToDealTemp;
		SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
		return 0;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&system, kNULL)) != kOK)
	{
		ToDealTemp.Format(_T("Error: GoSystem_Construct:%d\r\n"), status);
		ToDealOut = ToDealTemp;
		SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
		return 0;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(system, &ipAddress, &sensor)) != kOK)
	{
		ToDealTemp.Format(_T("Error: GoSystem_FindSensor:%d\r\n"), status);
		ToDealOut += ToDealTemp;
		SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
		return 0;
	}

	// create connection to GoSystem object
	if ((status = GoSystem_Connect(system)) != kOK)
	{
		ToDealTemp.Format(_T("Error: GoSystem_Connect:%d\r\n"), status);
		ToDealOut = ToDealTemp;
		SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
		return 0;
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(system, kTRUE)) != kOK)
	{
		ToDealTemp.Format(_T("Error: GoSensor_EnableData:%d\r\n"), status);
		ToDealOut += ToDealTemp;
		SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
		return 0;
	}

	// set data handler to receive data asynchronously
	if ((status = GoSystem_SetDataHandler(system, onToDealData, &contextPointer)) != kOK)
	{
		ToDealTemp.Format(_T("Error: GoSystem_SetDataHandler:%d\r\n"), status);
		ToDealOut += ToDealTemp;
		SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
		return 0;
	}

	// start Gocator sensor
	if ((status = GoSystem_Start(system)) != kOK)
	{
		ToDealTemp.Format(_T("Error: GoSystem_Start:%d\r\n"), status);
		ToDealOut += ToDealTemp;
		SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
		return 0;
	}

	ToDealTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
	ToDealOut += ToDealTemp;
	SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
	//m_ToDealLogDlg->WARN(_T("正在准备停止相机...\n"));
	//getchar();

	//// stop Gocator sensor
	//if ((status = GoSystem_Stop(system)) != kOK)
	//{
	//	printf("Error: GoSystem_Stop:%d\n", status);
	//	return 0;
	//}

	// destroy handles
	GoDestroy(system);
	GoDestroy(api);

	ToDealTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
	ToDealOut = ToDealTemp;
	SetDlgItemText(IDC_SYSINFOSHOW, ToDealOut);
	m_ToDealLogDlg->WARN(_T("相机已停止...\r\n"));
	//getchar();
	return 0;
}

void ToDealOutSetCamera::SetStaticTxt()
{
	GetDlgItem(IDC_FILENAMEINFOSHOW)->SetWindowText(_T("成功"));
}

void ToDealOutSetCamera::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(ToDealOutSetCamera, CDialogEx)
END_MESSAGE_MAP()


// ToDealOutSetCamera 消息处理程序
