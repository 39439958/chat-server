#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
using namespace muduo;

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}


ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2)});
}

MsgHandler ChatService::getHandler(int msgid)
{   
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end()) {
        return [=](const TcpConnectionPtr &conn, json &js) {
            LOG_ERROR << "msgid:" << msgid << " can not find!";
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js) 
{
    LOG_INFO << "do login service";
}


void ChatService::reg(const TcpConnectionPtr &conn, json &js)
{
    LOG_INFO << "do reg service";
}
