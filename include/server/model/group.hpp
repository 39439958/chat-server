#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include "groupuser.hpp"
using namespace std;

class Group
{
public:
    Group(int id = -1, string name = "", string desc = " ")
    {
        this->id_ = id;
        this->name_ = name;
        this->desc_ = desc;
    }

    void setId(int id) { this->id_ = id; }
    void setName(string name) { this->name_ = name; }
    void setDesc(string desc) { this->desc_ = desc; }

    int getId() { return id_; }
    string getName() { return name_; }
    string getDesc() { return desc_; }
    vector<GroupUser> &getUsers() { return users_; }

private:
    int id_;
    string name_;
    string desc_;
    vector<GroupUser> users_;
};

#endif