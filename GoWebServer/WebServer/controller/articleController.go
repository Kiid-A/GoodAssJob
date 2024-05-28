package controllers

import (
	"fmt"
	"gowebserver/WebServer/db"
	models "gowebserver/WebServer/model"
	"net/http"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

type ArticleController struct {
	DB *gorm.DB
}

func GetArticleController() *ArticleController {
	return &ArticleController{
		DB: db.InitArticleDB(),
	}
}

func (ac *ArticleController) LoadArticle(e *gin.Engine) {
	e.GET("/recommend", func(c *gin.Context) {
		c.HTML(200, "recommend.html", gin.H{})
	})

	e.GET("/article", ac.getArticle)
	e.GET("/article/:id", ac.showArticle)
	e.POST("/article", ac.postArticle)

	e.POST("/article/:id/comment", func(c *gin.Context) {
		// id := c.Param("id")
		// commentContent := c.PostForm("content")
		// add comment ...

		c.JSON(http.StatusOK, gin.H{
			"message": "Comment added successfully",
		})
	})
}

func (ac *ArticleController) getArticle(c *gin.Context) {
	id := c.Query("id")

	fmt.Println("article id: ", id)

	var article models.Article
	if err := ac.DB.Where("id = ?", id).First(&article).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid article id"})
		return
	}

	fmt.Println("from db ", article.Id, article.Title)

	c.JSON(http.StatusOK, gin.H{
		"code":            200,
		"article_content": article.Content,
		"message":         "send article successfully",
	})
}

func (ac *ArticleController) postArticle(c *gin.Context) {
	var reqArticle models.Article

	c.BindJSON(&reqArticle)

	fmt.Println("title: ", reqArticle.Title)

	var cnt int64
	ac.DB.Count(&cnt)
	reqArticle.Id = cnt + 1

	ac.DB.Create(&reqArticle)

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "register successfully",
	})
}

func (ac *ArticleController) showArticle(c *gin.Engine) {

}
