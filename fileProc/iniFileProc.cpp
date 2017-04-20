#include "inifileproc.h"
#include "logfileproc.h"

//typedef struct
//{
//    QString Name;
//    QString value;
//} iniFileRecord;

//typedef struct
//{
//    QString Name;
//    quint8 paramNum;
//    QList<iniFileRecord> records;
//} iniFileSection;

static const QString iniFileName = "RTC_RMV.ini";

// "main" params
static const iniFileRecord defaultMainSaveIni = { "SaveIni", "1", "int", 0, 1 };
static const iniFileSection defaultMainSection = { "main", 1, QList<iniFileRecord>() << defaultMainSaveIni };


// window params
static const iniFileRecord defaultWindowTop = { "top", "0", "int", 0, 1 };
static const iniFileRecord defaultWindowLeft = { "left", "0", "int", 0, 1 };
static const iniFileRecord defaultWindowWidth = { "width", "640", "int", 0, 1 };
static const iniFileRecord defaultWindowHigh = { "height", "480", "int", 0, 1 };
static const iniFileRecord defaultWindowIsMaximized = { "isMaximized", "0", "int", 0, 1 };
static const iniFileRecord defaultWindowPosSaveEnable = { "win_pos_save_ena", "1", "int", 0, 1 };
static const iniFileSection defaultWindowSection = { "window", 6, QList<iniFileRecord>() << defaultWindowTop
                                                     << defaultWindowLeft << defaultWindowWidth << defaultWindowHigh
                                                     << defaultWindowIsMaximized << defaultWindowPosSaveEnable };

// net params
static const iniFileRecord defaultNetIpIn = { "ipInMCP", "0.0.0.0", "string", 0, 0 };
static const iniFileRecord defaultNetIpOut = { "ipOutMCP", "127.0.0.1", "string", 0, 0 };
static const iniFileRecord defaultNetPortIn = { "portInMCP", "53201", "int", 0, 0 };
static const iniFileRecord defaultNetPortOut = { "portOutMCP", "53700", "int", 0, 0 };
static const iniFileRecord defaultNetPortSTCIn = { "portInSTC", "53201", "int", 0, 0 };
static const iniFileRecord defaultNetPortSTCOut = { "portOutSTC", "53700", "int", 0, 0 };
static const iniFileRecord defaultNetIpSTCOut = { "ipOutSTC", "127.0.0.1", "string", 0, 0 };
static const iniFileSection defaultNetSection = { "net", 7, QList<iniFileRecord>() << defaultNetIpIn << defaultNetIpOut << defaultNetPortIn <<
                                                  defaultNetPortOut << defaultNetPortSTCIn << defaultNetPortSTCOut << defaultNetIpSTCOut };

// 3d params
static const iniFileRecord default3dFilter = { "filter", "-0.2", "float", 0, 1 };
static const iniFileRecord default3dRotX = { "rotX", "15", "float", 0, 1 };
static const iniFileRecord default3dRotY = { "rotY", "15", "float", 0, 1 };
static const iniFileRecord default3dTransX = { "transX", "0.0", "float", 0, 1 };
static const iniFileRecord default3dTranY = { "transY", "0.0", "float", 0, 1 };
static const iniFileRecord default3dScale = { "scale", "1.0", "float", 0, 1 };
static const iniFileRecord default3dMaxJointAngle = { "maxJointAngle", "15.0", "float", 0, 1 };
static const iniFileRecord defaultColorOn = { "colorOn", "1.0; 1.0; 1.0", "color", 0, 1 };
static const iniFileRecord defaultColorOff = { "colorOff", "0.1; 0.1; 0.1", "color", 0, 1 };
static const iniFileSection default3dSection = { "3d", 9, QList<iniFileRecord>() << default3dFilter << default3dRotX << default3dRotY << default3dTransX
                                                 << default3dTranY << default3dScale << default3dMaxJointAngle << defaultColorOn << defaultColorOff };

// beacons params
static const iniFileRecord defaultBeaconLed = { "LED", "0", "int", 0, 1 };
static const iniFileRecord defaultBeacon0Coord = { "T0", "120.0; 120.0; 120.0", "beacon", 0, 1 };
static const iniFileRecord defaultBeacon1Coord = { "T1", "120.0; -120.0; 120.0", "beacon", 0, 1 };
static const iniFileRecord defaultBeacon2Coord = { "T2", "-120.0; 120.0; 120.0", "beacon", 0, 1 };
static const iniFileRecord defaultBeacon3Coord = { "T3", "-120.0; -120.0; 120.0", "beacon", 0, 1 };
static const iniFileRecord defaultBeacon4Coord = { "T4", "120.0; 120.0; 200.0", "beacon", 0, 1 };
static const iniFileRecord defaultBeacon5Coord = { "T5", "120.0; -120.0; 200.0", "beacon", 0, 1 };
static const iniFileRecord defaultBeacon6Coord = { "T6", "-120.0; 120.0; 200.0", "beacon", 0, 1 };
static const iniFileRecord defaultBeacon7Coord = { "T7", "-120.0; -120.0; 200.0", "beacon", 0, 1 };
static const iniFileSection defaultBeaconSection = { "beacons", beaconNum+1, QList<iniFileRecord>() << defaultBeaconLed
                                                     << defaultBeacon0Coord << defaultBeacon1Coord << defaultBeacon2Coord << defaultBeacon3Coord
                                                     << defaultBeacon4Coord << defaultBeacon5Coord << defaultBeacon6Coord << defaultBeacon7Coord };

// log file
static const iniFileRecord defaultLogEna = { "LogEna", "1", "int", 0, 1 };
static const iniFileRecord defaultLogPath = { "LogPath", "logout/", "string", 0, 0 };
static const iniFileRecord defaultLogTargetState = { "LogTargetState", "1", "int", 0, 1 };
static const iniFileRecord defaultLogMovCtrl = { "LogMovCtrl", "1", "int", 0, 1 };
static const iniFileSection defaultLogFilesSection = { "logFile", 4, QList<iniFileRecord>() << defaultLogEna << defaultLogPath
                                                     << defaultLogTargetState << defaultLogMovCtrl};

static const QList<iniFileSection> defaultIniFileStruct = QList<iniFileSection>() << defaultMainSection<< defaultWindowSection << defaultNetSection << default3dSection
                                                                                     << defaultLogFilesSection << defaultBeaconSection;
applicationIniParametersStruct iniFileProc::appIniParameters;
QList<iniFileSection> iniFileProc::iniFileStructure;
QFile* iniFileProc::iniFile;

iniFileProc::iniFileProc(QObject *parent) :
    QObject(parent)
{
}

bool iniFileProc::readIniFile()
{
    // TODO: if file doesn't exist or is wrong - logout

    iniFile = new QFile(iniFileName);
    if(!(QFile::exists(iniFileName)))
    {
        logFileProc::writeToLog("INI file doesn't exist - load from default settings");
        loadFromDefaultValues();
        return true;
    }
    iniFile->open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream iniText(iniFile);

    int currentSectionIndex = -1;
    int currentRecordIndex = -1;

    // create iniFileStruct
    iniFileStructure = defaultIniFileStruct;
//    for(int i=0; i<iniFileStructure.size(); i++)
//    {
//        for(int j=0; j<iniFileStructure[i].paramNum; j++)
//        {
//            iniFileStructure[i].records[j].value = "";
//        }
//    }

    // clear beacon section except LED
    int beaconSectionIndex = findSectionIndex("beacons");
    for(int i=1; i<iniFileStructure[beaconSectionIndex].paramNum; i++)
    {
        iniFileStructure[beaconSectionIndex].records[i].value = "";
    }

    QString line;
    while( !iniText.atEnd())
    {
        line = iniText.readLine();

        // check if string section identification include
        if(line[0] == '[')
        {
            line.remove('[');
            line.remove(']');
            currentSectionIndex = findSectionIndex(line);
            continue;
        }

        if(currentSectionIndex == -1)   // no section choosen to read param
            continue;

        // read key=value
        QStringList keyValueCouple = line.split('=');
        if(keyValueCouple.size() != 2)
            continue;
        // check key
        if((currentRecordIndex = findRecordIndex(keyValueCouple[0].remove(' '), currentSectionIndex, defaultIniFileStruct))== -1)   // no record appropriate to read param
            continue;
        // if beacon - add record to iniSection
        if((currentSectionIndex == findSectionIndex("beacons")) && (keyValueCouple[0].data()[0] == 'T'))
        {
//            iniFileRecord beaconRecord;
//            beaconRecord.name = keyValueCouple[0];
//            beaconRecord.paramType = "beacon";
//            beaconRecord.value = "";
//            iniFileStructure[beaconSectionIndex].records << beaconRecord;
//            iniFileStructure[beaconSectionIndex].paramNum++;
        }
        iniFileStructure[currentSectionIndex].records[currentRecordIndex].value = keyValueCouple[1].remove(' '); //read value
        iniFileStructure[currentSectionIndex].records[currentRecordIndex].isRead = true;
    }
    iniFile->close();

    // check log file Path correctness
    int logSectionIndex = findSectionIndex("logFile");
    int logRecordIndex = findRecordIndex("LogPath", logSectionIndex, defaultIniFileStruct);
    if(!logPathIsCorrect())
    {
        QString defaultPath = defaultIniFileStruct[logSectionIndex].records[logRecordIndex].value;
        iniFileStructure[logSectionIndex].records[logRecordIndex].value = defaultPath;
        logFileProc::Open(defaultPath);
        logFileProc::writeToLog("Wrong log file path - log file was in default path " + defaultPath + " created");
    }
    else
    {
        logFileProc::Open(iniFileStructure[logSectionIndex].records[logRecordIndex].value);
    }


    // don't even check, that all parameters read - just fill missing from default

//    // check all parameters have read, and fill missing parameters from defaults
//    for(int i=0; i<iniFileStructure.size(); i++)
//    {
//        for(int j=0; j<iniFileStructure[i].paramNum; j++)
//        {
//            if(iniFileStructure[i].records[j].value == "")
//            {
//                // if beacon is not read - it's Ok, cause not all beacons must be
//                if(iniFileStructure[i].records[j].paramType == "beacon")
//                {
//                    continue;
//                }

//                logFileProc::writeToLog("INI file couldn't be read - load from default settings");
//                loadFromDefaultValues();
//                return true;
//            }
//        }
//    }

    logFileProc::writeToLog("INI file read successfully\n");
    fillAppInitParameters(iniFileStructure);

//    writeIniFile(iniFileStructure);

    return true;
}


QString iniFileProc::GetRecordFromFile(QString section, QString key)
{
    iniFile = new QFile(iniFileName);
    if(!(QFile::exists(iniFileName)))
    {
        logFileProc::writeToLog("Cannot find INI file");
        return "error";
    }
    if(iniFile->open(QIODevice::ReadOnly | QIODevice::Text) != true )
    {
        logFileProc::writeToLog("Cannot open INI file");
        return "error";
    }

    QTextStream iniText(iniFile);

    QString line;
    while( !iniText.atEnd())
    {
        line = iniText.readLine();

        // check if string section identification include
        if(line[0] == '[')
        {
            line.remove('[');
            line.remove(']');
            if(line != section)
            {
                continue;
            }
        }

        // read key=value
        QStringList keyValueCouple = line.split('=');
        if(keyValueCouple.size() != 2)
            continue;
        // check key
        keyValueCouple[0].remove(' ');
        keyValueCouple[1].remove(' ');
        if( keyValueCouple[0] == key)   // no section choosen to read param
        {
            iniFile->close();
            return keyValueCouple[1];
        }
        else
        {
            continue;
        }

    }
    iniFile->close();
    return "error";
}

int iniFileProc::findSectionIndex(QString str)
{
    for(int i=0; i<defaultIniFileStruct.size(); i++)
    {
        if(str == defaultIniFileStruct[i].name)
            return i;
    }
    return -1;
}

int iniFileProc::findRecordIndex(QString str, int sectionIndex, QList<iniFileSection> fileStruct)
{
    for(int i=0; i<defaultIniFileStruct[sectionIndex].paramNum; i++)
    {
        if(str == defaultIniFileStruct[sectionIndex].records[i].name)
            return i;
    }
    return -1;
}

void iniFileProc::writeIniFile(QList<iniFileSection> fileStruct)
{
    iniFile->open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream iniText(iniFile);
    for(int i=0; i<fileStruct.size(); i++)
    {
        iniText << "[" << fileStruct[i].name << "]\n";
        for(int j=0; j<fileStruct[i].paramNum; j++)
        {
            // don't write empty beacons records
            if((i == findSectionIndex("beacons")) && (fileStruct[i].records[j].name.data()[0] == 'T'))
            {
                if(fileStruct[i].records[j].value == "")
                {
                    continue;
                }
            }
            iniText << fileStruct[i].records[j].name << " = " << fileStruct[i].records[j].value << "\n";
        }
        iniText << "\n";
    }

    iniText.flush();
    iniFile->close();
}

QString iniFileProc::getRecordValue(QList<iniFileSection> fileStruct, QString section, QString key)
{
    for(int i=0; i<fileStruct.size(); i++)
    {
        if(fileStruct[i].name == section)
        {
            for(int j=0; j<fileStruct[i].paramNum; j++)
            {
                if(fileStruct[i].records[j].name == key)
                    return fileStruct[i].records[j].value;
            }
        }
    }
    return "";
}

void iniFileProc::setRecordValue(QList<iniFileSection> *fileStruct, QString section, QString key, QString value)
{
    for(int i=0; i<fileStruct->size(); i++)
    {
        if((*fileStruct)[i].name == section)
        {
            for(int j=0; j<(*fileStruct)[i].paramNum; j++)
            {
                if((*fileStruct)[i].records[j].name == key)
                {
                    (*fileStruct)[i].records[j].value = value;
                    return;
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
/* Function fills parameters of application from list (may be default list or list read from file for ex.)
 */
void iniFileProc::fillAppInitParameters(QList<iniFileSection> fileStruct)
{
    appIniParameters.windowParam.top = getRecordValue(fileStruct, "window", "top").toInt();
    appIniParameters.windowParam.left = getRecordValue(fileStruct, "window", "left").toInt();
    appIniParameters.windowParam.width = getRecordValue(fileStruct, "window", "width").toInt();
    appIniParameters.windowParam.height = getRecordValue(fileStruct, "window", "height").toInt();
    appIniParameters.windowParam.isMaximized = getRecordValue(fileStruct, "window", "isMaximized").toInt();
    appIniParameters.windowParam.posSave = getRecordValue(fileStruct, "window", "win_pos_save_ena").toInt();

    appIniParameters.netParam.ipInMCP = getRecordValue(fileStruct, "net", "ipInMCP");
    appIniParameters.netParam.ipOutMCP = getRecordValue(fileStruct, "net", "ipOutMCP");
    appIniParameters.netParam.portInMCP = getRecordValue(fileStruct, "net", "portInMCP").toInt();
    appIniParameters.netParam.portOutMCP = getRecordValue(fileStruct, "net", "portOutMCP").toInt();
    appIniParameters.netParam.portInSTC = getRecordValue(fileStruct, "net", "portInSTC").toInt();
    appIniParameters.netParam.portOutSTC = getRecordValue(fileStruct, "net", "portOutSTC").toInt();
    appIniParameters.netParam.ipOutSTC = getRecordValue(fileStruct, "net", "ipOutSTC");

    appIniParameters.cameraParam.filterValue = getRecordValue(fileStruct, "3d", "filter").toFloat();
    appIniParameters.cameraParam.rotX = getRecordValue(fileStruct, "3d", "rotX").toFloat();
    appIniParameters.cameraParam.rotY = getRecordValue(fileStruct, "3d", "rotY").toFloat();
    appIniParameters.cameraParam.transX = getRecordValue(fileStruct, "3d", "transX").toFloat();
    appIniParameters.cameraParam.transY = getRecordValue(fileStruct, "3d", "transY").toFloat();
    appIniParameters.cameraParam.scale = getRecordValue(fileStruct, "3d", "scale").toFloat();
    appIniParameters.cameraParam.maxJointAngle = getRecordValue(fileStruct, "3d", "maxJointAngle").toFloat();

    appIniParameters.logFileParam.LogEna = getRecordValue(fileStruct, "logFile", "LogEna").toInt();
    appIniParameters.logFileParam.LogTargetState = getRecordValue(fileStruct, "logFile", "LogTargetState").toInt();
    appIniParameters.logFileParam.LogMovCtrl = getRecordValue(fileStruct, "logFile", "LogMovCtrl").toInt();
    appIniParameters.logFileParam.LogPath = getRecordValue(fileStruct, "logFile", "LogPath");

    color3f color;
    QStringList colorCompon = getRecordValue(fileStruct, "3d", "colorOn").split(';');
    color.r = colorCompon[0].remove(' ').toFloat();
    color.g = colorCompon[1].remove(' ').toFloat();
    color.b = colorCompon[2].remove(' ').toFloat();
    appIniParameters.cameraParam.colorOn = color;

    colorCompon = getRecordValue(fileStruct, "3d", "colorOff").split(';');
    color.r = colorCompon[0].remove(' ').toFloat();
    color.g = colorCompon[1].remove(' ').toFloat();
    color.b = colorCompon[2].remove(' ').toFloat();
    appIniParameters.cameraParam.colorOff = color;

    quint8 LED = getRecordValue(fileStruct, "beacons", "LED").toUShort();
    appIniParameters.beacons.clear();
    int beaconSectionIndex = findSectionIndex("beacons");
    beacon newBeacon;
    for(int i=0; i<beaconNum; i++)
    {
        newBeacon.number = i;

        // если маяк прочитан из ini - показать его
        if(getRecordValue(fileStruct, "beacons", "T" + QString::number(i)) != "")
        {
            QStringList fields = getRecordValue(fileStruct, "beacons", "T" + QString::number(i)).split(';');
            if(fields.size() != 3)
            {
                logFileProc::writeToLog("Wrong beacon position in ini!");
                return;
            }

            newBeacon.x = fields[0].remove(' ').toFloat();
            newBeacon.y = fields[1].remove(' ').toFloat();
            newBeacon.z = fields[2].remove(' ').toFloat();

            newBeacon.isShining = (LED & (1 << i));
            newBeacon.isShown = 1;
        }
        else    // иначе не выводить маяк
        {
            newBeacon.x = newBeacon.y = newBeacon.z = 0;
            newBeacon.isShining = 0;
            newBeacon.isShown = 0;
        }

        appIniParameters.beacons.append(newBeacon);
    }
}

bool iniFileProc::logPathIsCorrect()
{
    int logSectionIndex = findSectionIndex("logFile");
    int logRecordIndex = findRecordIndex("LogPath", logSectionIndex, defaultIniFileStruct);
    QString path = iniFileStructure[logSectionIndex].records[logRecordIndex].value;

    if(!QDir(path).exists())
    {
        if(!QDir(path).mkpath("."))
        {
             return false;
        }
    }

    return true;
}

QList<beacon> iniFileProc::getBeaconParams()
{
    return appIniParameters.beacons;
}

logFileParamStruct iniFileProc::getLogFileParams()
{
    return appIniParameters.logFileParam;
}

windowParamStruct iniFileProc::getWindowParams()
{
    return appIniParameters.windowParam;
}

gl3dParamStruct iniFileProc::getCameraParams()
{
    return appIniParameters.cameraParam;
}

netParamStruct iniFileProc::getNetParams()
{
    return appIniParameters.netParam;
}

void iniFileProc::setWindowParams(windowParamStruct param)
{
    appIniParameters.windowParam = param;

    setRecordValue(&iniFileStructure, "window", "top", QString::number(param.top));
    setRecordValue(&iniFileStructure, "window", "left", QString::number(param.left));
    setRecordValue(&iniFileStructure, "window", "width", QString::number(param.width));
    setRecordValue(&iniFileStructure, "window", "height", QString::number(param.height));
    setRecordValue(&iniFileStructure, "window", "isMaximized", QString::number(param.isMaximized));
    setRecordValue(&iniFileStructure, "window", "win_pos_save_ena", QString::number(param.posSave));

    QString mustSave = getRecordValue(iniFileStructure,"main", "SaveIni");
    if (mustSave != "")
    {
        if (mustSave.toInt() != false)
        {
            writeIniFile(iniFileStructure);
        }
    }
}

void iniFileProc::setCameraParams(gl3dParamStruct param)
{
    appIniParameters.cameraParam = param;

    setRecordValue(&iniFileStructure, "3d", "filter", QString::number(param.filterValue));
    setRecordValue(&iniFileStructure, "3d", "rotX", QString::number(param.rotX));
    setRecordValue(&iniFileStructure, "3d", "rotY", QString::number(param.rotY));
    setRecordValue(&iniFileStructure, "3d", "transX", QString::number(param.transX));
    setRecordValue(&iniFileStructure, "3d", "transY", QString::number(param.transY));
    setRecordValue(&iniFileStructure, "3d", "scale", QString::number(param.scale));
    setRecordValue(&iniFileStructure, "3d", "maxJointAngle", QString::number(param.maxJointAngle));

    QString colorString;
    colorString = QString::number(param.colorOn.r,'f',2) + "; " + QString::number(param.colorOn.g,'f',2)
            +  "; " + QString::number(param.colorOn.b,'f',2);
    setRecordValue(&iniFileStructure, "3d", "colorOn", colorString);
    colorString = QString::number(param.colorOff.r,'f',2) +  "; " + QString::number(param.colorOff.g,'f',2)
            + "; " + QString::number(param.colorOff.b,'f',2);
    setRecordValue(&iniFileStructure, "3d", "colorOff", colorString);

    QString mustSave = getRecordValue(iniFileStructure,"main", "SaveIni");
    if (mustSave != "")
    {
        if (mustSave.toInt() != false)
        {
            writeIniFile(iniFileStructure);
        }
    }
}

void iniFileProc::setNetParams(netParamStruct param)
{
    appIniParameters.netParam = param;

    setRecordValue(&iniFileStructure, "net", "ipInMCP", param.ipInMCP);
    setRecordValue(&iniFileStructure, "net", "ipOutMCP", param.ipOutMCP);
    setRecordValue(&iniFileStructure, "net", "portInMCP", QString::number(param.portInMCP));
    setRecordValue(&iniFileStructure, "net", "portOutMCP", QString::number(param.portOutMCP));
    setRecordValue(&iniFileStructure, "net", "portInSTC", QString::number(param.portInSTC));
    setRecordValue(&iniFileStructure, "net", "portOutSTC", QString::number(param.portOutSTC));
    setRecordValue(&iniFileStructure, "net", "ipOutSTC", param.ipOutSTC);

    QString mustSave = getRecordValue(iniFileStructure,"main", "SaveIni");
    if (mustSave != "")
    {
        if (mustSave.toInt() != false)
        {
            writeIniFile(iniFileStructure);
        }
    }
}

void iniFileProc::setLogFileParams(logFileParamStruct param)
{
    appIniParameters.logFileParam = param;

    setRecordValue(&iniFileStructure, "logFile", "LogEna",QString::number(param.LogEna));
    setRecordValue(&iniFileStructure, "logFile", "LogPath", param.LogPath);
    setRecordValue(&iniFileStructure, "logFile", "LogTargetState", QString::number(param.LogTargetState));
    setRecordValue(&iniFileStructure, "logFile", "LogMovCtrl", QString::number(param.LogMovCtrl));

    QString mustSave = getRecordValue(iniFileStructure,"main", "SaveIni");
    if (mustSave != "")
    {
        if (mustSave.toInt() != false)
        {
            writeIniFile(iniFileStructure);
        }
    }
}

void iniFileProc::setBeaconParams(QList<beacon> param)
{
    appIniParameters.beacons = param;
    int beaconSectionIndex = findSectionIndex("beacons");

    // clear all beacons records "Tx=xxx;yyy;zzz", leave "LED=defaultLed"
    iniFileStructure[beaconSectionIndex].records.clear();
    iniFileStructure[beaconSectionIndex].records << defaultBeaconLed;
    iniFileStructure[beaconSectionIndex].paramNum = 1;

    quint8 LED = 0;
    for(int i=0; i<beaconNum; i++)
    {
        // fill only beacon structures that are shown
        if(param[i].isShown)
        {
            iniFileStructure[beaconSectionIndex].paramNum++;

            iniFileRecord beaconRecord;
            beaconRecord.name = "T" + QString::number(i);
            beaconRecord.paramType = "beacon";

            QString beaconCoordString;
            beaconCoordString = QString::number(param[i].x,'f',1) + "; " + QString::number(param[i].y,'f',1) + "; "
                    + QString::number(param[i].z,'f',1);
            beaconRecord.value = beaconCoordString;

            iniFileStructure[beaconSectionIndex].records << beaconRecord;

            setRecordValue(&iniFileStructure, "beacons", "T" + QString::number(i), beaconCoordString);
            if(param[i].isShining)
            {
                LED += (1<<i);
            }
        }
    }

    setRecordValue(&iniFileStructure, "beacons", "LED", QString::number(LED));

    QString mustSave = getRecordValue(iniFileStructure,"main", "SaveIni");
    if (mustSave != "")
    {
        if (mustSave.toInt() != false)
        {
            writeIniFile(iniFileStructure);
        }
    }
}

bool iniFileProc::OnReloadFromIniFile()
{
    // TODO: if file doesn't exist or is wrong - logout

    iniFile = new QFile(iniFileName);
    if(!(QFile::exists(iniFileName)))
    {
        logFileProc::writeToLog("INI file doesn't exist - load from default settings");
        return true;
    }
    iniFile->open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream iniText(iniFile);

    int currentSectionIndex = -1;
    int currentRecordIndex = -1;

    iniFileStructure = defaultIniFileStruct;

    // clear beacon section except LED
    int beaconSectionIndex = findSectionIndex("beacons");
    for(int i=1; i<iniFileStructure[beaconSectionIndex].paramNum; i++)
    {
        iniFileStructure[beaconSectionIndex].records[i].value = "";
    }

    QString line;
    while( !iniText.atEnd())
    {
        line = iniText.readLine();

        // check if string section identification include
        if(line[0] == '[')
        {
            line.remove('[');
            line.remove(']');
            currentSectionIndex = findSectionIndex(line);
            continue;
        }

        if(currentSectionIndex == -1)   // no section choosen to read param
            continue;

        // read key=value
        QStringList keyValueCouple = line.split('=');
        if(keyValueCouple.size() != 2)
            continue;
        // check key
        if((currentRecordIndex = findRecordIndex(keyValueCouple[0].remove(' '), currentSectionIndex, defaultIniFileStruct))== -1)   // no record appropriate to read param
            continue;
        // if beacon - add record to iniSection
        if((currentSectionIndex == findSectionIndex("beacons")) && (keyValueCouple[0].data()[0] == 'T'))
        {
//            iniFileRecord beaconRecord;
//            beaconRecord.name = keyValueCouple[0];
//            beaconRecord.paramType = "beacon";
//            beaconRecord.value = "";
//            iniFileStructure[beaconSectionIndex].records << beaconRecord;
//            iniFileStructure[beaconSectionIndex].paramNum++;
        }
        iniFileStructure[currentSectionIndex].records[currentRecordIndex].value = keyValueCouple[1].remove(' '); //read value
        iniFileStructure[currentSectionIndex].records[currentRecordIndex].isRead = true;
    }
    iniFile->close();

    // check log file Path correctness
    int logSectionIndex = findSectionIndex("logFile");
    int logRecordIndex = findRecordIndex("LogPath", logSectionIndex, defaultIniFileStruct);
    if(!logPathIsCorrect())
    {
        QString defaultPath = defaultIniFileStruct[logSectionIndex].records[logRecordIndex].value;
        iniFileStructure[logSectionIndex].records[logRecordIndex].value = defaultPath;
        logFileProc::Open(defaultPath);
        logFileProc::writeToLog("Wrong log file path - log file was in default path " + defaultPath + " created");
    }
    else
    {
        logFileProc::Open(iniFileStructure[logSectionIndex].records[logRecordIndex].value);
    }

    logFileProc::writeToLog("INI file values reloaded successfully\n");
    fillAppInitParameters(iniFileStructure);
//    writeIniFile(iniFileStructure);

    emit reloadBeaconList(appIniParameters.beacons);
    emit reloadCameraParam(appIniParameters.cameraParam);
    emit reloadLogParam(appIniParameters.logFileParam);
    emit reloadWindowParam(appIniParameters.windowParam);

    return true;
}

void iniFileProc::loadFromDefaultValues()
{
    writeIniFile(defaultIniFileStruct);
    logFileProc::writeToLog("INI default file created\n");
    iniFileStructure = defaultIniFileStruct;
    fillAppInitParameters(iniFileStructure);
}


