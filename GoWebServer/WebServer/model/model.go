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
}

type Map struct {
	gorm.Model
	Id      string  `gorm:"column:id;primary_key;"`
	Name    string  `gorm:"column:name;"`
	Tags    string  `gorm:"column:tags;"`
	Img     string  `gorm:"column:img;"`
	Url     string  `gorm:"column:url"`
	Rate    float64 `gorm:"column:rate;" json:"rate"`
	Rater   int64   `gorm:"column:rater;" json:"rater"`
	Clicks  int64   `gorm:"column:clicks;" json:"clicks"`
	Hot     float64 `gorm:"column:hot;" json:"hot"`
	Summary string  `gorm:"column:summary"`
	AreaId  string  `gorm:"column:area_id;" json:"area_id"`
}

type MapImg struct {
	gorm.Model
	Id    string `gorm:"column:id;primary_key;" json:"id"`
	Path  string `gorm:"column:path;" json:"path"`
	MapId string `gorm:"column:map_id;" json:"map_id"`
}

type Article struct {
	gorm.Model
	Id       int     `gorm:"column:id;primary_key;" json:"id"`
	MapId    string  `gorm:"column:map_id;" json:"map_id"`
	Title    string  `gorm:"column:title;" json:"title"`
	AuthorId string  `gorm:"column:author_id;" json:"author"`
	Tags     string  `gorm:"column:tags;" json:"tags"`
	Content  string  `gorm:"column:content;" json:"content"`
	Summary  string  `gorm:"column:summary;" json:"summary"`
	Img      string  `gorm:"column:img;" json:"img"`
	Rate     float64 `gorm:"column:rate;" json:"rate"`
	Rater    int64   `gorm:"column:rater;" json:"rater"`
	Clicks   int64   `gorm:"column:clicks;" json:"clicks"`
	Hot      float64 `gorm:"column:hot;" json:"hot"`
	AreaId   string  `gorm:"column:area_id;" json:"area_id"`
}

type ArticleImg struct {
	gorm.Model
	Id        string `gorm:"column:id;primary_key;" json:"id"`
	Path      string `gorm:"column:path;" json:"path"`
	ArticleId string `gorm:"column:article_id;" json:"article_id"`
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

type Area struct {
	Id   int    `gorm:"column:id;"`
	Name string `gorm:"column:name;primary_key;" json:"name"`
}

type NaviReq struct {
	GraphId int   `json:graph_id`
	Start   int   `json:"start"`
	Temp    []int `json:"temp"`
	Target  int   `json:"target"`
}

type ScanReq struct {
	GraphId    int `json:"graph_id"`
	PresentIdx int `json:"p_idx"`
	Dist       int `json:"dist"`
}

type TimeFirst struct {
	Time float64 `json:"time"`
	Dist float64 `json:"dist"`
	Path []int   `json:"path"`
}

type DistFirst struct {
	Time float64 `json:"time"`
	Dist float64 `json:"dist"`
	Path []int   `json:"path"`
}

type Facility struct {
	Toilet    int `json:"toilet"`
	Resturant int `json:"resturant"`
}
