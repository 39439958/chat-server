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
    msgHandlerMap_.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
    msgHandlerMap_.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3)});
    msgHandlerMap_.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, _1, _2, _3)});
    msgHandlerMap_.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, _1, _2, _3)});
    msgHandlerMap_.insert({LOGINOUT_MSG, std::bind(&ChatService::loginOut, this, _1, _2, _3)});
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
            std::vector<std::string> offlineMsgVec = offlineMsgModel_.query(id);
            if (!offlineMsgVec.empty()) {
                response["offlinemsg"] = offlineMsgVec;
                offlineMsgModel_.remove(id);
            }

            // 查询好友列表
            std::vector<User> friendVec = friendModel_.query(id);
            if (!friendVec.empty()) {
                std::vector<string> friendJsonVec;
                for (auto &user : friendVec) {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    friendJsonVec.emplace_back(js.dump());
                }
                response["friends"] = friendJsonVec;
            }

            // 查询群组列表
            vector<Group> groupVec = groupModel_.queryGroups(id);
            if (!groupVec.empty()) {
                std::vector<string> groupJsonVec;
                for (auto &group : groupVec) {
                    json group_js;
                    group_js["id"] = group.getId();
                    group_js["groupname"] = group.getName();
                    group_js["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    group_js["users"] = userV;
                    groupJsonVec.emplace_back(group_js.dump());
                }
                response["groups"] = groupJsonVec;
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
    int to_id = js["toid"].get<int>();

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

void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"].get<int>();
    int friendId = js["friendid"].get<int>();

    friendModel_.insert(userId, friendId);
}

// 创建群组业务
void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int creatorId = js["id"].get<int>();
    string groupName = js["groupname"];
    string groupDesc = js["groupdesc"];

    Group group(-1, groupName, groupDesc);
    if (groupModel_.createGroup(group)) {
        groupModel_.addGroup(creatorId, group.getId(), "creator");
    }
}

// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"].get<int>();
    int groupId = js["groupid"].get<int>();

    groupModel_.addGroup(userId, groupId, "normal");
}

// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"].get<int>();
    int groupId = js["groupid"].get<int>();
    vector<int> recvivers = groupModel_.queryGroupUsers(userId, groupId);

    lock_guard<mutex> lock(connMutex_);
    for (int id : recvivers) {
       auto it = userConnMap_.find(id);
       if (it != userConnMap_.end()) {
        // 用户在线
        it->second->send(js.dump());
       } else {
        // 用户不在线，存储离线消息
        offlineMsgModel_.insert(id, js.dump());
       } 
    }
}

void ChatService::loginOut(const TcpConnectionPtr &conn, json &js, Timestamp time) {
    int userId = js["id"].get<int>();

    {
        lock_guard<mutex> lock(connMutex_);
        auto it = userConnMap_.find(userId);
        if (it != userConnMap_.end()) {
            userConnMap_.erase(it);
        }
    }

    User user(userId, "", "", "offline");
    userModel_.updateState(user);
}