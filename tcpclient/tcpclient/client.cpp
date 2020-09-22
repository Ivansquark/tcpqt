#include <QtWidgets>
#include <QtNetwork>

#include "client.h"

Client::Client(QWidget *parent):
    QDialog(parent), nextBlockSize(0)
{
    lineIP = new QLineEdit(this);
    linePort = new QLineEdit(this);
    labIP = new QLabel(this);
    labIP->setText("IP:");
    labPort = new QLabel(this);
    labPort->setText("Port:");
    txtInfo = new QTextEdit;
    txtInput = new QLineEdit;
    QPushButton* pcmd = new QPushButton("&Send");
    QPushButton* conn = new QPushButton("&Connect");
    QPushButton* disconnect = new QPushButton("&Disconnect",this);
    QPushButton* close = new QPushButton("Close",this);
    connect(pcmd, SIGNAL(clicked() ), SLOT(slotSendToServer()));
    connect(conn,&QPushButton::clicked,this,&Client::onConnectClick);
    connect(disconnect,&QPushButton::clicked,this,&Client::onDisconnectClick);
    connect(close,&QPushButton::clicked,this,[this](){this->close();});
    connect(txtInput, SIGNAL(returnPressed()),this, SLOT(slotSendToServer()));    
	
	lineReceive = new QLineEdit(this);
    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));
    pvbxLayout->addWidget(lineReceive);
    pvbxLayout->addWidget(txtInfo);
    pvbxLayout->addWidget(txtInput);
    pvbxLayout->addWidget(pcmd);
    pvbxLayout->addWidget(conn);
    pvbxLayout->addWidget(disconnect);
    pvbxLayout->addWidget(close);
    pvbxLayout->addWidget(labIP);
    pvbxLayout->addWidget(lineIP);
    pvbxLayout->addWidget(labPort);
    pvbxLayout->addWidget(linePort);

    setLayout(pvbxLayout);
}

void Client::onConnectClick()
{
    tcpSocket = new QTcpSocket(this);
    //"172.22.1.89",50000
    QString strHost = lineIP->text();
    uint16_t nPort = linePort->text().toUShort();
    tcpSocket->connectToHost(strHost, nPort);
    connect(tcpSocket, SIGNAL(connected()), SLOT(slotConnected()));
    connect(tcpSocket, SIGNAL(readyRead() ), SLOT(slotReadyRead())); //вызывается при поступлении данных от сервера.
    connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(slotError(QAbstractSocket::SocketError)));
    connect(tcpSocket, &QTcpSocket::disconnected,[this](){this->txtInfo->append("\ndisconnected\n");});

    txtInfo->setReadOnly(true);
}

void Client::onDisconnectClick()
{    
    tcpSocket->disconnectFromHost();
}

void Client::slotReadyRead()
{
    QDataStream in(tcpSocket);
    in.setVersion(QDataStream::Qt_5_12);
    uint16_t count = tcpSocket->bytesAvailable();
    QByteArray bufReceive;
    for(uint8_t i=0;i<count;i++){
        uint8_t byte; in>>byte;bufReceive.append(byte);
    }
    qDebug()<<bufReceive;
    uint32_t val = (bufReceive[0]<<24)|(bufReceive[1]<<16)|(bufReceive[2]<<8)|(bufReceive[3]);
    lineReceive->setText(QString::number(val));
    nextBlockSize = 0;
}

void Client::slotError(QAbstractSocket::SocketError err)
{
    QString strError = "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                                        "The host was not found." :
                                        err == QAbstractSocket::RemoteHostClosedError ?
                                            "The remote host is closed." :
                                            err == QAbstractSocket::ConnectionRefusedError?
                                                "The connection was refused." :
                                                QString(tcpSocket->errorString()));
    txtInfo->append(strError);
}

void Client::slotSendToServer()
{
    QByteArray arrBlock;arrBlock.clear();
    //QDataStream out(&arrBlock, QIODevice::WriteOnly);
    //out.setVersion(QDataStream::Qt_5_12);
    //out << txtInput->text(); //sets in first byte data length //<< QTime::currentTime()
    //out.device()->seek(0);
    //out << quint16(arrBlock.size() - sizeof(quint16));
    QString str = txtInput->text();
    for(const auto character: str){
        arrBlock.append(character.toLatin1());
    }
    tcpSocket->write(arrBlock);
    txtInput->setText("");
}

void Client::slotConnected()
{
    txtInfo->append("Received the connected() signal");
}


