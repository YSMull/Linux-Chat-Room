#ifndef CONNTHREAD
#define CONNTHREAD

#include <QThread>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

#define BUF_SIZE 1000
class connThread: public QThread
{
    Q_OBJECT
private:
    int sockfd;
    QString ip;
    QString port;
    QString name;
    char buf[BUF_SIZE];
public:
    connThread();


    void setData(const QString &ip, const QString &port, const QString &name);
    void send_msg(const QString& );
    void strcli(int sockfd);
    void run();

signals:
    void msg_ready(const char*);

    void serv_closed(void);

};

#endif // CONNTHREAD

