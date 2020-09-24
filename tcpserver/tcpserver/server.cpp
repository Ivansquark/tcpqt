#include "server.h"

Server::Server(QWidget *parent)
    : QDialog(parent)    
{    
    m_ptxt = new QTextEdit(this);
    m_ptxt->setReadOnly(true);
    LinePort = new QLineEdit(this);
    LabIP = new QLabel(this);
    LabPort = new QLabel(this);
    CreateServer = new QPushButton(this);
    butDisconnect = new QPushButton("Kill Server",this);
    LabPort->setText("Enter port");
    CreateServer->setText("Create Server");

    connect(CreateServer,&QPushButton::clicked,this,&Server::onCreateClick);
    connect(butDisconnect,&QPushButton::clicked,this,&Server::onDisconnectClick);
    pvbxLayout =  new QVBoxLayout;
    title = new QLabel("<H1>Server</H1>");
    pvbxLayout->addWidget(title);
    pvbxLayout->addWidget(m_ptxt);
    pvbxLayout->addWidget(LabIP);
    pvbxLayout->addWidget(LabPort);
    pvbxLayout->addWidget(LinePort);
    pvbxLayout->addWidget(CreateServer);pvbxLayout->addWidget(butDisconnect);
    setLayout(pvbxLayout);
}

Server::~Server()
{
    delete tcpServer;
    delete networkSession;
    delete m_ptxt;
    delete LabIP;
    delete LabPort;delete LinePort;delete CreateServer;delete title;delete pvbxLayout;

}

void Server::onCreateClick()
{
    nPort = LinePort->text().toUShort();
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()),this, SLOT(slotNewConnection())); //сигнал отправляется при каждом присоединении клиента
    if (!tcpServer->listen(QHostAddress::Any, nPort)) //if no false запускаем сервер слушаем порт
    {
        QMessageBox::critical(nullptr,
        "Server Error",
        "Unabe to start the server:"
        + tcpServer->errorString());
        tcpServer->close();
        return;
    }
    else{CreateServer->setText("Created on port "+QString::number(nPort));}
    QList<QHostAddress> list = QNetworkInterface::allAddresses();
    QString IPs;
    for(int nIter=0; nIter<list.count(); nIter++)
    {
        if(!list[nIter].isLoopback())
        {
            if (list[nIter].protocol() == QAbstractSocket::IPv4Protocol )
            {qDebug() << list[nIter].toString();IPs+=list[nIter].toString()+" | ";}
        }
    }
    LabIP->setText(IPs);
}

void Server::onDisconnectClick()
{
    tcpServer->close();
    //tcpServer->disconnect();
    //delete tcpServer;
    //tcpServer=nullptr;
    pClientSocket->disconnectFromHost();
}

void Server::slotNewConnection() //вызывается каждый раз при соединении с новым клиентом.
{
    /*returns the QTcpSocket representing the SERVER side of the connection*/
    pClientSocket = tcpServer->nextPendingConnection(); // Для подтверждения соединения с клиентом возвращаем сокет,
                                                        // для связи с номером порта сервера.
    connect(pClientSocket, SIGNAL(disconnected()),pClientSocket, SLOT(deleteLater()));
    connect(pClientSocket, SIGNAL(readyRead()),this,SLOT(slotReadClient()));
    sendToClient(pClientSocket, "Server Response: Connected!"); //передаем в сокет клиента
}

void Server::slotReadClient()
{
    /*!Returns a pointer to the object that sent the signal,
        if called in a slot activated by a signal; otherwise it returns nullptr.*/
    QTcpSocket* pClientSocket = static_cast<QTcpSocket*>(QObject::sender()); //данные пришли
    QDataStream in(pClientSocket);
    in.setVersion(QDataStream::Qt_5_12);
    QByteArray receivedBuf;
    for(uint8_t i=0;i<pClientSocket->bytesAvailable();i++) {
        in>>receivedBuf;
    }
    QString strMessage = receivedBuf;
    m_ptxt->append(strMessage);
    sendToClient(pClientSocket,"1"); //отправляем обратно данные в сокет
}

void Server::sendToClient(QTcpSocket *pSocket, const QString &str) //формируем данные, которые будут отосланы клиенту
{
    QByteArray arrBlock; //выделяем блок байтов
    QDataStream out(&arrBlock, QIODevice::WriteOnly); //записываем все данные блока в arrBlock
    out.setVersion(QDataStream::Qt_5_12);
    out << str;// причем вместо реального размера записываем О.
    pSocket->write(arrBlock); //блок записывается в сокет
}
