#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <vector>
#include "connthread.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QPushButton;
class QTextEdit;
class QLineEdit;
class QListWidget;
QT_END_NAMESPACE

#define MAXN 1000
class Dialog : public QDialog
{
    Q_OBJECT
public slots:
    void conn();
    void disconn();
    void push_msg_to_conn_th();
    void set_msg(const char*);
    void serv_closed(void);
    void parse_msg(const char* buf);

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();
    connThread conn_th;

protected:
    QLabel *ipLabel;
    QLineEdit * ipEdit;
    QLabel *portLabel;
    QLineEdit * portEdit;
    QLabel *nameLabel;
    QLineEdit * nameEdit;

    QPushButton *connBtn;
    QPushButton *disconnBtn;

    QTextEdit *msgText;
    QLineEdit *msgEdit;
    QPushButton *sendBtn;

    QListWidget *clientList;

    QString messages;
    std::vector<QString> cli_QList;





};


#endif // DIALOG_H
