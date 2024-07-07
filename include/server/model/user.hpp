#ifndef USER_H
#define USER_H

#include <string>

class User
{
public:
    User(int id = -1, std::string name = "", std::string pwd = "", std::string state = "offline")
    {
        this->id_ = id;
        this->name_ = name;
        this->password_ = pwd;
        this->state_ = state;
    }

    void setId(int id) { this->id_ = id; }
    void setName(std::string name) { this->name_ = name; }
    void setPwd(std::string pwd) { this->password_ = pwd; }
    void setState(std::string state) { this->state_ = state; }

    int getId() { return id_; }
    std::string getName() { return name_; }
    std::string getPwd() { return password_; }
    std::string getState() { return state_; }

private:
    int id_;
    std::string name_;
    std::string password_;
    std::string state_;
};

#endif