// DatabaseManager.h
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include "sqlite3.h"
#include "Article.h"

class DatabaseManager
{
public:
    sqlite3 *db;       // 指向 SQLite 数据库的指针
    char *err_message; // 错误信息

    // 构造函数和析构函数
    DatabaseManager(const char *dbname);
    ~DatabaseManager();

    // 成员函数
    void createTable();
    void insertArticle(const Article &article);
    void printArticles();
    void updateArticle(int id, const std::string &title, const std::string &author, int tags, int readCount);
    void deleteArticle(int id);
};

#endif // DATABASE_MANAGER_H
