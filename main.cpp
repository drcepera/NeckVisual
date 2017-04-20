#include "mainwindow.h"
#include <QApplication>
#include <QObject>

#include "stdio.h"

#include "windows.h"
#include "tlhelp32.h"
#include "string.h"
#include "wchar.h"

static bool isApplicationAlreadyLaunched();

int main(int argc, char *argv[])
{
    if(isApplicationAlreadyLaunched())
        return 0;

    printf("0");
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
//    w.MoveWindow();
    
    QObject::connect(&w, SIGNAL(QuitApp()), &a, SLOT(closeAllWindows()));
    return a.exec();
}

static bool isApplicationAlreadyLaunched()
{
    HANDLE CONST hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    TCHAR thisProgNameTC[] = TEXT("RTC_RMV");
    int numOfProcessWithThisName = 0;

    PROCESSENTRY32 peProcessEntry;
    TCHAR szBuff[1024];
    DWORD dwTemp;
    HANDLE CONST hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(INVALID_HANDLE_VALUE == hSnapshot)
    {
        return true;
    }

    peProcessEntry.dwSize = sizeof(PROCESSENTRY32);
    Process32First(hSnapshot, &peProcessEntry);
    do
    {
        if(wcsstr(peProcessEntry.szExeFile, thisProgNameTC))
        {
            numOfProcessWithThisName++;
            if(numOfProcessWithThisName > 1)
            {
                CloseHandle(hSnapshot);
                return true;
            }
        }
    } while(Process32Next(hSnapshot, &peProcessEntry));

    CloseHandle(hSnapshot);
    return false;
}
