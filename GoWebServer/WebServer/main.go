package main

import (
	controllers "gowebserver/WebServer/controller"
	"gowebserver/WebServer/db"

	"github.com/gin-gonic/gin"
)

func main() {
	var db db.MyDB
	var ctrl controllers.Controller
	db.InitDB()
	ctrl.InitCtrl(db)

	r := gin.Default()
	r.LoadHTMLGlob("views/*")
	r.Static("assets/", "data/src/assets/")
	// r.Static("user/assets/", "data/src/assets/")
	controllers.LoadIndex(r)
	ctrl.Uc.LoadUser(r)
	ctrl.Ac.LoadArticle(r)

	r.Run(":9999")
}
