package db

import (
	"database/sql"
	models "gowebserver/WebServer/model"

	_ "github.com/go-sql-driver/mysql"
	_ "github.com/ziutek/mymysql/godrv"
	"gorm.io/driver/mysql"
	"gorm.io/gorm"
)

type MyDB struct {
	DB        *gorm.DB
	UserDB    *gorm.DB
	ArticleDB *gorm.DB
	MapDB     *gorm.DB
	RatingDB  *gorm.DB
}

func (db *MyDB) InitDB() {
	dsn := "root:1234@tcp(127.0.0.1:3306)/mydatabase?charset=utf8mb4&parseTime=True&loc=Local"
	sqlDB, err := sql.Open("mysql", dsn)
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}
	sqlDB.SetMaxIdleConns(1000)
	sqlDB.SetMaxOpenConns(100000)
	sqlDB.SetConnMaxLifetime(-1)
	DB, err := gorm.Open(mysql.New(mysql.Config{Conn: sqlDB}), &gorm.Config{})
	if err != nil {
		panic("failed to connect database, err:" + err.Error())
	}
	db.DB = DB
	db.DB.AutoMigrate(&models.Article{}, &models.User{}, &models.Map{}, &models.Rating{}, &models.ArticleImg{}, &models.MapImg{}, &models.Area{})

	db.UserDB = db.DB
	db.ArticleDB = db.DB
	db.MapDB = db.DB
	db.RatingDB = db.DB
}
