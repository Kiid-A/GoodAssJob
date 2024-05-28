package models

import (
	_ "github.com/go-sql-driver/mysql"
	_ "github.com/ziutek/mymysql/godrv"
	"gorm.io/gorm"
)

type User struct {
	gorm.Model
	Id       string `gorm:"column:id;"`
	Name     string `gorm:"column:name;"`
	Username string `gorm:"column:username;primary_key;" json:"username"`
	Passwd   string `gorm:"column:passwd;" json:"passwd"`

	// Gender   string
	// Token    string
}

type Map struct {
	gorm.Model
	Id   string   `gorm:"column:id;primary_key;"`
	Name string   `gorm:"column:name;"`
	Type []string `gorm:"column:type;"`
}

type Article struct {
	gorm.Model
	Id      int64  `gorm:"column:id;primary_key;" json:"id"`
	Title   string `gorm:"column:title;" json:"title"`
	Type    string `gorm:"column:type;" json:"type"`
	Content string `gorm:"column:content;" json:"content"`
}

type Comment struct {
	Id        int    `gorm:"column:id;primary_key;" json:"id"`
	ArticleID int    `gorm:"column:article_id;" json:"article_id"`
	Content   string `gorm:"column:type;" json:"content"`
	UserID    int    `gorm:"column:user_id;" json:"user_id"`
}
