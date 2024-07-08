#include "groupmodel.hpp"
#include "db.hpp"

bool GroupModel::createGroup(Group &group) {
    char sql[1024] = {0};
    sprintf(sql, "insert into allgroup(groupname, groupdesc) values('%s', '%s')",
        group.getName().c_str(), group.getDesc().c_str());
    
    MySQL mysql;
    if (mysql.connect()) {
        if (mysql.update(sql)) {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

void GroupModel::addGroup(int userId, int groupId, string role) {
    char sql[1024] = {0};
    sprintf(sql, "insert into groupuser(groupid, userid, grouprole) values('%d', '%d', '%s')",
        userId, groupId, role.c_str());

    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

vector<Group> GroupModel::queryGroups(int userId) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.groupname,a.groupdesc from allgroup a inner join groupuser b on b.groupid = a.id where userid = %d", userId);

    std::vector<Group> groupVec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res =  mysql.query(sql); // 获取查询结果
        if (res != nullptr) {
            MYSQL_ROW row; // 获取结果中的行信息
            while ((row = mysql_fetch_row(res)) != nullptr) {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.emplace_back(group);
            }        
            mysql_free_result(res);
        }
    }

    for (auto &group : groupVec) {
        sprintf(sql, "select a.id,a.name,a.state,b.grouprole from user a \
            inner join groupuser b on b.userid = a.id where b.groupid=%d",
                group.getId());

        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUsers().push_back(user);
            }
            mysql_free_result(res);
        }
    }

    return groupVec;
}

vector<int> GroupModel::queryGroupUsers(int userId, int groupId) {
    char sql[1024] = {0};
    sprintf(sql, "select userid from groupuser where groupid = %d and userid != %d", groupId, userId);

    vector<int> userVec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res =  mysql.query(sql); // 获取查询结果
        if (res != nullptr) {
            MYSQL_ROW row; // 获取结果中的行信息
            while ((row = mysql_fetch_row(res)) != nullptr) {
                userVec.emplace_back(atoi(row[0]));
            }        
            mysql_free_result(res);
        }
    }
    return userVec;
}