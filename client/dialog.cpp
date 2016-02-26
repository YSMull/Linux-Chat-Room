#include "dialog.h"
#include "connthread.h"
#include "ui_dialog.h"
#include <QtWidgets>

#include <iostream>



using std::endl;
using std::cout;

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent)
{
    //connArea
    ipLabel = new QLabel("&ip:");
    ipEdit  = new QLineEdit;
    ipEdit->setText("0.0.0.0");
    ipLabel->setBuddy(ipEdit);

    portLabel = new QLabel("&port:");
    portEdit  = new QLineEdit;
    portEdit->setText("8888");
    portLabel->setBuddy(portEdit);

    nameLabel = new QLabel("&name:");
    nameEdit  = new QLineEdit;
    nameEdit->setText("mys");
    nameLabel->setBuddy(nameEdit);

    disconnBtn = new QPushButton("&disconnect");
    connBtn = new QPushButton("&connect");
    connBtn->setDefault(true);

    QHBoxLayout *ipLayout = new QHBoxLayout;
    ipLayout->addWidget(ipLabel);
    ipLayout->addStretch(1);
    ipLayout->addWidget(ipEdit);


    QHBoxLayout *portLayout = new QHBoxLayout;
    portLayout->addWidget(portLabel);
    portLayout->addStretch(1);
    portLayout->addWidget(portEdit);

    QHBoxLayout *nameLayout = new QHBoxLayout;
    nameLayout->addWidget(nameLabel);
    nameLayout->addStretch(1);
    nameLayout->addWidget(nameEdit);

    QHBoxLayout *connBtnLayout = new QHBoxLayout;
    connBtnLayout->addWidget(disconnBtn);
    connBtnLayout->addWidget(connBtn);


    QVBoxLayout *connLayout = new QVBoxLayout;
    connLayout->addStretch(1);
    connLayout->addLayout(ipLayout);
    connLayout->addLayout(portLayout);
    connLayout->addLayout(nameLayout);
    connLayout->addStretch(1);

    connLayout->addLayout(connBtnLayout);



    //messageArea

    msgEdit = new QLineEdit;
    sendBtn = new QPushButton("&send");
    QHBoxLayout *sendMsgLayout = new QHBoxLayout;
    sendMsgLayout->addWidget(msgEdit);
    sendMsgLayout->addWidget(sendBtn);

    msgText = new QTextEdit;



    QVBoxLayout *msgLayout = new QVBoxLayout;
    msgLayout->addWidget(msgText);
    msgLayout->addLayout(sendMsgLayout);


    //clientListArea
    clientList = new QListWidget;


    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addLayout(connLayout, 0, 0);
    mainLayout->addLayout(msgLayout, 0, 1);
    mainLayout->addWidget(clientList, 0, 2);
    setLayout(mainLayout);

    connect(connBtn, SIGNAL(clicked()), this, SLOT(conn()));
    connect(disconnBtn, SIGNAL(clicked(bool)), this, SLOT(disconn()));
    connect(sendBtn, SIGNAL(clicked()), this, SLOT(push_msg_to_conn_th()));

    //connect(&conn_th, SIGNAL(msg_ready(const char*)), this, SLOT(set_msg(const char*)));
    connect(&conn_th, SIGNAL(msg_ready(const char*)), this, SLOT(parse_msg(const char*)));
    connect(&conn_th, SIGNAL(serv_closed(void)), this, SLOT(serv_closed(void)));


}
void Dialog::push_msg_to_conn_th() {
    QString msg;
    msg = "{\"msg_type\":\"msg\",\"name\": \"" + nameEdit->text() + "\",\"msg_context\":\" " + msgEdit->text() + "\"}";
    conn_th.send_msg(msg);
}

void Dialog::serv_closed() {
    set_msg("lost connection!");
}

void Dialog::conn() {
    cout << "in conn()" << endl;
    conn_th.setData(ipEdit->text(),portEdit->text(),nameEdit->text());
    if(!conn_th.isRunning())
        conn_th.start();
    else
        cout << "Already connected!" << endl;
    cout << "out conn()" << endl;
}

void Dialog::parse_msg(const char *buf) {
    Document document;
    document.Parse(buf);
    if(document.HasMember("msg_type") &&
       document["msg_type"] == "update_cli_list") {
        cli_QList.clear();
        clientList->clear();
        const Value& cli_list = document["cli_list"];
        for(SizeType i = 0; i < cli_list.Size(); i++) {
            cli_QList.push_back(cli_list[i].GetString());
            clientList->insertItem(i, QString(cli_list[i].GetString()));
        }
    }
    if(document.HasMember("msg_type") &&
       document["msg_type"] == "msg") {
        QString msg;
        msg = msg + document["name"].GetString() + ":" + document["msg_context"].GetString();

        set_msg(msg.toStdString().c_str());
    }
}

void Dialog::set_msg(const char *buf) {

    //parse json
    messages += QString(buf) ;
    messages += "<br/>";
    msgText->setText(messages);
}

void Dialog::disconn() {

}


Dialog::~Dialog()
{

}
