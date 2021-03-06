#define _CRT_RAND_S
#include <time.h>
#include <tchar.h>
#include <utility.h>
#include <config.h>
#include <libfunc.h>
#include <win32/pim_ime.h>
#include <shlobj.h>

#define MAX_CAND_HINT_SIZE			60		//提示最大长度
#define MAX_CAND_HINT_NUM			100		//提示最大条数

//const TCHAR CS_ConfigFile[]			= TEXT("unispim6\\ini\\自动更新.ini");

const TCHAR CS_HintFile[]			= TEXT("unispim6\\ini\\提示.ini");
const TCHAR CS_Theme_Path[]			= TEXT("unispim6\\theme\\");
const TCHAR CS_Theme_File[]			= TEXT("theme.ini");
const TCHAR CS_Theme_DeletedFile[]  = TEXT("theme.deleted.ini");
const TCHAR CS_Theme_Section[]      = TEXT("Unispim Theme");
const TCHAR CS_Theme_Name[]			= TEXT("主题名称");
const TCHAR CS_Theme_DefaultPath[]  = TEXT("默认主题");

//备份和恢复相关常量定义
const int CN_ReservedBackups		= 5;

//自动更新.ini
//const TCHAR CS_INI_UpdateInfo[]     = TEXT("更新信息");
//const TCHAR CS_INI_LibUpdateMode[]  = TEXT("LibUpdateMode");
//const TCHAR CS_INI_AppUpdateMode[]  = TEXT("AppUpdateMode");
//const TCHAR CS_INI_UserID[]         = TEXT("UserID");
//const TCHAR CS_INI_WordlibVersion[] = TEXT("LibVersion");
//const TCHAR CS_INI_AppVersion[]     = TEXT("AppVersion");
//const TCHAR CS_INI_LatestDate[]     = TEXT("LatestDate");
//const TCHAR CS_INI_CheckTime[]      = TEXT("CheckTime");
//const TCHAR CS_INI_BalanceTime[]    = TEXT("BalanceTime");
//const TCHAR CS_INI_URLMaxSize[]     = TEXT("URLMaxSize");

//默认值
const int CN_Default_WL_Version		= 0;
const int CN_Default_URLMaxSize		= 256;
const int CN_Default_BalanceTime	= 30;

//更新方式
const int CN_Update_Auto			= 1;  //自动
const int CN_Update_Prompt			= 2;  //提示
const int CN_Update_Manual			= 3;  //手动

typedef struct tagThemeInfo
{
	TCHAR ThemeName[MAX_THEME_NAME_LENGTH];
	TCHAR FolderName[MAX_PATH];
} stThemeInfo;

//客户端配置信息//!作为一个结构体，可以被使用者写入和引用
typedef struct TagLocalConfig
{
	int LibUpdateMode;				//词库更新方式,1自动; 2提示;3手动
	int AppUpdateMode;				//程序更新方式,1自动; 2提示;3手动
	TCHAR UserID[MAX_PATH];			//用户ID
	int WordlibVersion;				//词库版本
	TCHAR AppVersion[MAX_PATH];		//程序版本
	SYSTEMTIME LastDate;			//最后一个检查日期
	SYSTEMTIME CheckTime;			//当天检查时间
	int balanceTime;				//负载均衡时间(分钟)
	int URLMaxSize;					//最大上传词条长度
}stLocalConfig;

//更新信息相关
stLocalConfig localconfig;			//全局变量，用于对客户端配置信息进行处理

static int HasChineseFont = 0;
static int HasEnglishFont = 0;


//检查本地配置信息是否正常，避免出现手工修改导致程序异常
void CheckLocalConfig()
{
	HRESULT hr;
	GUID guid;

	//检查词库更新方式
	if (localconfig.LibUpdateMode < CN_Update_Auto || localconfig.LibUpdateMode > CN_Update_Manual)
		localconfig.LibUpdateMode = CN_Update_Auto;

	//检查程序更新方式
	if (localconfig.AppUpdateMode < CN_Update_Auto || localconfig.AppUpdateMode > CN_Update_Manual)
		localconfig.AppUpdateMode = CN_Update_Manual;

	//检查唯一ID是否有效
	TrimString(localconfig.UserID);

	if (32 != _tcslen(localconfig.UserID))
	{
		hr = CoCreateGuid(&guid);
		if (S_OK == hr)
		{
			_stprintf_s(localconfig.UserID,
						MAX_PATH, 
						TEXT("%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X"),
						guid.Data1,
						guid.Data2,
						guid.Data3,
						guid.Data4[0],
						guid.Data4[1],
						guid.Data4[2],
						guid.Data4[3],
						guid.Data4[4],
						guid.Data4[5],
						guid.Data4[6],
						guid.Data4[7]);
		}
	}

	//检查词库版本是否有效
	if (localconfig.WordlibVersion < CN_Default_WL_Version)
		localconfig.WordlibVersion = CN_Default_WL_Version;

	//检查负载均衡的时间是否有效
	if (localconfig.balanceTime < 0 || localconfig.balanceTime > 24 * 60)
		localconfig.balanceTime = CN_Default_BalanceTime;

	//检查上传词的个数是否有效
	if (localconfig.URLMaxSize < 0 || localconfig.URLMaxSize > 1000)
		localconfig.URLMaxSize = CN_Default_URLMaxSize;
}

//读自动更新.ini文件给客户端配置信息stLocalConfig赋值
//int LoadLocalConfig()
//{
//	TCHAR AllAppDir[MAX_PATH], ConfigFile[MAX_PATH], OtherInfo[MAX_PATH], *p;
//	HANDLE hfile;
//
//	//文件打开失败时的值
//	localconfig.LibUpdateMode = CN_Update_Manual;
//	localconfig.AppUpdateMode = CN_Update_Manual;
//
//	ZeroMemory(&localconfig.CheckTime, sizeof(SYSTEMTIME));
//	ZeroMemory(&localconfig.LastDate,  sizeof(SYSTEMTIME));
//
//	GetAllUserAppDirectory(AllAppDir);
//	CombineDirAndFile(AllAppDir,CS_ConfigFile,ConfigFile);
//
//	hfile = CreateFile((ConfigFile),
//						GENERIC_READ,
//						FILE_SHARE_READ,
//						0,
//						OPEN_EXISTING,
//						FILE_ATTRIBUTE_NORMAL,
//						0);
//
//	if (hfile)
//		CloseHandle(hfile);
//	else
//		return 0;
//
//	localconfig.AppUpdateMode	= CN_Update_Manual;
//	localconfig.LibUpdateMode	= GetPrivateProfileInt(CS_INI_UpdateInfo, CS_INI_LibUpdateMode,  -1, ConfigFile);
//	localconfig.WordlibVersion	= GetPrivateProfileInt(CS_INI_UpdateInfo, CS_INI_WordlibVersion, -1, ConfigFile);//词库版本
//	localconfig.balanceTime		= GetPrivateProfileInt(CS_INI_UpdateInfo, CS_INI_BalanceTime,	 -1, ConfigFile);
//	localconfig.URLMaxSize		= GetPrivateProfileInt(CS_INI_UpdateInfo, CS_INI_URLMaxSize,	 -1, ConfigFile);
//
//	GetPrivateProfileString(CS_INI_UpdateInfo, CS_INI_UserID,     TEXT(""), localconfig.UserID,     MAX_PATH, ConfigFile); //用户ID
//	GetPrivateProfileString(CS_INI_UpdateInfo, CS_INI_AppVersion, TEXT(""), localconfig.AppVersion, MAX_PATH, ConfigFile); //程序版本
//	GetPrivateProfileString(CS_INI_UpdateInfo, CS_INI_LatestDate, TEXT(""), OtherInfo,				MAX_PATH, ConfigFile); //LastDate:%04u-%02u-%02u
//
//	//LastDate
//	p = OtherInfo;
//	while(1)
//	{
//		localconfig.LastDate.wYear = _ttoi(p);
//		if (!(p = _tcsstr(p, TEXT("-"))))
//			break;
//
//		localconfig.LastDate.wMonth = _ttoi(++p);
//		if (!(p = _tcsstr(p, TEXT("-"))))
//			break;
//
//		localconfig.LastDate.wDay = _ttoi(++p);
//		break;
//	}
//
//	//CheckTime
//	GetPrivateProfileString(CS_INI_UpdateInfo, CS_INI_CheckTime, TEXT(""), OtherInfo, MAX_PATH, ConfigFile); //CheckTime:%04u-%02u-%02u %u:%u:%u
//	p = OtherInfo;
//	while(1)
//	{
//		localconfig.CheckTime.wYear = _ttoi(p);
//		if (!(p = _tcsstr(p, TEXT("-"))))
//			break;
//
//		localconfig.CheckTime.wMonth = _ttoi(++p);
//		if (!(p = _tcsstr(p, TEXT("-"))))
//			break;
//
//		localconfig.CheckTime.wDay = _ttoi(++p);
//		if (!(p = _tcsstr(p, TEXT(" "))))
//			break;
//
//		localconfig.CheckTime.wHour = _ttoi(++p);
//		if (!(p = _tcsstr(p, TEXT(":"))))
//			break;
//
//		localconfig.CheckTime.wMinute = _ttoi(++p);
//		if (!(p = _tcsstr(p, TEXT(":"))))
//			break;
//
//		localconfig.CheckTime.wSecond = _ttoi(++p);
//		break;
//	}
//
//	CheckLocalConfig();
//
//	return 1;
//};

//备份用户数据到指定目录
int BackupReg(const TCHAR *TodayBackDir)
{
	//获取RegEdit.exe的全路径文件名(GetWindowPath + 'regedit.exe')
	TCHAR dir[MAX_PATH], RegTool[MAX_PATH], BackFile[MAX_PATH];
	int has_uac = (GetWindowVersion() >= 0x0600);

	//vista下调用regedit.exe会触发UAC提示，改为使用reg.exe
	if (has_uac)
	{
		SHGetFolderPath(0, CSIDL_SYSTEM, 0, 0, dir);
		CombineDirAndFile(dir, TEXT("reg.exe"), RegTool);
	}
	else
	{
		SHGetFolderPath(0, CSIDL_WINDOWS, 0, 0, dir);
		CombineDirAndFile(dir, TEXT("regedit.exe"), RegTool);
	}

	if (!FileExists(RegTool))//windows\regedit.exe
		return 0;

	_tcscpy_s(dir, MAX_PATH, TodayBackDir);
	CombineDirAndFile(dir, TEXT("Config.dat"), BackFile);//BackFile = backDir\Config.dat
	if (FileExists(BackFile))
	{
		//设置为存档文件，再删除(否则可能删不掉)
		SetFileAttributes(BackFile, FILE_ATTRIBUTE_ARCHIVE);
		DeleteFile(BackFile);
	}

	if (has_uac)
	{
		//在regedit.exe中导出Config.dat，不要显示窗口，成功返回TRue
		_tcscpy_s(dir, MAX_PATH, TEXT(" EXPORT "));
		_tcscat_s(dir, MAX_PATH, REG_FULL_CONFIG_KEY_NAME);
		_tcscat_s(dir, MAX_PATH, TEXT(" "));
		_tcscat_s(dir, MAX_PATH, BackFile);
	}
	else
	{
		//在regedit.exe中导出Config.dat，不要显示窗口，成功返回TRue
		_tcscpy_s(dir, MAX_PATH, TEXT(" /e "));
		_tcscat_s(dir, MAX_PATH, BackFile);
		_tcscat_s(dir, MAX_PATH, TEXT(" "));
		_tcscat_s(dir, MAX_PATH, REG_FULL_CONFIG_KEY_NAME);
	}

	//将注册表信息导出到Config.dat
	if ((INT_PTR)ShellExecute(0, TEXT("open"), RegTool, dir, TEXT(""), SW_HIDE) > 32)
		return 1;

	return 0;
}

TCHAR *left(TCHAR *dst, const TCHAR *src, int n)
{
	const TCHAR *p = src;
	TCHAR *q = dst;
	int len = (int)_tcslen(src);

	if (n > len) 
		n = len;

	while(n--) 
		*(q++) = *(p++);

	*(q++)='\0';

	return dst;
}

TCHAR *right(TCHAR *dst,const TCHAR *src, int n)
{
	const TCHAR *p = src;
	TCHAR *q = dst;
	int len = (int)_tcslen(src);

	if (n > len) 
		n = len;

	p += (len-n);

	while (*(q++) = *(p++));

	return dst;
}

int SortString(const TCHAR *str1,const TCHAR *str2)
{
	return _tcsicmp(str1, str2);
}

//删除目录(..\\)下的所有文件（夹）
void DeleteDirectory(const TCHAR *Dir)// 以\\结束
{
	//struct  Stat statbuf;
	WIN32_FIND_DATA FindFileData;
	HANDLE hfind;
	TCHAR Findpath[MAX_PATH],DeletePath[MAX_PATH];

	_tcscpy_s(Findpath, MAX_PATH, Dir);

	//必须加   *.*,保证输入路径以\\结束
	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(Dir))
		return;

	_tcscat_s(Findpath, MAX_PATH, TEXT("*.*"));
	if (!(hfind = FindFirstFile(Findpath, &FindFileData)))
		return;

	do
	{
		if (0 == _tcsicmp(FindFileData.cFileName, TEXT(".")) || 
			0 == _tcsicmp(FindFileData.cFileName, TEXT("..")))
			continue; //不搜索.  和   ..

		_tcscpy_s(DeletePath,MAX_PATH,Dir);
		_tcscat_s(DeletePath,MAX_PATH,FindFileData.cFileName);

		//文件直接删除
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			DeleteFile(DeletePath);
		else
		{
			_tcscat_s(DeletePath, MAX_PATH, TEXT("\\"));
			DeleteDirectory(DeletePath);
		}
	}while(FindNextFile(hfind, &FindFileData));

	FindClose(hfind);

	RemoveDirectory(Dir);
}

//对备份目录BackupDir下的unispim6的备份文件夹按名称进行排序，然后将时间最早的备份删除掉
//输入参数： BackupDir              ----- 备份目录
//           iNum                   ----- 允许保留的最大备份数
void DeleteOverdueDir(const TCHAR *BackupDir, int iNum)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hfind;
	TCHAR TodayBackDir[MAX_PATH], Findpath[MAX_PATH], BackList[50][MAX_FILE_NAME_LENGTH];
	int i, iSize = 0;

	_tcscpy_s(TodayBackDir, MAX_PATH, BackupDir);
	if (BackupDir[_tcslen(BackupDir) - 1] != '\\')
		_tcscat_s(TodayBackDir, MAX_PATH, TEXT("\\"));

	//组建数组
	_tcscpy_s(Findpath, MAX_PATH, TodayBackDir);
	_tcscat_s(Findpath, MAX_PATH, TEXT("*.*"));
	if (!(hfind = FindFirstFile(Findpath, &FindFileData)))
		return;

	do
	{
		if (0 != _tcsicmp(FindFileData.cFileName, TEXT(".")) && 
			0 != _tcsicmp(FindFileData.cFileName, TEXT("..")) && 
			(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			//取前9位与"unispim6_"比较
			left(TodayBackDir, FindFileData.cFileName, 9);

			if (0 == _tcsicmp(TodayBackDir,TEXT("unispim6_")))
				_tcscpy_s(BackList[iSize++], MAX_FILE_NAME_LENGTH, FindFileData.cFileName);
		}
	}while(FindNextFile(hfind, &FindFileData));

	FindClose(hfind);

	//排序
	qsort(BackList,iSize, sizeof(BackList[0]), SortString);

	iSize -= iNum;
	for (i = 0; i < iSize; ++i)
	{
		CombineDirAndFile(BackupDir, BackList[i], Findpath);
		_tcscat_s(Findpath, MAX_PATH, TEXT("\\"));
		DeleteDirectory(Findpath);
	}
}

//拷贝目录，须保证输入路径以\\结束
int CopyDirectory(const TCHAR * fromDir, const TCHAR * toDir, BOOL bCoverIfFileExists)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hfind;
	TCHAR Findpath[MAX_PATH],Topath[MAX_PATH],FromPath[MAX_PATH];

	_tcscpy_s(Findpath,MAX_PATH,fromDir);

	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(fromDir))
		return 0;

	if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(toDir))
	{
		if(!CreateDirectory(toDir, NULL))
			return 0;
	}

	_tcscat_s(Findpath,MAX_PATH, TEXT("*.*"));
	if(!(hfind = FindFirstFile(Findpath, &FindFileData)))
		return 0;

	do
	{
		if (0 == _tcsicmp(FindFileData.cFileName, TEXT(".")) || 
			0 == _tcsicmp(FindFileData.cFileName, TEXT("..")))
			continue;//不搜索.  和   ..

		_tcscpy_s(FromPath, MAX_PATH, fromDir);
		_tcscat_s(FromPath, MAX_PATH, FindFileData.cFileName);

		_tcscpy_s(Topath, MAX_PATH, toDir);
		_tcscat_s(Topath, MAX_PATH, TEXT("\\"));
		_tcscat_s(Topath, MAX_PATH, FindFileData.cFileName);

		//文件直接拷贝
		if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			CopyFile(FromPath,Topath,!bCoverIfFileExists);
		else
		{
			_tcscat_s(FromPath, MAX_PATH, TEXT("\\"));
			_tcscat_s(Topath, MAX_PATH, TEXT("\\"));

			CopyDirectory(FromPath, Topath, bCoverIfFileExists);
		}
	}while(FindNextFile(hfind, &FindFileData));

	FindClose(hfind);

	return 1;
}

//备份用户数据到指定目录
int BackupUserData(const TCHAR *BackupDir)
{
	BOOL bret = FALSE;
	TCHAR TodayBackDir[MAX_PATH], NowDate[0x20], CurUsePath[MAX_PATH], BackUsePath[MAX_PATH];
	int iYear, iMonth, iDay, iHour, iMinute, iSecond, iMsecond;

	//获取当天的备份目录
	_tcscpy_s(TodayBackDir, MAX_PATH, BackupDir);
	if (0 == _tcsicmp(TodayBackDir,TEXT("")))
		return 0;

	if (BackupDir[_tcslen(BackupDir) - 1] != '\\')
		_tcscat_s(TodayBackDir, MAX_PATH, TEXT("\\"));

	if (!FileExists(TodayBackDir))
		return 0;

	//"lujing\unispim6_20070426"
	_tcscat_s(TodayBackDir, MAX_PATH, TEXT("unispim6_"));
	GetTimeValue(&iYear, &iMonth, &iDay, &iHour, &iMinute, &iSecond, &iMsecond);
	_stprintf_s(NowDate, _SizeOf(NowDate), TEXT("%d%02d%02d"), iYear, iMonth, iDay);
	_tcscat_s( TodayBackDir,MAX_PATH,NowDate);

	if (FileExists(TodayBackDir))
		return 0;

	if (0 == CreateDirectory(TodayBackDir, NULL))
		return 0;

	//创建备份当前用户数据目录：需要保存到"lujing\unispim6_20070426\CurrentUser"
	_tcscpy_s(BackUsePath, MAX_PATH, TodayBackDir);
	if (BackUsePath[(int)_tcslen(BackUsePath) - 1] != '\\')
		_tcscat_s(BackUsePath,MAX_PATH, TEXT("\\"));

	_tcscat_s(BackUsePath, MAX_PATH, TEXT("CurrentUser"));
	if (!FileExists(BackUsePath) && 0 == CreateDirectory(BackUsePath,NULL))
		return 0;

	//copy目录并且备份注册表
	//将GetCurUserPath + "unispim6\"路径下的文件（夹）备份到 BackUsePath
	GetUserAppDirectory(CurUsePath);
	if (CurUsePath[_tcslen(CurUsePath) - 1] != '\\')
		_tcscat_s(CurUsePath, MAX_PATH, TEXT("\\"));

	_tcscat_s(CurUsePath, MAX_PATH, TEXT("unispim6\\"));

	if (CopyDirectory(CurUsePath, BackUsePath, TRUE) && BackupReg(TodayBackDir))
	{
		//删除5天之外的备份目录
		DeleteOverdueDir(BackupDir, CN_ReservedBackups);
		return 1;
	}

	return 0;
}

int SortByThemeName(const stThemeInfo *Theme1,const stThemeInfo *Theme2)
{
	if (0 == _tcsicmp(Theme1->ThemeName, CS_Theme_DefaultPath))
		return -1;

	if (0 == _tcsicmp(Theme2->ThemeName, CS_Theme_DefaultPath))
		return 1;

	return (_tcsicmp(Theme1->ThemeName, Theme2->ThemeName));
}

int ThemeExist(stThemeInfo themeInfo,stThemeInfo pThemenames[MAX_THEME_COUNT],int iSize)
{
	int i;

	for(i = 0; i < iSize; ++i)
	{
		if (0 == _tcsicmp(pThemenames[i].ThemeName, themeInfo.ThemeName))
			return i;
	}

	return -1;
}

//获得theme目录
TCHAR* GetThemePath(TCHAR pDefaultthemePath[MAX_PATH])
{
	//获得所有用户的Application目录
	TCHAR AllAppDir[MAX_PATH];

	GetAllUserAppDirectory(AllAppDir);
	CombineDirAndFile(AllAppDir, CS_Theme_Path, pDefaultthemePath);

	return pDefaultthemePath;
}

//读取所有主题列表
int LoadThemes(stThemeInfo *ThemeList)
{
	TCHAR pExistFile[MAX_PATH], pImgFile[MAX_PATH], main_image_name[MAX_PATH];
	TCHAR Pathtmp[MAX_PATH], PathFold[MAX_PATH], pThemepath[MAX_PATH];
	stThemeInfo Infotmp;
	WIN32_FIND_DATA FindFileData;
	HANDLE hfind;
	int i = 0;

	//获得theme目录
	GetThemePath(pThemepath);
	_tcscpy_s(Pathtmp, MAX_PATH, pThemepath);
	_tcscat_s(Pathtmp, MAX_PATH, TEXT("*.*"));

	hfind = FindFirstFile(Pathtmp, &FindFileData);

	if (INVALID_HANDLE_VALUE == hfind)
		return 0;

	//查找主题子文件夹
	do
	{
		//find subfold
		if (0 == _tcsicmp(FindFileData.cFileName,TEXT(".")) || //"."当前目录
			0 == _tcsicmp(FindFileData.cFileName,TEXT(".."))|| //".."上一级目录
			!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) //文件
			continue;
		
		//处理主题子文件夹中的theme.ini
		_tcscpy_s(Infotmp.FolderName, MAX_PATH, FindFileData.cFileName);
		CombineDirAndFile(pThemepath, FindFileData.cFileName, PathFold);//子文件夹
		CombineDirAndFile(PathFold, CS_Theme_DeletedFile, pExistFile);	//all app data/themename/theme.deleted.ini

		if (FileExists(pExistFile))
		{
			DeleteDirectory(PathFold);
			continue;
		}

		CombineDirAndFile(PathFold, CS_Theme_File, pExistFile);	//pExistFile:all app data/themename/theme.ini
		if (!FileExists(pExistFile))
			continue;

		GetPrivateProfileString(CS_Theme_Section, CS_Theme_Name,TEXT(""), Infotmp.ThemeName, MAX_PATH, pExistFile);
		GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏背景图"), TEXT(""), main_image_name, MAX_PATH, pExistFile);

		//如果没有主题名称
		if (0 == _tcsicmp(Infotmp.ThemeName, TEXT("")))
			continue;

		//如果有背景图片，但是背景图片不存在，也跳过，此为原delphi逻辑
		//如果背景图片值为空，或者背景图片不存在，也跳过，此为修改后逻辑
		if (0 == _tcsicmp(main_image_name, TEXT("")))
			continue;
		else
		{
			CombineDirAndFile(PathFold, main_image_name, pImgFile);

			if (!FileExists(pImgFile))
				continue;
		}

		if (-1 == ThemeExist(Infotmp, ThemeList, i))
			ThemeList[i++] = Infotmp;

	}while(FindNextFile(hfind, &FindFileData));

	FindClose(hfind);

	//对主题名进行排序
	qsort(ThemeList, i, sizeof(stThemeInfo), SortByThemeName);

	return i;
}

//生成Theme列表
int GetThemeList(TCHAR ThemeList[MAX_THEME_COUNT][MAX_FILE_NAME_LENGTH])
{
	stThemeInfo ThemeInfoList[MAX_THEME_COUNT];

	int iInfoIndex, iInfoSize;
	int inameHigh = MAX_THEME_COUNT;

	iInfoSize = LoadThemes(ThemeInfoList);//给ThemeList赋值
	inameHigh = min(inameHigh, iInfoSize);

	for (iInfoIndex = 0; iInfoIndex < inameHigh; ++iInfoIndex)
		_tcscpy_s(ThemeList[iInfoIndex], MAX_FILE_NAME_LENGTH, ThemeInfoList[iInfoIndex].ThemeName);

	return iInfoIndex;
}

int GetDrawMode(const TCHAR* pMode, int iDefaultMode)
{
	return (0 == _tcsicmp(pMode, TEXT("平铺"))) ? (0) : ((0 == _tcsicmp(pMode, TEXT("拉伸"))) ? 1 : iDefaultMode);
}

int StrToBoolNum(const TCHAR* pStr, int idefault)
{
	return (0 == _tcsicmp(pStr, TEXT(""))) ? (idefault) : ((0 == _tcsicmp(pStr, TEXT("是"))) ? 1 : 0);
}

int intToRGB(int iColor)
{
	return ((iColor & 0xFF0000) >> 16) | ((iColor & 0x0000FF) << 16) | (iColor & 0x00FF00);
}

int CALLBACK EnumFontFamiliesExProc(ENUMLOGFONTEX *lpelfe, NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam )

{
	if (!HasChineseFont && GB2312_CHARSET == lpelfe->elfLogFont.lfCharSet)
	{
		HasChineseFont = 1;
		return 1;
	}

	if (!HasEnglishFont && ANSI_CHARSET == lpelfe->elfLogFont.lfCharSet)
	{
		HasEnglishFont = 1;
		return 1;
	}

	return 0;
}

//处理主题里文件的路径，返回正确的绝对路径
TCHAR *GetFullPath(const TCHAR *Filepath, const TCHAR *Themepath, TCHAR *FullPath)
{
	//判断是否绝对路径
	if (Filepath[1] != TEXT(':') || Filepath[2] != TEXT('\\') || (!FileExists(Filepath)))
	{
		TCHAR *pdest = _tcsrchr( Themepath, TEXT('\\') );	//最后一个
		int idest = (int)(pdest - Themepath) + 1;
		TCHAR FileName[MAX_PATH];

		left(FullPath, Themepath, idest);	//带‘\’

		idest = (int)_tcslen(Filepath);
		pdest = _tcsrchr(Filepath, TEXT('\\') );

		if (NULL != pdest)
			idest = (int)_tcslen(pdest) - 1;

		right(FileName, Filepath, idest);
		_tcscat_s(FullPath, MAX_PATH, FileName);
	}

	return FullPath;
}

//从Themepath文件加载主题信息到Config
int LoadThemeFromFile(PIMCONFIG *Config, const TCHAR *Themepath, BOOL bOnlyImageFile, BOOL bLoadPubInfo)
{
	TCHAR *pDest, FullPath[MAX_PATH], tmpName[MAX_PATH], ThemeName[MAX_PATH];//全路径、相对路径公共部分、主题名(文件夹)
	TCHAR image_name[MAX_PATH];	//要素名
	TCHAR pDrawMode[MAX_PATH];

	pDest = _tcsrchr(Themepath, TEXT('\\'));	//最后一个
	left(tmpName, Themepath, (int)(pDest- Themepath));

	pDest = _tcsrchr(tmpName, TEXT('\\'));		//最后一个
	right(ThemeName,tmpName,(int)_tcslen(tmpName) - (int)(pDest- tmpName) - 1);	//foldname
	_tcscpy_s(tmpName, MAX_PATH, CS_Theme_Path);
	_tcscat_s(tmpName, MAX_PATH, ThemeName);	//将FileName格式化为'unispim6\theme\+fold\'

	//主题名称ThemeName
	if (0 == GetPrivateProfileString(CS_Theme_Section, CS_Theme_Name, TEXT(""), ThemeName, MAX_PATH, Themepath))
		return 0;

	//LoadImageFileName
	//赋值失败时要清空原来的值
	_tcscpy_s(Config->main_image_name,			 MAX_FILE_NAME_LENGTH, TEXT(""));
	_tcscpy_s(Config->main_vert_image_name,		 MAX_FILE_NAME_LENGTH, TEXT(""));
	_tcscpy_s(Config->main_line_image_name,		 MAX_FILE_NAME_LENGTH, TEXT(""));
	_tcscpy_s(Config->main_vert_line_image_name, MAX_FILE_NAME_LENGTH, TEXT(""));
	_tcscpy_s(Config->status_image_name,		 MAX_FILE_NAME_LENGTH, TEXT(""));
	_tcscpy_s(Config->status_buttons_image_name, MAX_FILE_NAME_LENGTH, TEXT(""));

	//读取'输入栏背景图'的值
	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏背景图"), TEXT(""), image_name, MAX_PATH,Themepath))
	{
		GetFullPath(image_name, Themepath, FullPath);
		if (FileExists(FullPath))
			CombineDirAndFile(tmpName, image_name, Config->main_image_name);//将FileName格式化为'unispim6\theme\+fold\*.png'
	}

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏竖排背景图"), TEXT(""), image_name, MAX_PATH, Themepath))
	{
		GetFullPath(image_name, Themepath, FullPath);
		if (FileExists(FullPath))
			CombineDirAndFile(tmpName, image_name, Config->main_vert_image_name);//将FileName格式化为'unispim6\theme\+fold\*.png'
	}

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏中心线图"), TEXT(""), image_name, MAX_PATH, Themepath))
	{
		GetFullPath(image_name, Themepath, FullPath);
		if (FileExists(FullPath))
			CombineDirAndFile(tmpName, image_name, Config->main_line_image_name);//将FileName格式化为'unispim6\theme\+fold\*.png'
	}

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏竖排中心线图"), TEXT(""), image_name, MAX_PATH, Themepath))
	{
		GetFullPath(image_name, Themepath, FullPath);
		if (FileExists(FullPath))
			CombineDirAndFile(tmpName, image_name, Config->main_vert_line_image_name);//将FileName格式化为'unispim6\theme\+fold\*.png'
	}

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("状态栏背景图"), TEXT(""), image_name, MAX_PATH, Themepath))
	{
		GetFullPath(image_name, Themepath, FullPath);
		if (FileExists(FullPath))
			CombineDirAndFile(tmpName, image_name, Config->status_image_name);//将FileName格式化为'unispim6\theme\+fold\*.png'
	}

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("状态栏按钮图"), TEXT(""), image_name, MAX_PATH, Themepath))
	{
		GetFullPath(image_name, Themepath, FullPath);
		if (FileExists(FullPath))
			CombineDirAndFile(tmpName, image_name, Config->status_buttons_image_name);//将FileName格式化为'unispim6\theme\+fold\*.png'
	}

	if (bOnlyImageFile)
		return TRUE;

	//读入显示信息
	Config->theme_use_for_horizon_mode  = 1;
	Config->theme_use_for_vertical_mode = 1;
	Config->theme_use_for_expand_mode   = 1;

	GetPrivateProfileString(CS_Theme_Section, TEXT("Tab扩展时使用竖排背景"), TEXT("否"), image_name, MAX_PATH, Themepath);

	Config->use_vert_bk_when_expanding		= StrToBoolNum(image_name, 0); //Tab扩展时使用竖排背景
	Config->main_window_min_width			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏最小宽度"),					default_config.main_window_min_width, Themepath);
	Config->main_window_min_height			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏最小高度"),					default_config.main_window_min_height, Themepath);
	Config->main_window_anchor_x			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏锚点位置-X"),				default_config.main_window_anchor_x, Themepath);
	Config->main_window_anchor_y			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏锚点位置-Y"),				default_config.main_window_anchor_y, Themepath);
	Config->main_window_vert_min_width		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排最小宽度"),				default_config.main_window_vert_min_width, Themepath);
	Config->main_window_vert_min_height		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排最小高度"),				default_config.main_window_vert_min_height, Themepath);
	Config->status_right_space				= GetPrivateProfileInt(CS_Theme_Section, TEXT("状态栏背景图右侧空白宽度"),		default_config.status_right_space, Themepath);
	Config->main_window_vert_anchor_x		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排锚点位置-X"),			default_config.main_window_vert_anchor_x, Themepath);
	Config->main_window_vert_anchor_y		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排锚点位置-Y"),			default_config.main_window_vert_anchor_y, Themepath);
	Config->main_left_margin				= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏背景图左边界"),				default_config.main_left_margin, Themepath);
	Config->main_right_margin				= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏背景图右边界"),				default_config.main_right_margin, Themepath);
	Config->main_top_margin					= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏背景图上边界"),				default_config.main_top_margin, Themepath);
	Config->main_bottom_margin				= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏背景图下边界"),				default_config.main_bottom_margin, Themepath);
	Config->main_text_left_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏文字左边界"),				default_config.main_text_left_margin, Themepath);
	Config->main_text_right_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏文字右边界"),				default_config.main_text_right_margin, Themepath);
	Config->main_text_top_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏文字上边界"),				default_config.main_text_top_margin, Themepath);
	Config->main_text_bottom_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏文字下边界"),				default_config.main_text_bottom_margin, Themepath);
	Config->main_center_gap_height			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏中心高度"),					default_config.main_center_gap_height, Themepath);
	Config->main_remain_number_margin		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏剩余候选数目字符右边界"),	default_config.main_remain_number_margin, Themepath);
	Config->main_line_left_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏中心线左边界"),				default_config.main_line_left_margin, Themepath);
	Config->main_line_right_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏中心线右边界"),				default_config.main_line_right_margin, Themepath);
	Config->hint_right_margin				= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏提示信息右边界"),			default_config.hint_right_margin, Themepath);
	Config->main_vert_left_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排背景图左边界"),			default_config.main_vert_left_margin, Themepath);
	Config->main_vert_right_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排背景图右边界"),			default_config.main_vert_right_margin, Themepath);
	Config->main_vert_top_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排背景图上边界"),			default_config.main_vert_top_margin, Themepath);
	Config->main_vert_bottom_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排背景图下边界"),			default_config.main_vert_bottom_margin, Themepath);
	Config->main_vert_text_left_margin		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排文字左边界"),			default_config.main_vert_text_left_margin, Themepath);
	Config->main_vert_text_right_margin		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排文字右边界"),			default_config.main_vert_text_right_margin, Themepath);
	Config->main_vert_text_top_margin		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排文字上边界"),			default_config.main_vert_text_top_margin, Themepath);
	Config->main_vert_text_bottom_margin	= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排文字下边界"),			default_config.main_vert_text_bottom_margin, Themepath);
	Config->main_vert_center_gap_height		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排中心高度"),				default_config.main_vert_center_gap_height, Themepath);
	Config->main_vert_remain_number_margin  = GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排剩余候选数目字符右边界"),	default_config.main_vert_remain_number_margin, Themepath);
	Config->main_vert_line_left_margin		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排中心线左边界"),			default_config.main_vert_line_left_margin, Themepath);
	Config->main_vert_line_right_margin		= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排中心线右边界"),			default_config.main_vert_line_right_margin, Themepath);
	Config->hint_vert_right_margin			= GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏竖排提示信息右边界"),		default_config.hint_vert_right_margin, Themepath);
	Config->status_window_top_margin		= GetPrivateProfileInt(CS_Theme_Section, TEXT("状态栏按钮图上边界"),				default_config.status_window_top_margin, Themepath);
	Config->status_left_margin				= GetPrivateProfileInt(CS_Theme_Section, TEXT("状态栏背景图左边界"),				default_config.status_left_margin, Themepath);
	Config->status_right_margin				= GetPrivateProfileInt(CS_Theme_Section, TEXT("状态栏背景图右边界"),				default_config.status_right_margin, Themepath);
	Config->status_left_space				= GetPrivateProfileInt(CS_Theme_Section, TEXT("状态栏背景图左侧空白宽度"),		default_config.status_left_space, Themepath);
	Config->status_min_width				= GetPrivateProfileInt(CS_Theme_Section, TEXT("状态栏最小宽度"),					default_config.status_min_width, Themepath);

	Config->main_string_color			= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏拼音字符颜色"),			default_config.main_string_color, Themepath));
	Config->main_caret_color			= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏光标颜色"),			default_config.main_caret_color, Themepath));
	Config->main_number_color			= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏数字颜色"),			default_config.main_number_color, Themepath));
	Config->main_selected_color			= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏活动候选字符颜色"),		default_config.main_selected_color, Themepath));
	Config->main_candidate_color		= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏候选字符颜色"),			default_config.main_candidate_color, Themepath));
	Config->main_icw_color				= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏智能组词候选字符颜色"),	default_config.main_icw_color, Themepath));
	Config->main_remains_number_color	= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("输入栏剩余候选数目字符颜色"),	default_config.main_remains_number_color, Themepath));
	Config->main_sp_hint_border_color	= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("双拼提示边框颜色"),			default_config.main_sp_hint_border_color, Themepath));
	Config->main_sp_hint_back_color		= intToRGB(GetPrivateProfileInt(CS_Theme_Section, TEXT("双拼提示背景颜色"),			default_config.main_sp_hint_back_color, Themepath));

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏背景图中心绘制模式"), TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_center_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏背景图左右边界绘制模式"),TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_vertical_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏背景图上下边界绘制模式"),TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_horizontal_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏中心线绘制模式"),TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_line_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏竖排背景图中心绘制模式"), TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_vert_center_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏竖排背景图左右边界绘制模式"), TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_vert_vertical_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏竖排背景图上下边界绘制模式"), TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_vert_horizontal_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("输入栏竖排中心线绘制模式"), TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->main_vert_line_mode = GetDrawMode(pDrawMode, 1);

	if (0 != GetPrivateProfileString(CS_Theme_Section, TEXT("状态栏背景图中心绘制模式"), TEXT(""), pDrawMode, MAX_PATH, Themepath))
		Config->status_center_mode = GetDrawMode(pDrawMode, 1);

	//给字体赋值
	if (bLoadPubInfo)
	{
		HDC hDC = GetDC(NULL);
		LOGFONT lf = { 0 };

		//给中文字体赋值
		GetPrivateProfileString(CS_Theme_Section, TEXT("中文字体"), TEXT("宋体"), lf.lfFaceName, MAX_PATH, Themepath);

		lf.lfCharSet = GB2312_CHARSET;
		EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, 0, 0);

		if (!HasChineseFont)
		{
			_tcscpy_s(lf.lfFaceName,LF_FACESIZE,TEXT("宋体"));
			EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, 0, 0);

			if (!HasChineseFont)
			{
				_tcscpy_s(lf.lfFaceName,LF_FACESIZE,TEXT("SimSun"));
				EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, 0, 0);
			}
		}

		if (HasChineseFont)
			_tcscpy_s(Config->chinese_font_name,MAX_FILE_NAME_LENGTH,lf.lfFaceName);

		//给英文字体赋值
		GetPrivateProfileString(CS_Theme_Section, TEXT("英文字体"), TEXT("Arial"), lf.lfFaceName, MAX_PATH, Themepath);
		lf.lfCharSet = ANSI_CHARSET;
		EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, 0, 0);

		if (!HasEnglishFont)
		{
			_tcscpy_s(lf.lfFaceName,LF_FACESIZE,TEXT("Arial"));
			EnumFontFamiliesEx(hDC, &lf, (FONTENUMPROC)EnumFontFamiliesExProc, 0, 0);
		}

		if (HasEnglishFont)
			_tcscpy_s(Config->english_font_name, LF_FACESIZE, lf.lfFaceName);

		ReleaseDC(NULL, hDC);
	}

	return 1;
}

//从Themepath文件加载主题信息到Config
//输入参数： Config              ----- 待填充的主题信息结构体
//			 OnlyImageFile       ----- 是否只加载图片信息
//			 LoadPubInfo         ----- 是否加载显示字体信息
//输出参数： Config              ----- 填充的主题信息结构体
int LoadThemeDetail(PIMCONFIG *Config, const TCHAR *Themepath, int OnlyImageFile, int LoadPubInfo)
{

	if(!LoadThemeFromFile(Config, Themepath, OnlyImageFile, LoadPubInfo))
		return 0;

	return 1;
}

//根据Theme_name名称，读取Theme信息到Config
int LoadThemeByCaption(PIMCONFIG *Config)
{
	TCHAR ThemeFold[MAX_PATH];
	TCHAR ThemeName[MAX_FILE_NAME_LENGTH];
	TCHAR ThemePath[MAX_PATH];

	if (!GetAllUserAppDirectory(ThemeFold))
		return 0;

	if (ThemeFold[_tcslen(ThemeFold) - 1] != '\\')
		_tcscat_s(ThemeFold, MAX_PATH, TEXT("\\"));

	_tcscat_s(ThemeFold, MAX_PATH, THEME_DIR_NAME);
	CombineDirAndFile(ThemeFold, Config->theme_name, ThemePath);
	CombineDirAndFile(ThemePath, CS_Theme_File, ThemeName);//完整的theme_name信息

	if (FileExists(ThemeName))
		return LoadThemeDetail(Config, ThemeName, 0, 1);//加载

	return 0;
}

//遍历所有主题，以得到目标主题
int LoadThemeByName(PIMCONFIG *Config)
{
	TCHAR DefaultThemePath[MAX_PATH], ThemeFullPath[MAX_PATH];
	stThemeInfo Infotmp, ThemeList[MAX_THEME_COUNT];
	int iSize, index;

	_tcscpy_s(Infotmp.ThemeName, MAX_THEME_NAME_LENGTH, Config->theme_name);
	iSize = LoadThemes(ThemeList);
	index = ThemeExist(Infotmp, ThemeList, iSize);

	if ( -1 != index )
	{
		Infotmp = ThemeList[index];
		CombineDirAndFile(GetThemePath(ThemeFullPath), Infotmp.FolderName, DefaultThemePath);
		CombineDirAndFile(DefaultThemePath, CS_Theme_File, ThemeFullPath);//完整的theme_name信息
		LoadThemeDetail(Config,ThemeFullPath, FALSE, TRUE);

		return 1;
	}

	return 0;
}

//根据config，读取Theme信息到Config
int MakeTheme(PIMCONFIG *Config)
{
	//根据Theme_Fold名称，读取Theme信息到Config
	//根据Theme_name名称，读取Theme信息到Config(Theme_Fold <> Theme_name)
	if (LoadThemeByCaption(Config) || LoadThemeByName(Config))
		return 1;

	return 0;
}

//用目标文件的相对路径，加上系统目录，组成绝对路径
// 输入参数： Filepath         ----- 目标文件的相对路径
//            AllUserDir       ----- 系统目录参数是allUserDir，还是current User路径
//			  pFullPath        ----- 目标文件的绝对路径
//			  AutoCreateDir    ----- 目标文件是否需要创建
// 输出参数： pFullPath        ----- 目标文件的绝对路径
TCHAR* GetFullFilePath(const TCHAR *Filepath, int AllUserDir, TCHAR *pFullPath, int AutoCreateDir)
{
	TCHAR pBasDir[MAX_PATH];

	if (AllUserDir)
		GetAllUserAppDirectory(pBasDir);
	else
		GetUserAppDirectory(pBasDir);

	CombineDirAndFile(pBasDir, Filepath, pFullPath);

	if (AutoCreateDir)
	{
		if('\\' == pFullPath[(int)_tcslen(pFullPath) - 1])
			pFullPath[(int)_tcslen(pFullPath)-1] = 0;

		SHCreateDirectoryEx(0, pFullPath, 0);
	}

	return pFullPath;
}

//按时分秒进行比较
int Timecompare(const SYSTEMTIME one,const SYSTEMTIME other)
{
	return ((one.wHour > other.wHour) ? (1)	:
			((one.wHour < other.wHour) ? (-1) :
			((one.wMinute > other.wMinute) ? (1) :
			((one.wMinute < other.wMinute) ? (-1) :
			((one.wSecond > other.wSecond) ? (1) :
			((one.wSecond < other.wSecond) ? (-1) : 0))))));
}

//按年、月、日进行比较
int datecompare(const SYSTEMTIME one,const SYSTEMTIME other, int OnlyDate)
{
	if (one.wYear > other.wYear)
		return 1;
	else if (one.wYear == other.wYear)
	{
		if (one.wMonth > other.wMonth)
			return 1;
		else if (one.wMonth == other.wMonth)
		{
			if (one.wDay > other.wDay)
				return 1;
			else if (one.wDay == other.wDay)
			{
				if (OnlyDate)
					return 0;
				else
					return Timecompare(one,other);
			}
			else
				return -1;
		}
		else
			return -1;
	}
	else
		return -1;
}

//保存自动更新配置文件
//void SaveLocalConfig()
//{
//	HANDLE hfile;
//	TCHAR AllAppDir[MAX_PATH],ConfigFile[MAX_PATH],tmp[2],NowDate[0x20];
//	SYSTEMTIME TimeNull;
//
//    //1ConfigFile := TUpdateFuns.GetLocalConfigFile;
//	GetAllUserAppDirectory(AllAppDir);
//	CombineDirAndFile(AllAppDir, CS_ConfigFile, ConfigFile);
//
//    //2 TUpdateFuns.FileLocked(ConfigFile)
//	hfile = CreateFile((ConfigFile), 
//						GENERIC_READ || GENERIC_WRITE,
//						FILE_SHARE_READ,
//						0,
//						OPEN_EXISTING,
//						FILE_ATTRIBUTE_NORMAL,
//						0);
//
//	if (hfile)
//		CloseHandle(hfile);
//	else
//		return;
//
//	_stprintf_s(tmp, _SizeOf(tmp), TEXT("%d"), localconfig.LibUpdateMode);
//	WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_LibUpdateMode, tmp, ConfigFile); //int->char*
//
//	_stprintf_s(tmp, _SizeOf(tmp), TEXT("%d"), localconfig.AppUpdateMode);
//	WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_AppUpdateMode, tmp, ConfigFile);
//
//	WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_UserID, localconfig.UserID, ConfigFile);
//
//	_stprintf_s(NowDate, _SizeOf(NowDate), TEXT("%d"), localconfig.WordlibVersion);
//	WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_WordlibVersion, NowDate, ConfigFile);			//词库版本
//
//	WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_AppVersion, localconfig.AppVersion, ConfigFile);	//程序版本
//
//	ZeroMemory(&TimeNull, sizeof(SYSTEMTIME));
//	if (0 != datecompare(localconfig.LastDate, TimeNull, 1))
//	{//有效
//		_stprintf_s(NowDate, 
//					_SizeOf(NowDate), 
//					TEXT("%u-%u-%u"), 
//					localconfig.LastDate.wYear,//%04u-%02u-%02u
//					localconfig.LastDate.wMonth, 
//					localconfig.LastDate.wDay);
//
//		WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_LatestDate, NowDate, ConfigFile);
//	}
//
//	if (0 != datecompare(localconfig.CheckTime,TimeNull,FALSE))
//	{
//		_stprintf_s(NowDate,
//					_SizeOf(NowDate), 
//					TEXT("%u-%u-%u %u:%u:%u"),
//					localconfig.CheckTime.wYear,
//					localconfig.CheckTime.wMonth,
//					localconfig.CheckTime.wDay,
//					localconfig.CheckTime.wHour,
//					localconfig.CheckTime.wMinute,
//					localconfig.CheckTime.wSecond);
//
//		WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_CheckTime, NowDate, ConfigFile);//2010-5-5 9:14:48
//	}
//
//	_stprintf_s(NowDate, _SizeOf(NowDate), TEXT("%d"), localconfig.balanceTime);
//	WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_BalanceTime, NowDate, ConfigFile);
//
//	_stprintf_s(NowDate, _SizeOf(NowDate), TEXT("%d"), localconfig.URLMaxSize);
//	WritePrivateProfileString(CS_INI_UpdateInfo, CS_INI_URLMaxSize, NowDate, ConfigFile);
//}

//生成随机时间
//输入参数： iMinute          ----- 负载均衡时间(分钟)
//返 回 值： SYSTEMTIME       ----- 随机时间
/*SYSTEMTIME GeneCheckTime(int iMinute)
{
	struct tm CheckTime;
	SYSTEMTIME Checkday;
	unsigned int iMax, irand;
	time_t randtime;	//基于秒

	ZeroMemory(&Checkday, sizeof(SYSTEMTIME));

	time(&randtime);

	//获得当前时间CheckTime
	if (localtime_s(&CheckTime, &randtime))
	{
		iMax = iMinute * 60;

		srand((unsigned)time(NULL));

		irand = (unsigned int) ((double)rand() / (RAND_MAX + 1) * (iMax - 0));

		randtime += irand;

		//获得当前时间CheckTime
		if (!localtime_s(&CheckTime, &randtime))
		{
			Checkday.wYear	 = CheckTime.tm_year + 1900;
			Checkday.wMonth  = CheckTime.tm_mon + 1;
			Checkday.wDay	 = CheckTime.tm_mday;
			Checkday.wHour	 = CheckTime.tm_hour;
			Checkday.wMinute = CheckTime.tm_min;
			Checkday.wSecond = CheckTime.tm_sec;
		}
	}

	return Checkday;
}*/

//同一天比较
/*int MinusSecond(SYSTEMTIME bigday, SYSTEMTIME littleday)
{
	return 60 * (60 * (24 * (bigday.wDay - littleday.wDay) + bigday.wHour - littleday.wHour) + 
		   bigday.wMinute - littleday.wMinute) + bigday.wSecond - littleday.wSecond;
}*/

//能否运行自动更新程序(判断当前是否允许更新、且是否已更新)
//输入参数： handlIme              ----- Ime窗口句柄
/*int CanRunUpdateApp(INT_PTR handlIme)
{
	int not_checked = 1;
	SYSTEMTIME Today;
	int nDelayTime;
	int imsgType = 1;//csCallMeLater

	GetLocalTime(&Today);

	//加载本地信息
	LoadLocalConfig();

	//1是否允许自动更新:AutoUpdate.CanAutoUpdate，根据参数判断是否需要运行更新,否则返回False
	if (CN_Update_Auto != localconfig.LibUpdateMode && CN_Update_Prompt != localconfig.LibUpdateMode &&
		CN_Update_Auto != localconfig.AppUpdateMode && CN_Update_Prompt != localconfig.AppUpdateMode)
	{
		PostMessage((HWND)handlIme, UIM_UPDATE_NOTIFY, 0, 0);//更新完毕,延迟0
		return 0;
	}

	//2今天没有检查过:!AutoUpdate.TodayChecked()
	//判断最后检查日期和当前日期是否相同,否则生成今天需要检查的时间并保存起来
	if (0 == datecompare(Today, localconfig.LastDate, 1))
	{
		imsgType = 0;//csFinished
		not_checked = 0;
	}
	//如果检查日期不是今天，就需要重新写检查日期
	else if (0 != datecompare(Today, localconfig.CheckTime, 1))
	{
		localconfig.CheckTime = GeneCheckTime(localconfig.balanceTime);
		SaveLocalConfig();
	}

	//通知一下IME，是不用再调用了，还是延时继续调用
	if (0 == imsgType)
		nDelayTime = 0;
	else
		nDelayTime = MinusSecond(localconfig.CheckTime, Today);

	PostMessage((HWND)handlIme, UIM_UPDATE_NOTIFY, imsgType, nDelayTime);

	if (!not_checked)//已经检查
		return 0;

	//3(现在)到检查时间了:AutoUpdate.IsTodayCheckTime()
	//检查时间大于等于当前时间说明需要进行检查
	if (0 > Timecompare(localconfig.CheckTime,Today))
		return 0;//不需要进行检查

	return 1;
}*/

//将文件按行装载到缓冲区
int LoadStringFromFile(const TCHAR *file_name, TCHAR buffer[][MAX_CAND_HINT_SIZE])
{
	FILE *fd;
	int i = 0;

	if (_tfopen_s(&fd,file_name, TEXT("rb")))
		return -1;

	while (!feof(fd))
		fgets((char*)buffer[i++], MAX_CAND_HINT_SIZE, fd);

	fclose(fd);

	return i;
}

//随机获取智能提示文字
int GetHintMessage(TCHAR *MsgBuffer, int BufferSize)
{
	int iline = -1;
	unsigned int iIndex;
	TCHAR HintInfoList[MAX_CAND_HINT_NUM][MAX_CAND_HINT_SIZE], pHintPath[MAX_PATH];
	FILE *fd;

	GetFullFilePath(CS_HintFile, TRUE, pHintPath, FALSE);

	if (_tfopen_s(&fd, pHintPath, TEXT("rb")))
		return 0;

	do
	{
		//读取一行数据
		if (!GetLineFromFile(fd, HintInfoList[++iline], MAX_CAND_HINT_SIZE))
			break;			//可能到结尾了
	}while(1);

	fclose(fd);

	if (--iline < 0)//失败或为空
		return 0;

	srand((unsigned)time(NULL));
	iIndex = rand();

	iIndex = (unsigned int) ((double)iIndex / (RAND_MAX + 1) * (iline - 0));
	_tcscpy_s(MsgBuffer, BufferSize, HintInfoList[iIndex]);

	return 1;
}