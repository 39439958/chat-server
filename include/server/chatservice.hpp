#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

#include "public.hpp"

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js)>;

class ChatService
{
 public:
    // 获取单例
    static ChatService* instance();
    // 登录
    void login(const TcpConnectionPtr &conn, json &js);
    // 注册
    void reg(const TcpConnectionPtr &conn, json &js);

    MsgHandler getHandler(int msgid);
 private:
    ChatService();

    // 存储msgid与对应的处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

};

#endif