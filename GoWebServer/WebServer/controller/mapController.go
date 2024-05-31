package controllers

import (
	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

type MapController struct {
	DB *gorm.DB
}

func LoadMap(e *gin.Engine) {
	// e.GET("/map", loadMap)
}
