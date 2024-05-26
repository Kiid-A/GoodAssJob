package models

import (
	_ "github.com/go-sql-driver/mysql"
	_ "github.com/ziutek/mymysql/godrv"
	"gorm.io/gorm"
)

type User struct {
	gorm.Model
	// Id       string
	// Name     string
	Username string `gorm:"column:username;primary_key" json:"username"`
	Passwd   string `gorm:"column:passwd;" json:"passwd"`
	// Gender   string
	// Token    string
}

type Map struct {
	Id   string
	Name string
	Type []string
}

type Article struct {
	gorm.Model
	ID      int      `json:"id"`
	Title   string   `json:"title"`
	Type    []string `json:"type"`
	Content string   `json:"content"`
}

type Comment struct {
	ID        int    `json:"id"`
	ArticleID int    `json:"article_id"`
	Content   string `json:"content"`
	UserID    int    `json:"user_id"`
}
