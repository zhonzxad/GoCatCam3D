#include "stdafx.h"
#include "NewReceiveProfile.h"
#include "resource.h"


NewReceiveProfile::NewReceiveProfile()
{
}


NewReceiveProfile::~NewReceiveProfile()
{
}

void NewReceiveProfile::SetStartCamera()   //int argc, char** argv
{
	NewProTemp = NEwProOut = "0";
	NEwProOut = "开始接收线扫相机\r\n进入初始化接收表面轮廓程序\r\n";
	SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);

	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		NewProTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT,1020, NEwProOut);
		return;
	}

	// construct GoSystem object
	if ((status = GoSystem_Construct(&system, kNULL)) != kOK)
	{
		NewProTemp.Format(_T("Error: GoSystem_Construct:%d\r\n"), status);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
		return;
	}

	// Parse IP address into address data structure
	kIpAddress_Parse(&ipAddress, SENSOR_IP);

	// obtain GoSensor object by sensor IP address
	if ((status = GoSystem_FindSensorByIpAddress(system, &ipAddress, &sensor)) != kOK)
	{
		NewProTemp.Format(_T("Error: GoSystem_FindSensor:%d\n"), status);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
		return;
	}

	// create connection to GoSensor object
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		NewProTemp.Format(_T("Error: GoSensor_Connect:%d\n"), status);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
		return;
	}

	// enable sensor data channel
	if ((status = GoSystem_EnableData(system, kTRUE)) != kOK)
	{
		NewProTemp.Format(_T("Error: GoSensor_EnableData:%d\n"), status);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
		return;
	}

	// retrieve setup handle
	if ((setup = GoSensor_Setup(sensor)) == kNULL)
	{
		NewProTemp.Format(_T("Error: GoSensor_Setup: Invalid Handle\n"));
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
	}

	//在启动传感器之前检索轮廓点的总数
	if (GoSetup_UniformSpacingEnabled(setup))
	{
		//启用统一间距。该数字基于X Spacing设置
		profilePointCount = GoSetup_XSpacingCount(setup, GO_ROLE_MAIN);
	}
	else
	{
		//启用非均匀间距。最大数量取决于相机中使用的列数。
		profilePointCount = GoSetup_FrontCameraWidth(setup, GO_ROLE_MAIN);
	}

	if ((profileBuffer = (ProfilePoint *)malloc(profilePointCount * sizeof(ProfilePoint))) == kNULL)
	{
		NewProTemp.Format(_T("Error: Cannot allocate profileData, %d points\n"), profilePointCount);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
		return;
	}

	// start Gocator sensor
	if ((status = GoSystem_Start(system)) != kOK)
	{
		NewProTemp.Format(_T("Error: GoSensor_Start:%d\r\n"), status);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
		return;
	}

	if (GoSystem_ReceiveData(system, &dataset, RECEIVE_TIMEOUT) == kOK)
	{
		NewProTemp.Format(_T("Data message received:\r\n"));
		NEwProOut += NewProTemp;
		NewProTemp.Format(_T("Dataset count: %u\r\n"), GoDataSet_Count(dataset));
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);

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

				NewProTemp.Format(_T("Stamp Message batch count: %u\r\n"), GoStampMsg_Count(stampMsg));
				NEwProOut += NewProTemp;
				SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);

				for (j = 0; j < GoStampMsg_Count(stampMsg); ++j)
				{
					stamp = GoStampMsg_At(stampMsg, j);
					NewProTemp.Format(_T("  Stamp Message batch count: %u\r\n"), GoStampMsg_Count(stampMsg));
					NEwProOut += NewProTemp;
					NewProTemp.Format(_T("  Timestamp: %llu\r\n"), stamp->timestamp);
					NEwProOut += NewProTemp;
					NewProTemp.Format(_T("  Encoder: %lld\r\n"), stamp->encoder);
					NEwProOut += NewProTemp;
					NewProTemp.Format(_T("  Frame index: %llu\r\n"), stamp->frameIndex);
					NEwProOut += NewProTemp;
					SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
				}
			}
			break;
			case GO_DATA_MESSAGE_TYPE_UNIFORM_PROFILE:
			{
				GoResampledProfileMsg profileMsg = dataObj;

				NewProTemp.Format(_T("Resampled Profile Message batch count: %u\r\n"), GoResampledProfileMsg_Count(profileMsg));
				NEwProOut += NewProTemp;
				SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);

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
					NewProTemp.Format(_T("  Profile Valid Point %d out of max %d\r\n"), validPointCount, profilePointCount);
					NEwProOut += NewProTemp;
					SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
				}
			}
			break;
			case GO_DATA_MESSAGE_TYPE_PROFILE_POINT_CLOUD: // Note this is NON resampled profile            
			{
				GoProfileMsg profileMsg = dataObj;

				NewProTemp.Format(_T("Profile Message batch count: %u\r\n"), GoProfileMsg_Count(profileMsg));
				NEwProOut += NewProTemp;
				SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);

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
					NewProTemp.Format(_T("  Profile Valid Point %d out of max %d\r\n"), validPointCount, profilePointCount);
					NEwProOut += NewProTemp;
					SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
				}
			}
			break;
			case GO_DATA_MESSAGE_TYPE_PROFILE_INTENSITY:
			{
				unsigned int validPointCount = 0;
				GoProfileIntensityMsg intensityMsg = dataObj;

				NewProTemp.Format(_T("Intensity Message batch count: %u\r\n"), GoProfileIntensityMsg_Count(intensityMsg));
				NEwProOut += NewProTemp;
				SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);

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
		NewProTemp.Format(_T("Error: No data received during the waiting period\r\n"));
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
	}

	return;
}

void NewReceiveProfile::SetStopCamera()
{
	// stop Gocator sensor
	if ((status = GoSystem_Stop(system)) != kOK)
	{
		NewProTemp.Format(_T("Error: GoSensor_Stop:%d\r\n"), status);
		NEwProOut += NewProTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);
		return;
	}

	// destroy handles
	GoDestroy(system);
	GoDestroy(api);
	free(profileBuffer);
}

void NewReceiveProfile::ReflushCamer()
{
	// TODO: 在此添加控件通知处理程序代码
	

	NewProTemp.Format(_T("等待相机进行重连操作....\r\n"));
	NEwProOut += NewProTemp;

	NewProTemp.Format(_T("重连成功....\r\n"));
	NEwProOut += NewProTemp;
	SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, NEwProOut);

	SetStartCamera();
	SetStopCamera();
}