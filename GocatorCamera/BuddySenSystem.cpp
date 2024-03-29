// BuddySenSystem.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "BuddySenSystem.h"
#include "afxdialogex.h"
#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <windows.h>

#define RECEIVE_TIMEOUT         (20000000) 
#define INVALID_RANGE_16BIT     ((signed short)0x8000)          // gocator transmits range data as 16-bit signed integers. 0x8000 signifies invalid range data. 
#define DOUBLE_MAX              ((k64f)1.7976931348623157e+308) // 64-bit double - largest positive value.  
#define INVALID_RANGE_DOUBLE    ((k64f)-DOUBLE_MAX)             // floating point value to represent invalid range data.    
#define SENSOR_IP               "192.168.1.10"  
#define BUDDY_IP                "192.168.1.6"

#define NM_TO_MM(VALUE) (((k64f)(VALUE))/1000000.0)
#define UM_TO_MM(VALUE) (((k64f)(VALUE))/1000.0)

typedef struct
{
	double x;   // x-coordinate in engineering units (mm) - position along laser line
	double z;   // z-coordinate in engineering units (mm) - height (at the given x position)
	unsigned char intensity;
}ProfilePoint;

#pragma warning(disable:4002)

ProfilePoint *profileBuffer = NULL;

// BuddySenSystem 对话框

IMPLEMENT_DYNAMIC(BuddySenSystem, CDialogEx)

BuddySenSystem::BuddySenSystem(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BUDDYSYSTEM, pParent)
{

}

BuddySenSystem::~BuddySenSystem()
{
}

void BuddySenSystem::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUDDYEDIT, m_buddyEdit);
}


BEGIN_MESSAGE_MAP(BuddySenSystem, CDialogEx)
	ON_BN_CLICKED(IDOK, &BuddySenSystem::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUDD_REFLESH, &BuddySenSystem::OnBnClickedBuddReflesh)
END_MESSAGE_MAP()


// BuddySenSystem 消息处理程序
void BuddySenSystem::SetUseBuddySysteam()  //int argc, char **argv
{

	BuddyStrOut = (_T("进入伙伴传感器连接系统（.6-3504与.10-3506）:\r\n"));
	BuddyStrTemp.Format("使用两种传感器，型号为3504系列和3506系列。\r\n");
	BuddyStrOut += BuddyStrTemp;
	BuddyStrTemp.Format("会根据不同的IP地址配置找到两个传感器。\r\n\r\n");
	BuddyStrOut += BuddyStrTemp;
	SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
	m_buddyEdit.SetSel(-1, 0, FALSE);
	m_buddyEdit.SetFocus();

	// construct Gocator API Library
	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		BuddyStrTemp.Format(_T("Error: GoSdk_Construct:%d\n"), status);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&Gosystem, kNULL)) != kOK)
	{
		BuddyStrTemp.Format(_T("Error: GoSystem_Construct:%d\n"), status);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	// Parse IP address into address data structure for main sensor
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address for main sensor
	if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress, &sensor)) != kOK)
	{
		BuddyStrTemp.Format(_T("Error: GoSystem_FindSensor:%d\n"), status);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	//NOTE: GoSystem_Connect() can be used on Gocator buddy systems already configured, the GoSystem() will be able to
	//recognize the buddy sensor as a paired buddy and not attempt to obtain a separate connection from the budfdy sensor. 
	//In this example, we are trying to pair two sensors as buddies, hence the need to use GoSensor_Connect() to retrieve sensor
	//handles for both Main and Buddy sensor.

	// create connection to GoSensor object for main sensor
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		BuddyStrTemp.Format(_T("Error: GoSensor_Connect:%d\n"), status);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	// Check if buddy sensor has already being assigned, assign buddy if buddy has not being assigned
	if (!GoSensor_HasBuddy(sensor))
	{
		// Parse IP address into address data structure for buddy sensor
		kIpAddress_Parse(&ipAddress_buddy, BUDDY_IP);

		// obtain GoSensor object by sensor IP address for buddy sensor
		if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress_buddy, &buddy)) != kOK)
		{
			//BuddyStrTemp.Format(_T("Error: GoSystem_FindSensor:%d\n", status));
			BuddyStrTemp.Format(_T("缺少必要的IP地址返回值！！！\r\n"));
			BuddyStrOut += BuddyStrTemp;
			SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
			return;
		}

		// create connection to GoSensor object for buddy sensor
		if ((status = GoSensor_Connect(buddy)) != kOK)
		{
			BuddyStrTemp.Format(_T("Error: GoSensor_Connect:%d\n"), status);
			BuddyStrOut += BuddyStrTemp;
			SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
			return;
		}

		// Assign buddy sensor connection
		if ((status = GoSensor_AddBuddy(sensor, buddy)) != kOK)
		{
			BuddyStrTemp.Format(_T("Error: GoSensor_AddBuddy:%d\n"), status);
			BuddyStrOut += BuddyStrTemp;
			SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
			return;
		}

		BuddyStrTemp.Format(_T("Buddy sensor assigned.\n"));
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);

		//m_logdlg->WARN(_T("Buddy sensor assigned.\n"));
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(Gosystem, kTRUE)) != kOK)
	{
		BuddyStrTemp.Format(_T("Error: GoSensor_EnableData:%d\n"), status);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	// retrieve setup handle for main sensor
	if ((setup = GoSensor_Setup(sensor)) == kNULL)
	{
		BuddyStrTemp.Format(_T("Error: GoSensor_Setup: Invalid Handle\n"));
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	// retrieve total number of profile points prior to starting the sensor
	if (GoSetup_UniformSpacingEnabled(setup))
	{
		// Uniform spacing is enabled. The number is based on the X Spacing setting
		profilePointCount = GoSetup_XSpacingCount(setup, GO_ROLE_MAIN) + GoSetup_XSpacingCount(setup, GO_ROLE_BUDDY);
	}
	else
	{
		// non-uniform spacing is enabled. The max number is based on the number of columns used in the camera. 
		profilePointCount = GoSetup_FrontCameraWidth(setup, GO_ROLE_MAIN);
	}

	if ((profileBuffer = (ProfilePoint *)malloc(profilePointCount * sizeof(ProfilePoint))) == kNULL)
	{
		BuddyStrTemp.Format(_T("Error: Cannot allocate profileData, %d points\n"), profilePointCount);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		//printf("Error: Cannot allocate profileData, %d points\n", profilePointCount);
		return;
	}

	m_buddyEdit.SetSel(-1, 0, FALSE);
	m_buddyEdit.SetFocus();

	// start Gocator system
	if ((status = GoSystem_Start(Gosystem)) != kOK)
	{
		BuddyStrTemp.Format(_T("Error: GoSystem_Start:%d\n"), status);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	if (GoSystem_ReceiveData(Gosystem, &dataset, RECEIVE_TIMEOUT) == kOK)
	{
		BuddyStrTemp.Format(_T("Data message received:\n"));
		BuddyStrOut += BuddyStrTemp;
		BuddyStrTemp.Format(_T("Dataset count: %u\n"), GoDataSet_Count(dataset));
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);

		/*m_logdlg->WARN(_T("Data message received:\n"));
		m_logdlg->WARN(_T("Dataset count: %u\n", GoDataSet_Count(dataset)));*/

		// each result can have multiple data items
		// loop through all items in result message
		for (i = 0; i < GoDataSet_Count(dataset); ++i)
		{
			dataObj = GoDataSet_At(dataset, i);
			//Retrieve GoStamp message
			switch (GoDataMsg_Type(dataObj))
			{
			case GO_DATA_MESSAGE_TYPE_STAMP:
			{
				GoStampMsg stampMsg = dataObj;

				BuddyStrTemp.Format(_T("Stamp Message batch count: %u\n"), GoStampMsg_Count(stampMsg));
				BuddyStrOut += BuddyStrTemp;
				SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
				//m_logdlg->WARN(_T("Stamp Message batch count: %u\n", GoStampMsg_Count(stampMsg)));
				for (j = 0; j < GoStampMsg_Count(stampMsg); ++j)
				{
					stamp = GoStampMsg_At(stampMsg, j);

					BuddyStrTemp.Format(_T("  Timestamp: %llu\n"), stamp->timestamp);
					BuddyStrOut += BuddyStrTemp;
					BuddyStrTemp.Format(_T("  Encoder: %lld\n"), stamp->encoder);
					BuddyStrOut += BuddyStrTemp;
					BuddyStrTemp.Format(_T("  Frame index: %llu\n"), stamp->frameIndex);
					BuddyStrOut += BuddyStrTemp;
					SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);

					/*m_logdlg->WARN(_T("  Timestamp: %llu\n", stamp->timestamp));
					m_logdlg->WARN(_T("  Encoder: %lld\n", stamp->encoder));
					m_logdlg->WARN(_T("  Frame index: %llu\n", stamp->frameIndex));*/
				}
			}
			break;
			case GO_DATA_MESSAGE_TYPE_RESAMPLED_PROFILE:
			{
				GoResampledProfileMsg profileMsg = dataObj;

				BuddyStrTemp.Format(_T("Resampled Profile Message batch count: %u\n"), GoResampledProfileMsg_Count(profileMsg));
				BuddyStrOut += BuddyStrTemp;
				SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
				//m_logdlg->WARN(_T("Resampled Profile Message batch count: %u\n", GoResampledProfileMsg_Count(profileMsg)));

				for (k = 0; k < GoResampledProfileMsg_Count(profileMsg); ++k)
				{
					unsigned int validPointCount = 0;
					short* data = GoResampledProfileMsg_At(profileMsg, k);
					double XResolution = NM_TO_MM(GoResampledProfileMsg_XResolution(profileMsg));
					double ZResolution = NM_TO_MM(GoResampledProfileMsg_ZResolution(profileMsg));
					double XOffset = UM_TO_MM(GoResampledProfileMsg_XOffset(profileMsg));
					double ZOffset = UM_TO_MM(GoResampledProfileMsg_ZOffset(profileMsg));

					//translate 16-bit range data to engineering units and copy profiles to memory array
					for (arrayIndex = 0; arrayIndex < GoResampledProfileMsg_Width(profileMsg); ++arrayIndex)
					{
						if (data[arrayIndex] != INVALID_RANGE_16BIT)
						{
							profileBuffer[arrayIndex].x = XOffset + XResolution * arrayIndex;
							profileBuffer[arrayIndex].z = ZOffset + ZResolution * data[arrayIndex];
							validPointCount++;
						}
						else
						{
							profileBuffer[arrayIndex].x = XOffset + XResolution * arrayIndex;
							profileBuffer[arrayIndex].z = INVALID_RANGE_DOUBLE;
						}
					}
					BuddyStrTemp.Format(_T("  Profile Valid Point %d out of max %d\n"), validPointCount, profilePointCount);
					BuddyStrOut += BuddyStrTemp;
					SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
				}
			}
			break;
			case GO_DATA_MESSAGE_TYPE_PROFILE: // Note this is NON resampled profile            
			{
				GoProfileMsg profileMsg = dataObj;

				BuddyStrTemp.Format(_T("Profile Message batch count: %u\n"), GoProfileMsg_Count(profileMsg));
				BuddyStrOut += BuddyStrTemp;
				SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);

				for (k = 0; k < GoProfileMsg_Count(profileMsg); ++k)
				{
					kPoint16s* data = GoProfileMsg_At(profileMsg, k);
					unsigned int validPointCount = 0;
					double XResolution = NM_TO_MM(GoProfileMsg_XResolution(profileMsg));
					double ZResolution = NM_TO_MM(GoProfileMsg_ZResolution(profileMsg));
					double XOffset = UM_TO_MM(GoProfileMsg_XOffset(profileMsg));
					double ZOffset = UM_TO_MM(GoProfileMsg_ZOffset(profileMsg));

					//translate 16-bit range data to engineering units and copy profiles to memory array
					for (arrayIndex = 0; arrayIndex < GoProfileMsg_Width(profileMsg); ++arrayIndex)
					{
						if (data[arrayIndex].x != INVALID_RANGE_16BIT)
						{
							profileBuffer[arrayIndex].x = XOffset + XResolution * data[arrayIndex].x;
							profileBuffer[arrayIndex].z = ZOffset + ZResolution * data[arrayIndex].y;
							validPointCount++;
						}
						else
						{
							profileBuffer[arrayIndex].x = INVALID_RANGE_DOUBLE;
							profileBuffer[arrayIndex].z = INVALID_RANGE_DOUBLE;
						}
					}
					BuddyStrTemp.Format(_T("  Profile Valid Point %d out of max %d\n"), validPointCount, profilePointCount);
					BuddyStrOut += BuddyStrTemp;
					SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
				}
			}
			break;
			case GO_DATA_MESSAGE_TYPE_PROFILE_INTENSITY:
			{
				unsigned int validPointCount = 0;
				GoProfileIntensityMsg intensityMsg = dataObj;

				BuddyStrTemp.Format(_T("Intensity Message batch count: %u\n"), GoProfileIntensityMsg_Count(intensityMsg));
				BuddyStrOut += BuddyStrTemp;
				SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);

				for (k = 0; k < GoProfileIntensityMsg_Count(intensityMsg); ++k)
				{
					unsigned char* data = GoProfileIntensityMsg_At(intensityMsg, k);
					for (arrayIndex = 0; arrayIndex < GoProfileIntensityMsg_Width(intensityMsg); ++arrayIndex)
					{
						profileBuffer[arrayIndex].intensity = data[arrayIndex];
					}
				}
			}
			break;
			}
		}
		GoDestroy(dataset);
	}
	else
	{
		BuddyStrTemp.Format(_T("Error: No data received during the waiting period\n"));
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
	}

	m_buddyEdit.SetSel(-1, 0, FALSE);
	m_buddyEdit.SetFocus();

	return ;
}

void BuddySenSystem::SetStopCamera()
{
	// stop Gocator sensor
	if ((status = GoSystem_Stop(Gosystem)) != kOK)
	{
		BuddyStrTemp.Format(_T("Error: GoSensor_Stop:%d\n"), status);
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
		return;
	}

	// Remove buddy sensor assignment, first check if buddy is still assigned
	if (GoSensor_HasBuddy(sensor))
	{
		// Remove buddy sensor assignment
		if ((status = GoSensor_RemoveBuddy(sensor)) != kOK)
		{
			BuddyStrTemp.Format(_T("Error: GoSensor_RemoveBuddy:%d\n"), status);
			BuddyStrOut += BuddyStrTemp;
			SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
			return;
		}
		BuddyStrTemp.Format(_T("Buddy sensor removed.\n"));
		BuddyStrOut += BuddyStrTemp;
		SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);
	}

	// destroy handles
	GoDestroy(Gosystem);
	GoDestroy(api);
	free(profileBuffer);

	/*BuddyStrTemp.Format(_T("\n"));
	BuddyStrOut += BuddyStrTemp;
	SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);*/

	m_buddyEdit.SetSel(-1, 0, FALSE);
	m_buddyEdit.SetFocus();

	//getchar();
	return;
}


void BuddySenSystem::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}


void BuddySenSystem::OnBnClickedBuddReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK);

	BuddyStrTemp.Format(_T("等待相机进行重连操作.....\n"));
	BuddyStrOut += BuddyStrTemp;
	SetDlgItemText(IDC_BUDDYEDIT, BuddyStrOut);

	SetUseBuddySysteam();
	SetStopCamera();
}
