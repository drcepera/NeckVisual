#include "socketudp.h"
#include "fileProc/logfileproc.h"
#include <QMessageBox>

static const int insertSpecialSymbol = -1;
static const int flagsOffset = 6;
static const int cmdOffset1 = 10;
static const int cmdOffset2 = 11;

static const int cmdSendParam = 0x0202;
static const int cmdReloadIni = 0x0207;
static const int cmdCloseApp = 0x0001;
static const int cmdCloseAppAccepted = 0x0002;

static quint16 STCPacketSeqNum = 1;

typedef struct
{
    int offset;
    char byteValue;
    QString valueType;
} byteKeyInUdp;

static const byteKeyInUdp byteKeyCheckMatrix[] = {  {0, 'K', "str"},   // Magic
                                                    {1, '2', "str"},
                                                    {2, 'A', "str"},
                                                    {3, 'C', "str"},
                                                    {insertSpecialSymbol, ' ', ""},
                                                    {insertSpecialSymbol, '-', ""},
                                                    {insertSpecialSymbol, '>', ""},
                                                    {5, 23, "int"}};    // DstID
static const quint16 dataOffsetInPacket = 12;

SocketUDP::SocketUDP(QObject *parent) :
    QObject(parent)
{
//    QByteArray txPacket;
//    uint st = 260;
//    fillBeaconStatePacket(&txPacket, st);

    udpSocket = 0;
}

void SocketUDP::SaveLogFileParams()
{
    iniFileProc::setLogFileParams(logParam);
}

static QList<command> receivedCommandList;

bool SocketUDP::open()
{
    netParam = iniFileProc::getNetParams();
    logParam = iniFileProc::getLogFileParams();

    udpSocket = new QUdpSocket(this);
    if(!udpSocket->bind(QHostAddress(netParam.ipInMCP), netParam.portInMCP))
    {
        return false;
    }

    connect(udpSocket, SIGNAL(readyRead()), SLOT(readUdp()));
    return true;
}

void SocketUDP::close()
{
    udpSocket->abort();
}


void SocketUDP::readUdp()
{
    while(udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize((udpSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort;

        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        readDatagramRoutine(datagram);
    }
}

void SocketUDP::sendBeaconStateToSTC(uint state)
{
    if(logParam.LogTargetState)
    {
        // write state to log
        QString logStr = "";
        logStr += QString("") + "RMV -> STC" + "\t" + "New beacon state: 0b" +
                QString("%1").arg((state & 0xFF), 8, 2, QChar('0'));
        logFileProc::writeToLog(logStr);
    }

    // Send packet to STC
    QUdpSocket socketTx(this);
    socketTx.connectToHost(QHostAddress(netParam.ipOutSTC), netParam.portOutSTC, QIODevice::ReadWrite);
    QByteArray txPacket;
    fillBeaconStatePacket(&txPacket, state);
    if(socketTx.writeDatagram(txPacket.data(), txPacket.size(), QHostAddress(netParam.ipOutSTC), netParam.portOutSTC) == -1)
    {
        QStringList text;
        text << "Warning! Cannot send UDP datagram to " + iniFileProc::getNetParams().ipOutSTC <<
                "You can change settings in \"RTC_RMV.ini\" file";
        emit TextOut(text);
//            QMessageBox::warning(0, "Warning", "Cannot send UDP datagram to " + iniFileProc::getNetParams().ipOut
//                                 + "\n\nYou can change settings in \"RTC_RMV.ini\" file");
        return;
    }
    socketTx.close();
}

void SocketUDP::OnReloadLogParam(logFileParamStruct param)
{
    logParam.LogEna = param.LogEna;
    logParam.LogTargetState = param.LogTargetState;
    logParam.LogMovCtrl = param.LogMovCtrl;
}

void SocketUDP::readDatagramRoutine(QByteArray datagram)
{
    QString logStr = "";

    //  check datagramByteKeys
    for(uint i=0; i<sizeof(byteKeyCheckMatrix)/sizeof(byteKeyInUdp); i++)
    {
        if (byteKeyCheckMatrix[i].offset == insertSpecialSymbol)    // formating logout with spaces and our insert characters
        {
            logStr += byteKeyCheckMatrix[i].byteValue;
            continue;
        }

        if( byteKeyCheckMatrix[i].offset > datagram.size()-1)  // datagram too short is
        {
            logFileProc::writeToLog(logStr);
            return;
        }

        // convert datagram bytes to string depending on data types
        if(byteKeyCheckMatrix[i].valueType == "str")
        {
            logStr += QString(datagram.data()[byteKeyCheckMatrix[i].offset]);
        }
        else if (byteKeyCheckMatrix[i].valueType == "int")
        {
            logStr += QString::number(datagram.data()[byteKeyCheckMatrix[i].offset]);
        }

        // wrong datagram head
        if(datagram.data()[byteKeyCheckMatrix[i].offset] != byteKeyCheckMatrix[i].byteValue)
        {
            logFileProc::writeToLog(logStr);
            return;
        }
    }

    int cmd = ( datagram.data()[cmdOffset2] << 8 ) | datagram.data()[cmdOffset1];
    if (cmd == cmdCloseApp)
    {
        // App Close
        logStr += " cmd closeApp";
        logFileProc::writeToLog(logStr);
        // send packet
        QUdpSocket socketTx(this);
        socketTx.connectToHost(QHostAddress(netParam.ipOutMCP), netParam.portOutMCP, QIODevice::ReadWrite);
        QByteArray txPacket;
        fillTxCloseCmdPacket(&txPacket);
        if(socketTx.writeDatagram(txPacket.data(), txPacket.size(), QHostAddress(netParam.ipOutMCP), netParam.portOutMCP) == -1)
        {
            QStringList text;
            text << "Warning! Cannot send UDP datagram to " + iniFileProc::getNetParams().ipOutMCP <<
                    "You can change settings in \"RTC_RMV.ini\" file";
            emit TextOut(text);
//            QMessageBox::warning(0, "Warning", "Cannot send UDP datagram to " + iniFileProc::getNetParams().ipOut
//                                 + "\n\nYou can change settings in \"RTC_RMV.ini\" file");
            return;
        }
        socketTx.close();
        emit QuitMainWindow();

        return;
    }

    logStr += " cmd 0x" + QString("%1").arg(cmd, 4, 16, QChar('0')).toUpper();
    if(cmd == cmdReloadIni)
    {
        emit ReloadFromIniCmdReceived();
        logFileProc::writeToLog(logStr + " Reload INI file");
        return;
    }

    if (cmd != cmdSendParam)
    {
        logFileProc::writeToLog(logStr + " wrong cmd");
        return;
    }

    int hidenCommandsNumber = 0;    // number of commands received must not be output in log file

    parsePacketData(datagram);
    for(int i=0; i<receivedCommandList.size(); i++)
    {
        // special cases for beacon state and move device (X/Y) commands logging depending on [log file] parameters
        if( (receivedCommandList[i].Key == "X") || (receivedCommandList[i].Key == "Y") )
        {
            if(logParam.LogMovCtrl)
            {
                logStr += " " + receivedCommandList[i].Key + "=" + receivedCommandList[i].Value;
            }
            else
            {
                hidenCommandsNumber++;
            }
            continue;
        }
        else if((receivedCommandList[i].Key[0] == 'T') || (receivedCommandList[i].Key == "LED"))
        {
            if(logParam.LogTargetState)
            {
                logStr += " " + receivedCommandList[i].Key + "=" + receivedCommandList[i].Value;
            }
            else
            {
                hidenCommandsNumber++;
            }
            continue;
        }

        // process "LogTargetState" and "LogMovCtrl" command keys
        else if(receivedCommandList[i].Key.toUpper() == QString("LogMovCtrl").toUpper())
        {
            int newval;
            try
            {
                newval = receivedCommandList[i].Value.toInt();
            }
            catch(...)
            {
                newval = (int)logParam.LogMovCtrl;
            }
            logParam.LogMovCtrl = (bool)newval;
//            iniFileProc::setLogFileParams(logParam);
        }
        else if(receivedCommandList[i].Key.toUpper() == QString("LogTargetState").toUpper())
        {
            int newval;
            try
            {
                newval = receivedCommandList[i].Value.toInt();
            }
            catch(...)
            {
                newval = (int)logParam.LogTargetState;
            }
            logParam.LogTargetState = (bool)newval;
//            iniFileProc::setLogFileParams(logParam);
        }

        logStr += " " + receivedCommandList[i].Key + "=" + receivedCommandList[i].Value;
    }

    if(hidenCommandsNumber < receivedCommandList.size())
    {
        logFileProc::writeToLog(logStr);
    }

    emit commandReceived(&receivedCommandList);
}

void SocketUDP::parsePacketData(QByteArray datagram)
{
    receivedCommandList.clear();

    int cntCmdByte = 0;
    char cmd[40];

    command rcvCommand;

    for(int i = dataOffsetInPacket; i<datagram.size(); i++)
    {
        if(datagram.data()[i] == '=')
        {
            // end command key
            cmd[cntCmdByte] = 0;
            rcvCommand.Key = QString(cmd);
            cntCmdByte = 0;
            continue;
        }
        else if(datagram.data()[i] == 13)    // <CR>
        {
            // end command value
            cmd[cntCmdByte] = 0;
            cntCmdByte = 0;
            rcvCommand.Value = QString(cmd);

            // add to command List
            receivedCommandList.append(rcvCommand);
            continue;
        }

        // fill command
        cmd[cntCmdByte] = datagram.data()[i];
        cntCmdByte++;
    }
}

void SocketUDP::fillTxCloseCmdPacket(QByteArray* packet)
{
    packet->clear();
    packet->append(QString("K2AC").toAscii());
    const static char data[8] = {23, 21, 0, 0, 1, 0, 2, 0};
    packet->append(data, 8);
}

void SocketUDP::fillBeaconStatePacket(QByteArray* packet, uint state)
{
    packet->clear();
    packet->append(QString("K2AC").toAscii());
    char data[10] = {23, 41, 0, 0, (quint8)(STCPacketSeqNum & 0xFF), (quint8)((STCPacketSeqNum >> 8) & 0xFF), 1, 2, 0xBC, (quint8) (state & 0xFF)};
    packet->append(data, 10);
    STCPacketSeqNum++;
}
