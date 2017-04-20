#ifndef LOGFILEPROC_H
#define LOGFILEPROC_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QTime>

class logFileProc : public QObject
{
    Q_OBJECT
public:
    explicit logFileProc(QObject *parent = 0);
    ~logFileProc()
    {
        logFile.close();
    }
    
    static void Open(QString path);
    static void writeToLog(QString str);

signals:
    
public slots:

private:
    static QFile logFile;
    static QTextStream logStream;
};

#endif // LOGFILEPROC_H
