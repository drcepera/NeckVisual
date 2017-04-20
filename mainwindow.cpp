#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainWindow)
{
    iniFileProcesssor.readIniFile();

    ui->setupUi(this);

    verticalSplitter.setOrientation(Qt::Vertical);
    verticalSplitter.addWidget(&NeckModelWindow);
    //verticalSplitter.addWidget(&outText);

    ui->horizontalLayout->addWidget(&verticalSplitter);
//    verticalSplitter.setStretchFactor(0, 7);
//    verticalSplitter.setStretchFactor(1, 1);

    // outText make-up
    QPalette pal = outText.palette();
    pal.setColor(QPalette::Base, Qt::white);
    outText.setPalette(pal);
    outText.setTextColor(Qt::darkBlue);
    outText.setFontPointSize(10);
    outText.setReadOnly(true);

    connect(&socket,SIGNAL(commandReceived(QList<command>*)), &NeckModelWindow, SLOT(OnCommandReceived(QList<command>*)));
    connect(&socket, SIGNAL(QuitMainWindow()), SLOT(QuitMainWindow()));
    connect(&socket, SIGNAL(TextOut(QStringList)), SLOT(OntextOutExtern(QStringList)));

    connect(&socket, SIGNAL(ReloadFromIniCmdReceived()), &iniFileProcesssor, SLOT(OnReloadFromIniFile()));
    connect(&iniFileProcesssor, SIGNAL(reloadBeaconList(QList<beacon>)), &NeckModelWindow, SLOT(OnReloadBeaconList(QList<beacon>)));
    connect(&iniFileProcesssor, SIGNAL(reloadCameraParam(gl3dParamStruct)), &NeckModelWindow, SLOT(OnReloadCameraParam(gl3dParamStruct)));
    connect(&iniFileProcesssor, SIGNAL(reloadLogParam(logFileParamStruct)), &socket, SLOT(OnReloadLogParam(logFileParamStruct)));
    connect(&iniFileProcesssor, SIGNAL(reloadWindowParam(windowParamStruct)), this, SLOT(OnReloadWindowParam(windowParamStruct)));

    connect(&textOutputTimer, SIGNAL(timeout()), SLOT(OnTextOutTimerElapsed()));
    textOutputTimer.setSingleShot(true);

    connect(this, SIGNAL(TextOut(QStringList)), &NeckModelWindow, SLOT(glOutText(QStringList)));

    connect(&NeckModelWindow, SIGNAL(SendNewBeaconStateToSTC(uint)), &socket, SLOT(sendBeaconStateToSTC(uint)));

    windowParam = iniFileProc::getWindowParams();

    if(!socket.open())
    {
        logFileProcessor.writeToLog("Socket error");

        QStringList text = QStringList();
        text << "Warning! UDP socket couldn't be opened:" << ("ip: " + iniFileProc::getNetParams().ipInMCP)
                        << ("port: " + QString::number(iniFileProc::getNetParams().portInMCP)) << "You can change settings in \"RTC_RMV.ini\" file";
        emit TextOut(text);
        textOutputTimer.start(textOutputTimerDelayMs);

//        QMessageBox::warning(0, "Warning", "UDP socket could not be opened:\nip: " + iniFileProc::getNetParams().ipIn
//                             + "\nport: " + QString::number(iniFileProc::getNetParams().portIn) + "\n\nYou can change settings in \"RTC_RMV.ini\" file");
    }

//    QSettings settings("RTC", "RTC_RMV");
//    QByteArray qba = settings.value("geometry").toByteArray();
//    this->restoreGeometry(settings.value("geometry").toByteArray());

    executeWindowParams();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::MoveWindow()
{
    this->frameGeometry().setTopLeft(QPoint(0, 0));
}

void MainWindow::QuitMainWindow()
{
    // TODO: check SaveIni in INI file, [main] section
    QString mustSave = iniFileProcesssor.GetRecordFromFile("main", "SaveIni");
    if (mustSave != "error")
    {
        if (mustSave.toInt() != false)
        {
            socket.SaveLogFileParams();
            iniFileProc::setWindowParams(windowParam);
            NeckModelWindow.SaveParamsToIniFile();
        }
    }
    QuitApp();
}

void MainWindow::OnTextOutTimerElapsed()
{
    QStringList text;
    text.clear();
    emit TextOut(text);
}

void MainWindow::OntextOutExtern(QStringList text)
{
    emit TextOut(text);
    textOutputTimer.start(textOutputTimerDelayMs);
}

void MainWindow::OnReloadWindowParam(windowParamStruct param)
{
    windowParam = param;
    executeWindowParams();
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    if(this->isMaximized())
    {
        windowParam.isMaximized = 1;
//        iniFileProc::setWindowParams(windowParam);
        return;
    }

    windowParam.isMaximized = 0;
    windowParam.width = this->width();   // e->size().width();
    windowParam.height = this->height();  // e->size().height();
//    iniFileProc::setWindowParams(windowParam);
}

void MainWindow::moveEvent(QMoveEvent *e)
{
    if(this->isMaximized())
    {
        windowParam.isMaximized = 1;
//        iniFileProc::setWindowParams(windowParam);
        return;
    }

    windowParam.isMaximized = 0;
    windowParam.left = this->x();   //e->pos().x();
    windowParam.top = this->y(); // e->pos().y();
//    iniFileProc::setWindowParams(windowParam);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    QString mustSave = iniFileProcesssor.GetRecordFromFile("main", "SaveIni");
    if (mustSave != "error")
    {
        if (mustSave.toInt() != false)
        {
            socket.SaveLogFileParams();
            iniFileProc::setWindowParams(windowParam);
            NeckModelWindow.SaveParamsToIniFile();
        }
    }

    if(iniFileProcesssor.getWindowParams().posSave)
    {
        QSettings settings("RTC", "RTC_RMV");
        settings.setValue("geometry", saveGeometry());
    }
    QWidget::closeEvent(e);
}

void MainWindow::executeWindowParams()
{
    this->move(windowParam.left, windowParam.top);
    this->resize(windowParam.width, windowParam.height);
    if(windowParam.isMaximized)
    {
        this->showMaximized();
    }
}


