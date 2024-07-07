#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include <mutex>
using namespace std;
using namespace muduo;
using namespace muduo::net;

#include "json.hpp"
using json = nlohmann::json;

#include "public.hpp"
#include "usermodel.hpp"
#include "offlinemsgmodel.hpp"

using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp time)>;

class ChatService
{
 public:
    // 获取单例
    static ChatService* instance();
    // 获取handler
    MsgHandler getHandler(int msgid);

    // 登录
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 注册
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);
    // 一对一聊天
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理服务器异常退出
    void reset();

    // 处理用户异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
 private:
    ChatService();

    // 存储msgid与对应的处理方法
    unordered_map<int, MsgHandler> msgHandlerMap_;

    // 存储在线的用户
    unordered_map<int, TcpConnectionPtr> userConnMap_;
    mutex connMutex_;

    // 处理User业务
    UserModel userModel_;

    // 处理离线信息业务
    OfflineMsgModel offlineMsgModel_;
};

#endif