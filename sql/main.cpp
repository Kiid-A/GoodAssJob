#include <iostream>
#include "sqlite3.h"
#include "Article.h"
#include "DatabaseManager.h"

int main()
{
    DatabaseManager dbManager("test.db");
    dbManager.createTable();

    // 插入示例文章
    dbManager.insertArticle(Article("Understanding AI", "John Doe", 101, 150));
    dbManager.insertArticle(Article("Data Science with Python", "Jane Smith", 102, 250));

    // 更新文章
    dbManager.updateArticle(1, "Advanced AI", "John Doe Updated", 201, 300);

    // 删除文章
    dbManager.deleteArticle(2);

    // 输出更新后的数据库中的文章`
    dbManager.printArticles();

    return 0;
}
