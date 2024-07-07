#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace muduo;

ChatService* ChatService::instance()
{
    static ChatService service;
    return &service;
}


ChatService::ChatService()
{
    msgHandlerMap_.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    msgHandlerMap_.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    msgHandlerMap_.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
}

MsgHandler ChatService::getHandler(int msgid)
{   
    auto it = msgHandlerMap_.find(msgid);
    if (it == msgHandlerMap_.end()) {
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp time) {
            LOG_ERROR << "msgid:" << msgid << " can not find!";
        };
    }
    else
    {
        return msgHandlerMap_[msgid];
    }
}

void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time) 
{
    int id = js["id"];
    string pwd = js["password"];

    User user = userModel_.query(id);
    if (user.getId() == id && user.getPwd() == pwd) {
        if (user.getState() == "online") {
            // 该用户已登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] ="this user is online, please don't login again!";
            conn->send(response.dump());
        } else {
            
            // 缩少mutex的作用域
            {   
                lock_guard<mutex> lock(connMutex_);
                userConnMap_.insert({id, conn});
            }

            // 验证正确，更新用户状态信息
            user.setState("online");
            userModel_.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();

            // 查询是否有离线消息
            std::vector<std::string> vec = offlineMsgModel_.query(id);
            if (!vec.empty()) {
                response["offlinemsg"] = vec;
                offlineMsgModel_.remove(id);
            }
            
            conn->send(response.dump());
        }
    } else {
        // 用户名或密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] ="login fail, id or password are wrong!";
        conn->send(response.dump());
    }

}


void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = userModel_.insert(user);
    if (state) {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn->send(response.dump());
    } else {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "reg fail!";
        conn->send(response.dump());
    }
}

void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int to_id = js["to_id"].get<int>();

    {
        lock_guard<mutex> lock(connMutex_);
        auto it = userConnMap_.find(to_id);
        if (it != userConnMap_.end()) {
            it->second->send(js.dump());
            return;
        }
    }

    // 用户不在线，存储离线消息
    offlineMsgModel_.insert(to_id, js.dump());
}

void ChatService::clientCloseException(const TcpConnectionPtr &conn) {
    User user;
    {
        lock_guard<mutex> lock(connMutex_);
        for (auto it = userConnMap_.begin(); it != userConnMap_.end(); it++) {
            if (it->second == conn) {
                user.setId(it->first);
                userConnMap_.erase(it);
                break;
            }
        }
    }

    if (user.getId() != -1) {
        user.setState("offline");
        userModel_.updateState(user);
    }
}

void ChatService::reset() {
    userModel_.resetState();
}