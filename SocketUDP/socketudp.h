#ifndef SOCKETUDP_H
#define SOCKETUDP_H

#include <QObject>
#include <QUdpSocket>
#include <QList>
#include <fileProc/iniFileProc.h>

typedef struct
{
    QString Key;
    QString Value;
} command;

class SocketUDP : public QObject
{
    Q_OBJECT
public:
    explicit SocketUDP(QObject *parent = 0);
    ~SocketUDP()
    {
        if((udpSocket != 0) && (udpSocket->isOpen()))
            udpSocket->abort();
    }
    
    void SaveLogFileParams();

    bool open();
    void close();

signals:
    void commandReceived(QList<command> *cmdList);
    void QuitMainWindow();
    void TextOut(QStringList text);
    bool ReloadFromIniCmdReceived();
    
public slots:
    void readUdp();
    void sendBeaconStateToSTC(uint state);

    void OnReloadLogParam(logFileParamStruct param);

protected:
    void readDatagramRoutine(QByteArray datagram);
    void parsePacketData(QByteArray datagram);
    void fillTxCloseCmdPacket(QByteArray *);
    void fillBeaconStatePacket(QByteArray* packet, uint state);

    QUdpSocket* udpSocket;
    netParamStruct netParam;
    logFileParamStruct logParam;
};

#endif // SOCKETUDP_H
