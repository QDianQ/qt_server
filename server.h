#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QVector>


class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server();
    ~Server();
    QTcpSocket *socket;


private:
    QVector <QTcpSocket*> Sockets;
    QByteArray Data;
    quint16 nextBlockSize;
    QTimer *timerCheckConnection;


    void sendToClient(QString str);
    void sendDiskInfo(QMap<int, QList<QString>> diskInfo);

    QMap<int, QList<QString>> getDiskInfo();


public slots:
    void incomingConnection(qintptr socketDescriptor);
    void slotReadyRead();
    void slotTimerCheckConnection();
};


#endif // SERVER_H
