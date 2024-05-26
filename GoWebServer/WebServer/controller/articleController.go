package controllers

import (
	models "gowebserver/WebServer/model"
	"net/http"

	"github.com/gin-gonic/gin"
)

func LoadArticle(e *gin.Engine) {
	e.GET("/article", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"article": models.Article{}.ID,
		})
	})

	e.GET("/article/:id", func(c *gin.Context) {
		// id := c.Param("id")
		// get via id ...

		c.JSON(http.StatusOK, gin.H{
			"article": models.Article{},
		})
	})

	e.POST("/article/:id/comment", func(c *gin.Context) {
		// id := c.Param("id")
		// commentContent := c.PostForm("content")
		// add comment ...

		c.JSON(http.StatusOK, gin.H{
			"message": "Comment added successfully",
		})
	})
}
