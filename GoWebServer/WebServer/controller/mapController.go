package controllers

import (
	"gowebserver/WebServer/db"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

type MapController struct {
	DB *gorm.DB
}

func GetMapController() *MapController {
	return &MapController{
		DB: db.InitMapDB(),
	}
}

func LoadMap(e *gin.Engine) {
	// e.GET("/map", loadMap)
}
