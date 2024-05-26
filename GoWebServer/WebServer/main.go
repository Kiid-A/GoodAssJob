package main

import (
	controllers "gowebserver/WebServer/controller"
	"gowebserver/WebServer/db"

	"github.com/gin-gonic/gin"
)

func main() {
	db.InitUserDB()
	uc := controllers.GetUserController()

	r := gin.Default()
	r.LoadHTMLGlob("views/*")
	r.Static("assets/", "data/src/assets/")
	r.Static("user/assets/", "data/src/assets/")
	controllers.LoadIndex(r)
	uc.LoadUser(r)

	r.Run(":9999")
}
