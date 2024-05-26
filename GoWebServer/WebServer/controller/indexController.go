package controllers

import (
	"github.com/gin-gonic/gin"
)

func LoadIndex(e *gin.Engine) {
	e.GET("/", welcome)
}

func welcome(c *gin.Context) {
	c.HTML(200, "index.html", gin.H{})
}
