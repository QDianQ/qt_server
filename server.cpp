#include "server.h"

#include <QStorageInfo>

Server::Server()
{
    timerCheckConnection = new QTimer(this);
    timerCheckConnection->setInterval(1000);
    connect(timerCheckConnection, SIGNAL(timeout()), this, SLOT(slotTimerCheckConnection()));

    if(this->listen(QHostAddress("127.0.0.1"), 2323))
    {
        qDebug() << "Server is starting";

    }
    else
    {
        qDebug() << "Start server is failed";
    }
    nextBlockSize = 0;
}
Server::~Server()
{

}
void Server::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket;
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, &QTcpSocket::readyRead, this, &Server::slotReadyRead);
    connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);

    Sockets.push_back(socket);
    qDebug() << "Client connected";
    timerCheckConnection->start();
}

void Server::slotReadyRead()
{
    socket = (QTcpSocket*)sender();
    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_6_4); // version
    if(in.status() == QDataStream::Ok)
    {
        for(;;)
        {
            if(nextBlockSize == 0)
            {
                if(socket->bytesAvailable() < 2)
                {
                    break;
                }
                in >> nextBlockSize;
            }
            if(socket->bytesAvailable() < nextBlockSize)
            {
                qDebug() << "Data not full, break = 0";
                break;
            }
            QString str;
            in >> str;
            nextBlockSize = 0;

            if (str == "_getDiskInfo")
            {
                sendDiskInfo(getDiskInfo());
                qDebug() << QTime::currentTime().toString() << " Disks information was sent to client";

            }
            else
                qDebug() << "error " <<str;
//            sendToClient(str);
            break;
        }
    }
    else{
        qDebug() << "DataStream error";
    }
}

void Server::slotTimerCheckConnection()
{
    if (QAbstractSocket::UnconnectedState == socket->state())
    {
        qDebug() << "Client was disconnected";
        timerCheckConnection->stop();
    }
}

void Server::sendToClient(QString str)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out << quint16(0) << str;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
//    socket->write(Data);
    for (int i = 0; i < Sockets.size(); i++)
    {
        Sockets[i]->write(Data);
    }
}

void Server::sendDiskInfo(QMap<int, QList<QString> > diskInfo)
{
    Data.clear();
    QDataStream out(&Data, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_6_4);
    out << quint16(0) << diskInfo;
    out.device()->seek(0);
    out << quint16(Data.size() - sizeof(quint16));
//    socket->write(Data);
    for (int i = 0; i < Sockets.size(); i++)
    {
        Sockets[i]->write(Data);
    }
}

QMap<int, QList<QString>> Server::getDiskInfo()
{
    QMap<int, QList<QString>> diskInfo;
    QList<QString> disk;

    int count = 0;
    foreach (const QStorageInfo &storage, QStorageInfo::mountedVolumes()) {
            if (storage.isValid() && storage.isReady()) {
                if (!storage.isReadOnly()) {
                    disk.append(storage.rootPath());
                    disk.append(storage.displayName());
                    disk.append(QString::number(storage.bytesTotal()/1000/1000) + " MB");
                    disk.append(QString::number(storage.bytesTotal()/1000/1000 - storage.bytesFree()/1000/1000) + " MB");
                    disk.append(QString::number(storage.bytesFree()/1000/1000) + " MB");

                    diskInfo.insert(count, disk);
                    count++;
                    disk.clear();
                }
            }
    }
    return diskInfo;
}
