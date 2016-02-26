#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <netinet/in.h>
#include <memory.h>

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
using std::string;
using std::vector;
using std::cout;
using std::endl;

const int MAX_CLI = 10000;
const int SERV_PORT = 8888;
const int LISTENQ = 500;
const int BUF_SIZE = 200;
const int MAXLINE = 100;
int test_n = 0;
#define OUTPUTINFO


#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

sql::Driver *driver;
sql::Connection *con;
sql::Statement *stmt;


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;

void conn_mysql()
{
  try {
      driver = get_driver_instance();
      con = driver->connect("tcp://127.0.0.1:3306", "root", "123");
      con->setSchema("ssg");
      stmt = con->createStatement();
  }
  catch (sql::SQLException &e) {
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
  }
}
void close_mysql()
{
  try {
    delete stmt;
    delete con;
  }
  catch (sql::SQLException &e) {
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
  }

}
void store_msg(const string &name, const string &msg)
{
  try {
    string exstr;
    exstr = "INSERT INTO msg VALUES ('" + name + "','" + msg + "')";
    stmt->execute(exstr);
  }
  catch (sql::SQLException &e) {
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
  }
}

struct Client
{
  string ip;
  string name;
  int connfd;
};
class cli_finder
{
public:
  cli_finder(const int &cmp_connfd):connfd(cmp_connfd){}
  bool operator()(const Client &cli)
  {
    return this->connfd == cli.connfd;
  }
private:
  int connfd;
};
vector<Client> cli_list;


int listen_fd, conn_fd[MAX_CLI];
int n;
char buf[BUF_SIZE];
int max_i;

void push_cli_list_to_clients()
{
  StringBuffer s;
  Writer<StringBuffer> writer(s);

  writer.StartObject();
  writer.Key("msg_type");
  writer.String("update_cli_list");
  writer.Key("cli_list");
  writer.StartArray();
  for(auto cli:cli_list) {
    writer.String(cli.name.c_str());
  }
  writer.EndArray();
  writer.EndObject();
  cout << s.GetString() << endl;
  printf("sending cli_list to all clients...\n");
  for(int j = 0; j <= max_i; j++)
  {
      if(conn_fd[j] > 0)
      {
        write(conn_fd[j], s.GetString(), strlen(s.GetString()));
      }
  }
  sleep(1);
}
int main()
{
    conn_mysql();


    sockaddr_in serv_addr, cli_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    bind(listen_fd, (sockaddr *)&serv_addr, sizeof(serv_addr) );
    listen(listen_fd, LISTENQ);

    int maxfd = listen_fd;
    max_i = -1;
    for(int i = 0; i < MAX_CLI; i++)
    {
        conn_fd[i] = -1;
    }
    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listen_fd, &allset);
    try
    {
        for( ; ; )
        {
            rset = allset;
            Client cur_cli;
            int nready = select(maxfd+1, &rset, NULL, NULL, NULL);
            if(FD_ISSET(listen_fd, &rset))
            {
#ifdef OUTPUTINFO
                printf("%d: new client!\n", ++test_n);
#endif
                socklen_t cli_len = sizeof(cli_addr);
                int connfd = accept(listen_fd, (sockaddr *)&cli_addr, &cli_len);

                cur_cli.connfd = connfd;
                int i;
                for(i = 0; i < MAX_CLI; i++)
                {
                    if(conn_fd[i] < 0)
                    {
                        conn_fd[i] = connfd;
                        break;
                    }
                }
                if(i == MAX_CLI)
                {
                    printf("touch max client numbers!\n");
                    continue;//TBT
                }

                FD_SET(connfd, &allset);
                if(maxfd < connfd)
                    maxfd = connfd;
                if(max_i < i)
                    max_i = i;
                if(--nready <= 0)
                    continue;
            }

            for(int i = 0; i <= max_i; i++)  //must <=
            {
                if(conn_fd[i] < 0)
                {
                    continue;
                }

                if(FD_ISSET(conn_fd[i], &rset))  //maybe bug
                {

                    //printf("select connfd!\n");
                    memset(buf, 0, sizeof(buf));
                    if( (n = read(conn_fd[i], buf, MAXLINE)) == 0)
                    {
#ifdef OUTPUTINFO
                        printf("client exits!\n");
#endif
                        close(conn_fd[i]);
                        FD_CLR(conn_fd[i], &allset);

                        auto it = find_if(cli_list.begin(),cli_list.end(),cli_finder(conn_fd[i]));
                        if(it == cli_list.end())
                          cout<<"can not find deleted client..." << endl;
                        else {
                          cli_list.erase(it);
                          push_cli_list_to_clients();
                        }

                        conn_fd[i] = -1;
                    }
                    else
                    {

                        Document document;
                        try
                        {
                          cout << buf << endl;
                          document.Parse(buf);
                        }
                        catch(...)
                        {
                          cout << "error parse" << endl;
                          exit(0);
                        }


                        if(document.HasMember("msg_type") &&
                           document["msg_type"] == "new_client") {
                          cout << "newclient:" << document["name"].GetString() << endl;
                          cur_cli.name = document["name"].GetString();
                          cli_list.push_back(cur_cli);

                          push_cli_list_to_clients();

                        }

                        if(document.HasMember("msg_type") &&
                           document["msg_type"] == "msg") {
                             string t_name = document["name"].GetString();
                             string t_msg_text = document["msg_context"].GetString();
                             cout << "name:" << t_name
                                  << ", msg_context: " << t_msg_text << endl;
                             store_msg(t_name, t_msg_text);

                        }
                        for(int j = 0; j <= max_i; j++)
                        {
                            if(conn_fd[j] > 0)
                            {
                              //printf("send_msg: %s...\n", buf);
                              write(conn_fd[j], buf, n);

                            }
                        }

                        memset(buf, 0, BUF_SIZE * sizeof(char));
                    }
                    if(--nready <= 0) break;
                }
                //perror("err:");
            }
        }
    }
    catch (...)
    {

        perror("catch err:\n");
    }


    close_mysql();
    printf("exit for\n");
    perror("server halt:");
}
