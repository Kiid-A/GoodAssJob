// DatabaseManager.cpp
#include "DatabaseManager.h"
#include <iostream>

// 构造函数
DatabaseManager::DatabaseManager(const char *dbname)
{
    db = nullptr;
    if (sqlite3_open(dbname, &db) != SQLITE_OK)
    {
        std::cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
    }
}

// 析构函数
DatabaseManager::~DatabaseManager()
{
    if (db != nullptr)
    {
        sqlite3_close(db);
    }
}

// 创建表
void DatabaseManager::createTable()
{
    const char *sql = "CREATE TABLE IF NOT EXISTS Articles ("
                      "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "Title TEXT NOT NULL, "
                      "Author TEXT NOT NULL, "
                      "Tags INTEGER, "
                      "ReadCount INTEGER);";
    if (sqlite3_exec(db, sql, 0, 0, &err_message) != SQLITE_OK)
    {
        std::cerr << "Failed to create table: " << err_message << std::endl;
        sqlite3_free(err_message);
    }
}

// 插入文章
void DatabaseManager::insertArticle(const Article &article)
{
    std::string sql = "INSERT INTO Articles (Title, Author, Tags, ReadCount) VALUES ('" + article.title + "', '" + article.author + "', " + std::to_string(article.tags) + ", " + std::to_string(article.readCount) + ");";
    if (sqlite3_exec(db, sql.c_str(), 0, 0, &err_message) != SQLITE_OK)
    {
        std::cerr << "Error executing SQLite3 statement: " << err_message << std::endl;
        sqlite3_free(err_message);
    }
}

// 实现 printArticles 方法
void DatabaseManager::printArticles()
{
    const char *sql = "SELECT ID, Title, Author, Tags, ReadCount FROM Articles;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare data retrieval statement: " << sqlite3_errmsg(db) << std::endl;
    }
    else
    {
        std::cout << "Articles in database:" << std::endl;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            const char *title = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
            const char *author = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            int tags = sqlite3_column_int(stmt, 3);
            int readCount = sqlite3_column_int(stmt, 4);
            std::cout << "ID: " << id << ", Title: " << title << ", Author: " << author
                      << ", Tags: " << tags << ", ReadCount: " << readCount << std::endl;
        }
    }
    sqlite3_finalize(stmt);
}

void DatabaseManager::updateArticle(int id, const std::string &title, const std::string &author, int tags, int readCount)
{
    std::string sql = "UPDATE Articles SET Title = ?, Author = ?, Tags = ?, ReadCount = ? WHERE ID = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare update statement: " << sqlite3_errmsg(db) << std::endl;
    }
    else
    {
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, author.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, tags);
        sqlite3_bind_int(stmt, 4, readCount);
        sqlite3_bind_int(stmt, 5, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << "Failed to update article: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}

void DatabaseManager::deleteArticle(int id)
{
    std::string sql = "DELETE FROM Articles WHERE ID = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare delete statement: " << sqlite3_errmsg(db) << std::endl;
    }
    else
    {
        sqlite3_bind_int(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::cerr << "Failed to delete article: " << sqlite3_errmsg(db) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
}
