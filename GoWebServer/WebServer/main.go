package main

/*
#include <iostream.h>
#include <stdio.h>
void p() {
	printf("hello, world\n");
}
*/import "C"

import (
	controllers "gowebserver/WebServer/controller"
	"gowebserver/WebServer/db"

	"github.com/gin-contrib/cors"
	"github.com/gin-gonic/gin"
)

// CORSMiddleware 中间件处理跨域问题
func CORSMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {
		c.Writer.Header().Set("Access-Control-Allow-Origin", "*")
		c.Writer.Header().Set("Access-Control-Allow-Credentials", "true")
		c.Writer.Header().Set("Access-Control-Allow-Methods", "POST, GET, OPTIONS, PUT, DELETE")
		c.Writer.Header().Set("Access-Control-Allow-Headers", "Origin, Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization")

		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}

		c.Next()
	}
}

func main() {
	var db db.MyDB
	var ctrl controllers.Controller
	db.InitDB()
	ctrl.InitCtrl(db)

	r := gin.Default()
	r.Use(cors.Default())
	
	// r.Use()
	r.LoadHTMLGlob("data/src/*.html")
	r.Static("assets/", "data/src/assets/")
	// r.Static("user/assets/", "data/src/assets/")
	controllers.LoadIndex(r)
	ctrl.Uc.LoadUser(r)
	ctrl.Ac.LoadArticle(r)
	ctrl.Mc.LoadMap(r)

	r.Run(":9999")
}
