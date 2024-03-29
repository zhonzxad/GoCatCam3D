// ReceiveFromProfile.cpp: 实现文件
//

#include "stdafx.h"
#include "GocatorCamera.h"
#include "ReceiveFromProfile.h"
#include "afxdialogex.h"

#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "CCLogWnd.h"


#define SENSOR_IP         "192.168.1.10"

#pragma warning(disable:4002)
#pragma warning(disable:26495)

CString profileTemp;
CString profileOut;

kStatus SetupProfilePositionTool(GoTools tools, GoSetup setup, kChar* toolName, k32s measId, k32s featId, kChar* featName);
kStatus SetupFeatureDimensionTool(GoTools tools, GoSetup setup, kChar* toolName, k32s toolId, k32s featInId0, k32s featInId1);

CLogWnd *m_RecFroProLogDlg;
CEdit m_profiEdit;

// ReceiveFromProfile 对话框
IMPLEMENT_DYNAMIC(ReceiveFromProfile, CDialogEx)

ReceiveFromProfile::ReceiveFromProfile(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RECFROMPROFILEMODE, pParent)
{

}

ReceiveFromProfile::~ReceiveFromProfile()
{
}

void ReceiveFromProfile::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROFILEMODEEDIT, m_profiEdit);
}

BEGIN_MESSAGE_MAP(ReceiveFromProfile, CDialogEx)
	ON_BN_CLICKED(IDOK, &ReceiveFromProfile::OnBnClickedOk)
	ON_BN_CLICKED(IDC_PRO_REFLESH, &ReceiveFromProfile::OnBnClickedProReflesh)
END_MESSAGE_MAP()

// ReceiveFromProfile 消息处理程序

kStatus SetupProfilePositionTool(GoTools tools, GoSetup setup, kChar* toolName, k32s measId, k32s featId, kChar* featName)
{
	kStatus status = kOK;

	GoProfilePosition        profilePositionTool = kNULL;
	GoProfilePositionZ        profileMeasurementTopZ = kNULL;
	GoProfileFeature        profileFeatureTop = kNULL;
	GoProfileRegion            regionTop = kNULL;
	GoProfilePositionPoint    outFeaturePoint = kNULL;

	profileTemp.Format(_T("正确进入异步接受表面轮廓数据中（异步数据中）:\r\n"));
	profileOut += profileTemp;
	SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	m_profiEdit.SetSel(-1, 0, FALSE);
	m_profiEdit.SetFocus();
	m_profiEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	//添加Profile Position工具，检索工具句柄
	if ((status = GoTools_AddToolByName(tools, "ProfilePosition", &profilePositionTool)) != kOK)
	{
		profileTemp.Format(_T("Error: GoTools_AddToolByName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((status = GoTool_SetName(profilePositionTool, toolName)) != kOK)
	{
		profileTemp.Format(_T("Error: GoTool_SetName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//设置功能输出
	if ((outFeaturePoint = GoProfilePosition_Point(profilePositionTool)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoProfilePosition_Point: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((status = GoFeature_Enable(outFeaturePoint, kTRUE)) != kOK)
	{
		profileTemp.Format(_T("Error: GoFeature_Enable: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((status = GoFeature_SetId(outFeaturePoint, featId)) != kOK)
	{
		profileTemp.Format(_T("Error: GoFeature_SetId: %d\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((status = GoFeature_SetName(outFeaturePoint, featName)) != kOK)
	{
		profileTemp.Format(_T("Error: GoFeature_SetName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//设置Z测量输出//为ProfilePosition工具添加Z测量
	if ((profileMeasurementTopZ = GoProfilePosition_ZMeasurement(profilePositionTool)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoProfilePosition_ZMeasurement: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//启用zprofileMeasurementTop
	if ((status = GoMeasurement_Enable(profileMeasurementTopZ, kTRUE)) != kOK)
	{
		profileTemp.Format(_T("Error: GoMeasurement_Enable: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//设置zprofileMeasurementTop的度量标识
	if ((status = GoMeasurement_SetId(profileMeasurementTopZ, measId)) != kOK)
	{
		profileTemp.Format(_T("Error: GoMeasurement_SetId: %d\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//设置zprofileMeasuermentTop的名称
	if ((status = GoMeasurement_SetName(profileMeasurementTopZ, "Profile Measurement Z")) != kOK)
	{
		profileTemp.Format(_T("Error: GoMeasurement_SetName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	}

	//将ProfilePosition功能设置为top
	if ((profileFeatureTop = GoProfilePosition_Feature(profilePositionTool)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoProfilePosition_Feature: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	}

	if ((status = GoProfileFeature_SetType(profileFeatureTop, GO_PROFILE_FEATURE_TYPE_MAX_Z)) != kOK)
	{
		profileTemp.Format(_T("Error: GoProfileFeature_SetType: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	}

	//设置ROI以填充整个活动区域
	if ((regionTop = GoProfileFeature_Region(profileFeatureTop)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoProfileFeature_Region: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	}
	if ((status = GoProfileRegion_SetX(regionTop, GoSetup_TransformedDataRegionX(setup, GO_ROLE_MAIN))) != kOK)
	{
		profileTemp.Format(_T("Error: GoProfileRegion_SetX: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	}
	if ((status = GoProfileRegion_SetZ(regionTop, GoSetup_TransformedDataRegionZ(setup, GO_ROLE_MAIN))) != kOK)
	{
		profileTemp.Format(_T("Error: GoProfileRegion_SetZ: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	}
	if ((status = GoProfileRegion_SetHeight(regionTop, GoSetup_TransformedDataRegionHeight(setup, GO_ROLE_MAIN))) != kOK)
	{
		profileTemp.Format(_T("Error: GoTools_AddToolByName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		printf("Error: GoProfileRegion_SetHeight: %d\r\n", status);
	}
	if ((status = GoProfileRegion_SetWidth(regionTop, GoSetup_TransformedDataRegionWidth(setup, GO_ROLE_MAIN))) != kOK)
	{
		profileTemp.Format(_T("Error: GoProfileRegion_SetWidth: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
	}

	m_profiEdit.SetSel(-1, 0, FALSE);
	m_profiEdit.SetFocus();

	m_profiEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	return kOK;
}
kStatus SetupFeatureDimensionTool(GoTools tools, GoSetup setup, kChar* toolName, k32s toolId, k32s featInId0, k32s featInId1)
{
	kStatus status = kOK;

	GoExtTool           extensibleTool = kNULL;
	GoMeasurement       profileMeasurementDistance = kNULL;
	GoProfileFeature    profileFeatureTop = kNULL;
	GoProfileRegion     regionTop = kNULL;
	GoExtParamFeature    paramInFeat0 = kNULL;
	GoExtParamFeature    paramInFeat1 = kNULL;

	m_profiEdit.SetSel(-1, 0, FALSE);
	m_profiEdit.SetFocus();
	m_profiEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	//添加Profile Position工具，检索工具句柄
	if ((status = GoTools_AddToolByName(tools, "FeatureDimension", &extensibleTool)) != kOK)
	{
		profileTemp.Format(_T("Error: GoTools_AddToolByName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((status = GoTool_SetName(extensibleTool, toolName)) != kOK)
	{
		profileTemp.Format(_T("Error: GoTool_SetName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//为可扩展工具添加“距离”测量
	if ((profileMeasurementDistance = GoExtTool_FindMeasurementByName(extensibleTool, "Distance")) == kNULL)
	{
		profileTemp.Format(_T("Error: GoExtTool_FindMeasurementByName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//启用距离测量
	if ((status = GoMeasurement_Enable(profileMeasurementDistance, kTRUE)) != kOK)
	{
		profileTemp.Format(_T("Error: GoMeasurement_Enable: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//设置距离测量的测量ID
	if ((status = GoMeasurement_SetId(profileMeasurementDistance, toolId)) != kOK)
	{
		profileTemp.Format(_T("Error: GoMeasurement_SetId: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//设置距离测量的名称
	if ((status = GoMeasurement_SetName(profileMeasurementDistance, "Profile Measurement Z")) != kOK)
	{
		profileTemp.Format(_T("Error: GoMeasurement_SetName: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	//设置功能输入
	if ((paramInFeat0 = GoExtTool_ParameterAt(extensibleTool, 0)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoExtTool_ParameterAt: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((status = GoExtParamFeature_SetFeatureId(paramInFeat0, featInId0)) != kOK)
	{
		profileTemp.Format(_T("Error: GoExtParamFeature_SetFeatureId:%d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((paramInFeat1 = GoExtTool_ParameterAt(extensibleTool, 1)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoExtTool_ParameterAt: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	if ((status = GoExtParamFeature_SetFeatureId(paramInFeat1, featInId1)) != kOK)
	{
		profileTemp.Format(_T("Error: GoExtParamFeature_SetFeatureId:%d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText((HWND)IDC_PROFILEMODEEDIT, 1020, profileOut);
		return status;
	}

	m_profiEdit.SetSel(-1, 0, FALSE);
	m_profiEdit.SetFocus();

	return kOK;
}

void ReceiveFromProfile::SetUseReceiveFromProfile()  //int argc, char **argv
{
	profileTemp = profileOut = "0";
	profileOut = "进入采集表面轮廓程序失败！！\r\n";
	SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);

	m_profiEdit.SetSel(-1, 0, FALSE);
	m_profiEdit.SetFocus();
	m_profiEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	
	MessageBox(_T("无效的操作模式，请停止相机拍摄!!!\r\n"), _T("模式不匹配"), MB_ICONEXCLAMATION | MB_OK);

	if ((status = GoSdk_Construct(&api)) != kOK)
	{
		profileTemp.Format(_T("Error: GoSdk_Construct:%d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	if ((status = GoSystem_Construct(&Gosystem, kNULL)) != kOK)
	{
		profileTemp.Format(_T("Error: GoSystem_Construct: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	if ((status = kIpAddress_Parse(&ipAddress, SENSOR_IP)) != kOK)
	{
		profileTemp.Format(_T("Error: kIpAddress_Parse: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	if ((status = GoSystem_FindSensorByIpAddress(Gosystem, &ipAddress, &sensor)) != kOK)
	{
		//profileTemp.Format(_T("Error: GoSystem_FindSensorByIpAddress: %d\r\n", status));
		profileTemp.Format(_T("无效的IP地址配置方式\r\n"));
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	//创建与GoSensor对象的连接
	if ((status = GoSensor_Connect(sensor)) != kOK)
	{
		profileTemp.Format(_T("Error: GoSensor_Connect: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	//启用传感器数据通道
	if ((status = GoSystem_EnableData(Gosystem, kTRUE)) != kOK)
	{
		profileTemp.Format(_T("Error: GoSensor_EnableData: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	//检索设置句柄
	if ((setup = GoSensor_Setup(sensor)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoSensor_Setup: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	//检索工具句柄
	if ((tools = GoSensor_Tools(sensor)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoSensor_Tools: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	//在传感器上设置三个工具：2个轮廓位置和1个特征尺寸
	if ((status = SetupProfilePositionTool(tools, setup, "profile position test 0", 10, 20, "out feature 0")) != kOK)
	{
		profileTemp.Format(_T("error: setupprofilepositiontool: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}
	if ((status = SetupProfilePositionTool(tools, setup, "profile position test 1", 11, 21, "out feature 0")) != kOK)
	{
		profileTemp.Format(_T("error: setupprofilepositiontool: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}
	if ((status = SetupFeatureDimensionTool(tools, setup, "feature dimension test", 12, 20, 21)) != kOK)
	{
		profileTemp.Format(_T("error: setupfeaturedimensiontool: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	//为测量工具启用以太网输出
	if ((outputModule = GoSensor_Output(sensor)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoSensor_Output: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}
	if ((ethernetOutput = GoOutput_Ethernet(outputModule)) == kNULL)
	{
		profileTemp.Format(_T("Error: GoOutput_Ethernet: Invalid Handle\r\n"));
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}
	if ((status = GoEthernet_ClearAllSources(ethernetOutput)) != kOK)
	{
		profileTemp.Format(_T("Error: GoEthernet_ClearAllSources: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	//输出三个测量值
	if ((status = GoEthernet_AddSource(ethernetOutput, GO_OUTPUT_SOURCE_MEASUREMENT, 10)) != kOK)
	{
		profileTemp.Format(_T("Error: GoEthernet_AddSource: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	if ((status = GoEthernet_AddSource(ethernetOutput, GO_OUTPUT_SOURCE_MEASUREMENT, 11)) != kOK)
	{
		profileTemp.Format(_T("Error: GoEthernet_AddSource: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}

	if ((status = GoEthernet_AddSource(ethernetOutput, GO_OUTPUT_SOURCE_MEASUREMENT, 12)) != kOK)
	{
		profileTemp.Format(_T("Error: GoEthernet_AddSource: %d\r\n"), status);
		profileOut += profileTemp;
		SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
		return;
	}
	// refer to ReceiveMeasurement.c for receiving of the measurement data.

	m_profiEdit.SetSel(-1, 0, FALSE);
	m_profiEdit.SetFocus();

	m_profiEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	return;
}

void ReceiveFromProfile::SetStopCamera()
{
	// destroy handles
	GoDestroy(Gosystem);
	GoDestroy(api);

	profileTemp.Format(_T("正在释放已创建的相机对象，请等待3秒之后再移动窗体。\r\n"));
	profileOut += profileTemp;
	SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);
	m_profiEdit.SetSel(-1, 0, FALSE);
	m_profiEdit.SetFocus();

	m_profiEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	//getchar();
}

void ReceiveFromProfile::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	MessageBox(_T("无效确认操作！\r\n"), _T("GoCator相机操作"), MB_OK | MB_ICONEXCLAMATION);
}

void ReceiveFromProfile::OnBnClickedProReflesh()
{
	// TODO: 在此添加控件通知处理程序代码
	MessageBox(_T("刷新相机需要重新创建配置\r\n期间请不要移动界面\r\n等待相机完成操作\r\n等待2到3秒"), _T("GoCator相机提示"), MB_OK);

	profileTemp.Format(_T("等待相机进行重连操作....\r\n"));
	profileOut += profileTemp;
	SetDlgItemText(IDC_PROFILEMODEEDIT, profileOut);

	m_profiEdit.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	/*SetUseReceiveFromProfile();
	SetStopCamera();*/

	m_newMode.SetStartCamera();
	m_newMode.SetStopCamera();
}
