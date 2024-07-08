#include "friendmodle.hpp"
#include "db.hpp"

void FriendModel::insert(int userId, int friendId) {
    char sql[1024] = {0};
    sprintf(sql, "insert into friend values('%d', '%d')", userId, friendId);
    
    MySQL mysql;
    if (mysql.connect()) {
        mysql.update(sql);
    }
}

std::vector<User> FriendModel::query(int userId) {
    char sql[1024] = {0};
    sprintf(sql, "select a.id,a.name,a.state from user a inner join friend b on b.friendid = a.id where userid = %d", userId);

    std::vector<User> vec;
    MySQL mysql;
    if (mysql.connect()) {
        MYSQL_RES *res =  mysql.query(sql); // 获取查询结果
        if (res != nullptr) {
            MYSQL_ROW row; // 获取结果中的行信息
            while ((row = mysql_fetch_row(res)) != nullptr) {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.emplace_back(user);
            }        
            mysql_free_result(res);
        }
    }
    return vec;
}