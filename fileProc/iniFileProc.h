#ifndef FILEPROC_H
#define FILEPROC_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QtCore>
#include <fileProc/logfileproc.h>

// ----------  application-specific parameters structures ------------------------
typedef struct
{
    int top;
    int left;
    int width;
    int height;
    int posSave;
    int isMaximized;
} windowParamStruct;

static const int beaconNum = 8;

typedef struct
{
    float r;
    float g;
    float b;
} color3f;

typedef struct
{
    int number; // номер ма€чка
    float x, y, z;
    float rotY;
    bool isShining;
    bool isShown;
} beacon;

typedef struct
{
    float rotX;
    float rotY;
    float scale;
    color3f colorOn;
    color3f colorOff;

    float transX;
    float transY;

    float filterValue;

    float maxJointAngle;
} gl3dParamStruct;    // 3d view parameters

typedef struct
{
    QString ipInMCP;
    QString ipOutMCP;
    int portInMCP;
    int portOutMCP;
    int portInSTC;
    int portOutSTC;
    QString ipOutSTC;
} netParamStruct;

typedef struct
{
    bool LogEna;
    QString LogPath;
    bool LogTargetState;
    bool LogMovCtrl;
} logFileParamStruct;

// все параметры ini-файла
typedef struct
{
    QList<beacon> beacons;
    windowParamStruct windowParam;
    gl3dParamStruct cameraParam;
    netParamStruct netParam;
    logFileParamStruct logFileParam;
} applicationIniParametersStruct;

// -------------------------------------------------------------------------------
// структура дл€ хранени€ key = value
typedef struct
{
    QString name;
    QString value;
    QString paramType;
    bool isRead;
    bool mustReload;
} iniFileRecord;

// [структура], объедин€юща€ вместе несколько iniFileRecord
typedef struct
{
    QString name;
    quint8 paramNum;
    QList<iniFileRecord> records;
} iniFileSection;


class iniFileProc : public QObject
{
    Q_OBJECT
public:
    explicit iniFileProc(QObject *parent = 0);
    
    bool readIniFile();
    QString GetRecordFromFile(QString section, QString key);

    static windowParamStruct getWindowParams();
    static gl3dParamStruct getCameraParams();
    static netParamStruct getNetParams();
    static logFileParamStruct getLogFileParams();
    static QList<beacon> getBeaconParams();

    static void setWindowParams(windowParamStruct param);
    static void setCameraParams(gl3dParamStruct param);
    static void setNetParams(netParamStruct param);
    static void setLogFileParams(logFileParamStruct param);
    static void setBeaconParams(QList<beacon> param);

signals:
    void reloadBeaconList(QList<beacon> beacons);
    void reloadCameraParam(gl3dParamStruct cameraParam);
    void reloadLogParam(logFileParamStruct logFileParam);
    void reloadWindowParam(windowParamStruct windowParam);

public slots:
    bool OnReloadFromIniFile();

private:
    static void loadFromDefaultValues();
    static int findSectionIndex(QString str);
    static int findRecordIndex(QString str, int sectionIndex, QList<iniFileSection> fileStruct);
    static void writeIniFile(QList<iniFileSection> fileStruct);
    static QString getRecordValue(QList<iniFileSection> fileStruct, QString section, QString key);
    static void setRecordValue(QList<iniFileSection>* fileStruct, QString section, QString key, QString value);
    static void fillAppInitParameters(QList<iniFileSection> fileStruct);

    static bool logPathIsCorrect();

    static QFile* iniFile;

    static applicationIniParametersStruct appIniParameters;

    static QList<iniFileSection> iniFileStructure;


};

#endif // FILEPROC_H
