
#include <windows.h>
#include "filesystemutil.h"


//////////////////////////////////////////////////////////////////////////
// seachFile
//
unsigned int SearchFile(char* dir, FILE* fp) {
    std::vector<std::basic_string<char> > aStr;
    travelDir(aStr, std::basic_string<char> (dir));
    for(std::vector<std::basic_string<char> >::iterator i = aStr.begin(); i != aStr.end(); i++) {
        fwrite(i->c_str(), sizeof(char), i->length(), fp);
        fwrite("\r\n", sizeof(char), i->length(), fp);
    }
    fclose(fp);
    return 0;
}



//////////////////////////////////////////////////////////////////////////
// Check if is a supported file
//
bool isSupportedPicFile(cstr_t lpFileName)            // point to file name

{
    if(lpFileName) {
        cstr_t pExt = strrchr(lpFileName, '.');
        if(pExt) {
            pExt++;
            if((0 == strcasecmp("mp3", pExt))
                || (0 == strcasecmp("ape", pExt))
                || (0 == strcasecmp("wav", pExt))) {
                return true;
            }
        }
    }
    return false;
}
//////////////////////////////////////////////////////////////////////////
// search files in directory
void travelDir(std::vector<std::basic_string<char> >& vsFiles,
    const std::basic_string<char> sDir) {
    char* pExtension = nullptr;
    WIN32_FIND_DATA data;
    std::basic_string<char> str = sDir + "\\*.*";
    std::basic_string<char> sTDir = sDir;

    HANDLE hFind = FindFirstFile(str.c_str(), &data);
    if (INVALID_HANDLE_VALUE != hFind) {
        if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes ) {
            if((0 != strcmp(".", data.cFileName) )
                && (0 != strcmp("..", data.cFileName) )) {
                sTDir = sDir + "\\" + data.cFileName;
                travelDir(vsFiles, sTDir);
            }
        } else {
            if(isSupportedPicFile(data.cFileName )) {
                vsFiles.push_back(sDir + "\\" + data.cFileName);
            }
        }

        while(FindNextFile(hFind, &data)) {
            if(FILE_ATTRIBUTE_DIRECTORY == data.dwFileAttributes) {
                if( (0 != strcmp(".", data.cFileName) )
                    && (0 != strcmp("..", data.cFileName) )) {
                    sTDir = sDir + "\\" + data.cFileName;
                    travelDir(vsFiles, sTDir);
                }
            } else {
                if(isSupportedPicFile(data.cFileName)) {
                    vsFiles.push_back(sDir + "\\" + data.cFileName);
                }
            }
        }// while
    }// if

}
#if 0
//////////////////////////////////////////////////////////////////////////
// get next file from specified directory vector
std::basic_string<char> GetNextFile(const std::vector<std::basic_string<char> >& vsDirectories,
    cstr_t pCurrentFile,
    bool bRandmon) {
    static bool bFirstIn = true;
    if(bFirstIn) {
        srand( (unsigned)time( nullptr ) );
        bFirstIn = false;
    }

    std::vector<std::basic_string<char> > vsFileList;
    for(uint32_t i = 0; i < vsDirectories.size(); i++) {
        travelDir(vsFileList, vsDirectories[i]);
    }

    if(0 == vsFileList.size()) {
        return std::basic_string<char>();
    } else if (1 == vsFileList.size()) {
        return vsFileList[0];
    } else {
        if(true == bRandmon) {
            int i = 0;
            do{
                i = rand() * vsFileList.size() / RAND_MAX;
            }while(0 == strcmp(vsFileList[i].c_str(), pCurrentFile));

            return vsFileList[i];
        } else {
            if(nullptr == pCurrentFile) {
                return vsFileList[0];
            } else {
                for(uint32_t j = 0; j < vsFileList.size(); j++) {
                    if(0 == strcmp(vsFileList[j].c_str(), pCurrentFile)) {
                        return vsFileList[(j + 1) % vsFileList.size()];
                    }
                }
            }

        }
    }
}



static NOTIFYICONDATA nid;
//////////////////////////////////////////////////////////////////////////
// add Notify icon
void addNotifyIconInSysTray(HWND hWnd) {
    ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(IDI_ICON_WALLPAPER));
    nid.hWnd = hWnd;
    strncpy(nid.szTip, LoadStringFromRes(IDS_SYS_APP_NAME).c_str(), 64);
    nid.uCallbackMessage = UM_NOTIFY_ICON;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

    Shell_NotifyIcon(NIM_ADD, &nid);
}
//////////////////////////////////////////////////////////////////////////
// remove Notify icon
void removeNotifyIconInSysTray(void) {
    Shell_NotifyIcon(NIM_DELETE, &nid);
}

#endif
#if 0

LPGUID pGuid = (LPGUID) & GUID_USB_DEVICE_ENUMERATOR_INTERFACE_CLASS;

HDEVINFO hardwareDeviceInfo;

ULONG predictedLength = 0;
ULONG requiredLength = 0;

hardwareDeviceInfo   =   SetupDiGetClassDevs   (
    pGuid,
    nullptr,   //   Define   no   enumerator   (global)
    nullptr,   //   Define   no
    (DIGCF_PRESENT   |   //   Only   Devices   present
    DIGCF_INTERFACEDEVICE   )
    );   //   Function   class   devices.

if(INVALID_HANDLE_VALUE   ==   hardwareDeviceInfo) {
    TRACE("Windows   \"SetupDiGetClassDevs\"   API   Fail",nullptr,MB_OK);
    return bRetValue;
}

SP_INTERFACE_DEVICE_DATA deviceInterfaceData;
ZeroMemory(&deviceInterfaceData,sizeof (SP_INTERFACE_DEVICE_DATA));
deviceInterfaceData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

while(SetupDiEnumDeviceInterfaces   (
    hardwareDeviceInfo,
    0,   //   No   care   about   specific   PDOs
    pGuid,
    index,   //device   -1
    &deviceInterfaceData)
    ) {
    SetupDiGetDeviceInterfaceDetail   (
        hardwareDeviceInfo,
        &deviceInterfaceData,
        nullptr,   //   probing   so   no   output   buffer   yet
        0,   //   probing   so   output   buffer   length   of   zero
        &requiredLength,
        nullptr
        );

    PSP_DEVICE_INTERFACE_DETAIL_DATA FlashDiskInterfaceDetailData = nullptr;
    FlashDiskInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc (requiredLength);
    ZeroMemory(FlashDiskInterfaceDetailData,sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA));
    FlashDiskInterfaceDetailData->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

    SP_DEVINFO_DATA device_info_data;
    ZeroMemory(&device_info_data,sizeof (SP_DEVINFO_DATA));
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

    predictedLength = requiredLength;

    if   (!   SetupDiGetDeviceInterfaceDetail   (
        hardwareDeviceInfo,
        &deviceInterfaceData,
        FlashDiskInterfaceDetailData,
        predictedLength,
        &requiredLength,
        &device_info_data)) {
        // messageBox(nullptr,"Windows   \"SetupDiGetInterfaceDeviceDetail\"   API   Fail",nullptr,MB_OK);
        SAFE_FREE(FlashDiskInterfaceDetailData);
        break;
    }
    //   find   device   interface
    // TRACE("device_info_data.DevInst   0x%04x\n",device_info_data.DevInst);

    bRetValue = IsDev(device_info_data.DevInst); //需求
    SAFE_FREE(FlashDiskInterfaceDetailData);
    index ++;

    if   (bRetValue   ==   true) {
        break;
    }
}

SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);


#endif
