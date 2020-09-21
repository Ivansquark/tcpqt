/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

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
    //Layout setup
    QVBoxLayout* pvbxLayout = new QVBoxLayout;
    pvbxLayout->addWidget(new QLabel("<H1>Client</H1>"));
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
    in.setVersion(QDataStream::Qt_5_13);
    for (;;) {
        if ( !nextBlockSize) //nextBlockSize=0
        {
            if (tcpSocket->bytesAvailable() < sizeof(quint16)) // если меньше двух байтов
            {
                break; //выходим из бесконечного цикла
            }
            in >> nextBlockSize;
        }
        if (tcpSocket->bytesAvailable() < nextBlockSize)
        {
            break;
        }
        QTime time;
        QString str;
        in >> time >> str;
        txtInfo->append(time.toString() +" "+ str);
        nextBlockSize = 0;
    }
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
    QByteArray arrBlock;
    QDataStream out(&arrBlock, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_13);
    out << quint16(0) << QTime::currentTime() << txtInput->text();
    out.device()->seek(0);
    out << quint16(arrBlock.size() - sizeof(quint16));
    tcpSocket->write(arrBlock);
    txtInput->setText("");
}

void Client::slotConnected()
{
    txtInfo->append("Received the connected() signal");
}


