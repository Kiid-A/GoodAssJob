package db

import (
	"database/sql"
	models "gowebserver/WebServer/model"

	_ "github.com/go-sql-driver/mysql"
	_ "github.com/ziutek/mymysql/godrv"
	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

func InitUserDB() *gorm.DB {

	dsn := "root:1234@tcp(127.0.0.1:3306)/mydatabase?charset=utf8mb4&parseTime=True&loc=Local"
	sqlDB, err := sql.Open("mysql", dsn)
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}
	UserDB, err := gorm.Open(mysql.New(mysql.Config{Conn: sqlDB}), &gorm.Config{})
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}

	UserDB.AutoMigrate(&models.User{})

	return UserDB.Table("users")
}

func InitMapDB() *gorm.DB {

	dsn := "root:1234@tcp(127.0.0.1:3306)/mydatabase?charset=utf8mb4&parseTime=True&loc=Local"
	sqlDB, err := sql.Open("mysql", dsn)
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}
	MapDB, err := gorm.Open(mysql.New(mysql.Config{Conn: sqlDB}), &gorm.Config{})
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}

	MapDB.AutoMigrate(&models.Map{})

	return MapDB.Table("maps")
}

func InitArticleDB() *gorm.DB {

	dsn := "root:1234@tcp(127.0.0.1:3306)/mydatabase?charset=utf8mb4&parseTime=True&loc=Local"
	sqlDB, err := sql.Open("mysql", dsn)
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}
	ArticleDB, err := gorm.Open(mysql.New(mysql.Config{Conn: sqlDB}), &gorm.Config{})
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}

	ArticleDB.AutoMigrate(&models.Article{})

	return ArticleDB.Table("articles")
}
