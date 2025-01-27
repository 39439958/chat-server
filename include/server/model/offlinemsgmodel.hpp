#ifndef OFFLINEMSGMODEL_H
#define OFFLINEMSGMODEL_H

#include <string>
#include <vector>

class OfflineMsgModel {
 public:
    void insert(int userId, std::string msg);

    void remove(int userId);

    std::vector<std::string> query(int userId);

};


#endif