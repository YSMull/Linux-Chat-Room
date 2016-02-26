#include "connthread.h"
#include <QtWidgets>
#include <iostream>
using std::cout;
using std::endl;

connThread::connThread()
{

}

void connThread::send_msg(const QString& msg) {
    cout << "send_msg()" << endl;
    memset(buf, 0, sizeof(buf));
    //QString msg = msgEdit->text();
    int n = msg.length();
    if(msg.length() != 0) {
        strcpy(buf, msg.toStdString().c_str());
        write(sockfd, buf, n);
    }
}

void connThread::setData(const QString &ip, const QString &port, const QString &name)
{
    this->ip = ip;
    this->port = port;
    this->name = name;
}

void connThread::strcli(int sockfd) {
    const int MAXLINE = 1000;
    int     maxfdp1;
    fd_set  rset;
    char    buf[MAXLINE];
    int     n;

    FD_ZERO(&rset);
    for( ; ; ) {
        FD_SET(sockfd, &rset);
        maxfdp1 = sockfd + 1;
        select(maxfdp1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &rset)) {
            cout << "msg_ready()" << endl;
            memset(buf, 0, sizeof(buf));
            perror("err:");
            if( (n = read(sockfd, buf, MAXLINE)) == 0) {
                cout << "no readable" << endl;
                emit serv_closed();
                return ;
            }
            emit msg_ready(buf);
            cout << "get text from server: " <<buf << endl;
        }
    }
}

void connThread::run()
{
    cout << "Thread is running..." << endl;
    int result;
    fd_set rset;
    FD_ZERO(&rset);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port.toShort());
    servaddr.sin_addr.s_addr = inet_addr(ip.toStdString().c_str());

    result = ::connect(sockfd, (sockaddr *)&servaddr, sizeof(servaddr));
    if(result == -1) {
        cout << "conn err" << endl;
        return;
    }
    QString new_client_msg = "{\"msg_type\":\"new_client\", \"name\":\""+ name +"\"}";
    sockaddr_in cliaddr;
    socklen_t cliaddr_len;
    if (getsockname(sockfd, (sockaddr *)&cliaddr, &cliaddr_len) < 0) {
        cout << "getsockname() err!" << endl;
    }
    write(sockfd, new_client_msg.toStdString().c_str(), new_client_msg.length());
    strcli(sockfd);//

    cout << "Thread is end..." << endl;
}
