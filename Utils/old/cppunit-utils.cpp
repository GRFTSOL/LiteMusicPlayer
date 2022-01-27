#include "cppunit-utils.h"

#ifdef _CPPUNIT_TEST

#include <cppunit/CompilerOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <stdexcept>
#include <fstream>
#include <cppunit/Portability/Stream.h>

using namespace CppUnit;

#ifdef _WIN32

#include <fcntl.h>
#include <io.h>


#define SZ_CLASSNAME "UnitTest"
#define SZ_WNDNAME    "UnitTest"

LRESULT CALLBACK LogWndProc(HWND hWnd, uint32_t uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_TIMER)
    {
        destroyWindow(hWnd);
        return 0;
    }
    else
        return defWindowProc(hWnd, uMsg, wParam, lParam);
}

ATOM unitTestRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    memset(&wcex, 0, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style            = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = LogWndProc;
    wcex.hInstance        = hInstance;
    wcex.hbrBackground    = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszClassName    = SZ_CLASSNAME;

    return registerClassEx(&wcex);
}

HWND g_hWndLogEdit = nullptr;
HWND g_hWndLog =  nullptr;

void logUnitTestResult(cstr_t szStr)
{
    if (g_hWndLogEdit == nullptr)
    {
        printf(szStr);
        return;
    }

    int    len = GetWindowTextLength(g_hWndLogEdit);
    sendMessage(g_hWndLogEdit, EM_SETSEL, len, len);

    sendMessage(g_hWndLogEdit, EM_REPLACESEL, false, (WPARAM)szStr);
}

void createUnitResultWnd(HINSTANCE hInstance)
{
    unitTestRegisterClass(hInstance);

    g_hWndLog = CreateWindow(SZ_CLASSNAME, SZ_WNDNAME, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
    if (!g_hWndLog)
        return;

    CRect rc;
    ::getClientRect(g_hWndLog, &rc);

    // create Edit
    uint32_t    dwStyle = WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | WS_VSCROLL | WS_HSCROLL| ES_MULTILINE | ES_WANTRETURN;
    g_hWndLogEdit = CreateWindow("EDIT", "", dwStyle, 
        0, 0, rc.right, rc.bottom, g_hWndLog, nullptr, hInstance, nullptr);

    showWindow(g_hWndLog, SW_SHOW);
    UpdateWindow(g_hWndLog);
}
/*
void redirectIOToConsole()
{
#define MAX_CONSOLE_LINES    500

    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;

    AllocConsole();

    // set the screen buffer to be big enough to let us scroll text
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    // redirect unbuffered STDOUT to the console
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stdout = *fp;
    setvbuf( stdout, nullptr, _IONBF, 0 );

    // redirect unbuffered STDIN to the console
    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
    setvbuf( stdin, nullptr, _IONBF, 0 );

    // redirect unbuffered STDERR to the console
    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen( hConHandle, "w" );
    *stderr = *fp;
    setvbuf( stderr, nullptr, _IONBF, 0 );

    // make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 
    // point to console as well
    ios::sync_with_stdio();
}*/
#else
void logUnitTestResult(cstr_t str)
{
    printf("%s", str);
}
#endif // _WIN32


class LogStreamBuffer : public StreamBuffer
{
public:
    virtual void write( const char *text, unsigned int length )
    {
        string    str;
        str.append(text, length);
        logUnitTestResult(str.c_str());
    }

};

void runUnitTest()
{
    // create the event manager and test controller
    CPPUNIT_NS::TestResult controller;
    string            testPath;

    // add a listener that colllects test result
    CPPUNIT_NS::TestResultCollector result;
    controller.addListener(&result);        

    // add a listener that print dots as test run.
#ifdef WIN32
    CPPUNIT_NS::TextTestProgressListener progress;
#else
    CPPUNIT_NS::BriefTestProgressListener progress;
#endif
    controller.addListener( &progress );      

    LogStreamBuffer buf;
    OStream streamLog(&buf);

    stdCOut().setBuffer(&buf);

    // add the top suite to the test runner
    CPPUNIT_NS::TestRunner runner;
    runner.addTest( CPPUNIT_NS::TestFactoryRegistry::getRegistry().makeTest() );   
    try
    {
        logUnitTestResult(("Running " + testPath).c_str());
        // std::cout << "Running "  <<  testPath;
        runner.run(controller, testPath);

        logUnitTestResult("\r\n");
        // std::cerr << std::endl;

        // print test in a compiler compatible format.
        CPPUNIT_NS::CompilerOutputter outputter(&result, streamLog);
        outputter.write(); 

        // Uncomment this for XML output
        //    std::ofstream file( "tests.xml" );
        //    CPPUNIT_NS::XmlOutputter xml( &result, file );
        //    xml.setStyleSheet( "report.xsl" );
        //    xml.write();
        //    file.close();
    }
    catch (std::invalid_argument &e)  // test path not resolved
    {
        logUnitTestResult("\r\nERROR: ");
        logUnitTestResult(e.what());
        logUnitTestResult("\r\n");
//         std::cerr  <<  std::endl  
//             <<  "ERROR: "  <<  e.what()
//             << std::endl;
        return;
    }

    stdCOut().setBuffer(nullptr);

#ifdef _WIN32
    if (result.wasSuccessful())
        setTimer(g_hWndLog, 0, 1000, nullptr);
#endif
}

#endif // _CPPUNIT_TEST
