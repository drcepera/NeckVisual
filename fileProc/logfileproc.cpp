#include "logfileproc.h"

static const QString LogPath = "logout/";

QFile logFileProc::logFile;
QTextStream logFileProc::logStream;

logFileProc::logFileProc(QObject *parent) :
    QObject(parent)
{
    // TODO: error in log file creation process
}

void logFileProc::Open(QString path)
{
    QString fileName = "RTC_RMV_" + QDate::currentDate().toString(Qt::ISODate).replace("-","") + "_"
            + QTime::currentTime().toString().replace(":", "") + ".log";
    logFile.setFileName(path + fileName);
    if(!logFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        logFile.setFileName(fileName);
        if(logFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            logStream.setDevice(&logFile);
            writeToLog("Wrong log file path - log file was in execution folder created");
        }
        return;
    }
    logStream.setDevice(&logFile);
}

void logFileProc::writeToLog(QString str)
{
    logStream << QTime::currentTime().toString() <<  "; " << str << "\n";
    logStream.flush();
}
