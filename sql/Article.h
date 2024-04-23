// Article.h
#ifndef ARTICLE_H
#define ARTICLE_H

#include <string>

class Article
{
public:
    int id;             // 文章的唯一标识符
    std::string title;  // 文章标题
    std::string author; // 作者名称
    int tags;           // 标签（整数表示）
    int readCount;      // 阅读量

    // 构造函数
    Article(std::string title, std::string author, int tags, int readCount);
};

#endif // ARTICLE_H
