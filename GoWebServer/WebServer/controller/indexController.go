package controllers

import (
	"gowebserver/WebServer/db"

	"github.com/gin-gonic/gin"
)

type Controller struct {
	Ac ArticleController
	Uc UserController
	Mc MapController
}

func (c *Controller) InitCtrl(db db.MyDB) {
	c.Ac = ArticleController{
		DB: db.ArticleDB,
	}
	c.Uc = UserController{
		DB: db.UserDB,
	}
	c.Mc = MapController{
		DB: db.MapDB,
	}
}

func LoadIndex(e *gin.Engine) {
	e.GET("/", welcome)
}

func welcome(c *gin.Context) {
	c.HTML(200, "index.html", gin.H{})
}
