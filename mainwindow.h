#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QtGui>
#include <QMainWindow>
#include <GLWindow/glwindow.h>
#include <SocketUDP/socketudp.h>
#include <QTextEdit>
#include <fileProc/iniFileProc.h>
#include <fileProc/logfileproc.h>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void MoveWindow();

public slots:
    void QuitMainWindow();
    void OnTextOutTimerElapsed();
    void OntextOutExtern(QStringList text);
    void OnReloadWindowParam(windowParamStruct param);

signals:
    void QuitApp();
    void TextOut(QStringList text);
    
private:
    void resizeEvent(QResizeEvent *e);
    void moveEvent(QMoveEvent *e);
    void closeEvent(QCloseEvent *e);
    void executeWindowParams();

    Ui::MainWindow *ui;
    QSplitter verticalSplitter;
    QTextEdit outText;

    GLWindow NeckModelWindow;

    SocketUDP socket;

    iniFileProc iniFileProcesssor;
    logFileProc logFileProcessor;

    windowParamStruct windowParam;

    QTimer textOutputTimer;
    static const int textOutputTimerDelayMs = 10000;
};

#endif // MAINWINDOW_H
