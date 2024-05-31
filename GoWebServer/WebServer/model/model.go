package models

import (
	_ "github.com/go-sql-driver/mysql"
	_ "github.com/ziutek/mymysql/godrv"
	"gorm.io/gorm"
)

type User struct {
	gorm.Model
	Id       string `gorm:"column:id;" json:"id"`
	Name     string `gorm:"column:name;" json:"name"`
	Username string `gorm:"column:username;primary_key;" json:"username"`
	Passwd   string `gorm:"column:passwd;" json:"passwd"`

	LikeTags string `gorm:"column:liketags;" json:"liketags`
	// Gender   string
	// Token    string
}

type Map struct {
	gorm.Model
	Id   string `gorm:"column:id;primary_key;"`
	Name string `gorm:"column:name;"`
	Tags string `gorm:"column:tags;"`
}

type Article struct {
	gorm.Model
	Id       string `gorm:"column:id;primary_key;" json:"id"`
	MapId    string `gorm:"column:map_id;" json:"map_id"`
	Title    string `gorm:"column:title;" json:"title"`
	AuthorId string `gorm:"column:author_id;" json:"author"`
	Tags     string `gorm:"column:tags;" json:"tags"`
	Content  string `gorm:"column:content;" json:"content"`

	Rate   float64 `gorm:"column:rate;" json:"rate"`
	Rater  int64   `gorm:"column:rater;" json:"rater"`
	Clicks int64   `gorm:"column:clicks;" json:"clicks"`
	Hot    float64 `gorm:"column:hot;" json:"hot"`
}

type Rating struct {
	UserId      string  `gorm:"column:user_id;primary_key;" json:"user_id"`
	Art         float64 `gorm:"column:art;" json:"art"`
	Culture     float64 `gorm:"column:culture;" json:"culture"`
	Education   float64 `gorm:"column:education;" json:"education"`
	Exploration float64 `gorm:"column:exploration;" json:"exploratino"`
	History     float64 `gorm:"column:history;" json:"history"`
	Nature      float64 `gorm:"column:nature;" json:"nature"`
	Science     float64 `gorm:"column:science;" json:"science"`
}

// let alone the mtfk commment
type Comment struct {
	Id        int    `gorm:"column:id;primary_key;" json:"id"`
	ArticleId int    `gorm:"column:article_id;" json:"article_id"`
	Content   string `gorm:"column:type;" json:"content"`
	UserId    int    `gorm:"column:user_id;" json:"user_id"`
}
