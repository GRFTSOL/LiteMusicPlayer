// SimpleJVM.cpp : Defines the entry point for the console application.
//


#include "SJVM.h"

 int __stdcall    Func(int a, int b, int c)
{
    printf ("a: %d, b: %d, c: %d\n", a, b, c);
    return a + b + c;
}

void ASMCallTest()
{
    int        nRet = 1;
    int        vParameter[10] = { 1, 2, 3, 4, };

    nRet = Func(vParameter[0], vParameter[1], vParameter[2]);


    printf("%d\n", nRet);

    int        nCount = 2;

    //    for (int i = (int)2; i >= 0; i--)
    {
        //        printf("%d", vParameter[i]);
        __asm
        {
            mov         eax,    dword ptr [nCount]
ParamStart:
            cmp         eax,    0 
                jl          Param_end
                mov         ecx,    dword ptr vParameter[eax*4] 
            push        ecx  
                sub         eax,    1 
                jmp            ParamStart 
Param_end:
        }
    }

    void *pNativeFunc = (void *)Func;

    __asm  {
        mov            eax,      dword ptr [pNativeFunc]
        call        eax
            mov         dword ptr [nRet],eax 
    };

    printf("%d\n", nRet);

}

void __stdcall SJVM_DebugLog_print_Int(int value)
{
    printf("%d\n", value);
}

void __stdcall SJVM_DebugLog_printStr(const uint32_t *szValue)
{
    wprintf(L"%s\n", szValue);
}

int runFile(cstr_t szFile)
{
    uint32_t        dwBeg, dwEnd;

    dwBeg = getTickCount();

    if (g_sjvm.compile(szFile) != ERR_OK)
        return 1;

    dwEnd = getTickCount();
    printf("compile cost: %d ms\n", dwEnd - dwBeg);

    g_sjvm.registerNativeAPI("DebugLog", "print", SJVM_DebugLog_print_Int, TID_VOID, TID_INT, TID_INVALID);

    g_sjvm.registerNativeAPI("DebugLog", "print", SJVM_DebugLog_printStr, TID_VOID, TID_STRING, TID_INVALID);

    g_sjvm.run();

    dwEnd = getTickCount();
    printf("execute cost: %d ms\n", dwEnd - dwBeg);

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
/*    ++argv, --argc;
    if (argc <= 0)
    {
        printf("Please input the file to be compiled.");
        return 1;
    }*/

    // set flag to freeze the current SJVM status as system module.
    g_sjvm.finishedInitSysModule();

    if (argc > 1)
    {
        runFile(argv[1]);
    }

    char        szFolder[MAX_PATH];

    getModulePath(szFolder, nullptr);
    string        strFolder = szFolder;
    strFolder += "..\\SimpleJVM\\test\\";

    VecStrings        vFiles;

    if (!enumFilesInDir(strFolder.c_str(), "*.*", vFiles, true))
    {
        printf("No file is in the test folder");
        return 1;
    }

    uint32_t        dwBeg, dwEnd;

    for (size_t i = 0; i < vFiles.size(); i++)
    {
        _tprintf("run test case: %s\n", vFiles[i].c_str());

        dwBeg = getTickCount();

        g_sjvm.reset();

        if (g_sjvm.compile(vFiles[i].c_str()) != ERR_OK)
            return 1;

        dwEnd = getTickCount();
        printf("compile cost: %d ms\n", dwEnd - dwBeg);

        g_sjvm.registerNativeAPI("DebugLog", "print", SJVM_DebugLog_print_Int, TID_VOID, TID_INT, TID_INVALID);

        g_sjvm.registerNativeAPI("DebugLog", "print", SJVM_DebugLog_printStr, TID_VOID, TID_STRING, TID_INVALID);

        g_sjvm.run();

        dwEnd = getTickCount();
        printf("execute cost: %d ms\n", dwEnd - dwBeg);
    }

    return 0;
}


int    func22(int a, int b, int c)
{
    return a + b + c;
}
