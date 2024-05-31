package controllers

import (
	"encoding/json"
	"fmt"
	myzip "gowebserver/WebServer/common/compress"
	recommend "gowebserver/WebServer/common/rcmd"
	models "gowebserver/WebServer/model"
	"io"
	"log"
	"net/http"
	"os"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

type ArticleController struct {
	DB *gorm.DB
}

func compress(a models.Article) string {
	jsonData, err := json.Marshal(a)
	if err != nil {
		panic("JSON marshaling failed:")
	}

	outFile, err := os.Create("ArticleId-" + a.Id + ".txt")
	if err != nil {
		panic(err)
	}
	defer outFile.Close()

	// gzipWriter := gzip.NewWriter(outFile)
	// _, err = gzipWriter.Write(jsonData)
	zipWriter := myzip.NewWriter(outFile)
	_, err = zipWriter.Write(jsonData)
	if err != nil {
		panic("Gzip compression failed:")
	}
	// gzipWriter.Close()
	zipWriter.Close()
	return string(jsonData)
}

func decompress(fileName string) models.Article {
	var article models.Article
	inFile, err := os.Open(fileName)
	if err != nil {
		panic(err)
	}
	defer inFile.Close()

	zipReader, err := myzip.NewReader(inFile)
	if err != nil {
		panic("Gzip decompression failed:")
	}
	defer zipReader.Close()
	json.NewDecoder(zipReader).Decode(&article)

	return article
}

func (ac *ArticleController) LoadArticle(e *gin.Engine) {
	e.GET("/recommend", ac.getRecommend)
	e.GET("/article/:id", ac.getArticle)
	e.GET("/article/search", ac.searchArticle)
	e.POST("/article", ac.postArticle)
	e.DELETE("/article/:id", ac.deleteArticle)
	e.POST("/article/:id/rate", ac.rateArticle)
	e.POST("/article/:id/compress", ac.compressArticle)
	e.POST("/article/decompress", ac.decompressArticle)

	e.POST("/article/:id/comment", func(c *gin.Context) {
		// id := c.Param("id")
		// commentContent := c.PostForm("content")
		// add comment ...

		c.JSON(http.StatusOK, gin.H{
			"message": "Comment added successfully",
		})
	})
}

func (ac *ArticleController) getRecommend(c *gin.Context) {
	var articles []models.Article
	var limit = 10 // how many articles to post
	var rt models.Rating
	var userRt []models.Rating
	var querys []string

	// ac.DB.Table("ratings").Where("user_id = ?", nowUser.Id).Find(&rt)
	ac.DB.Table("ratings").Where("user_id = ?", "1").Find(&rt)
	ac.DB.Table("ratings").Find(&userRt)
	len := len(userRt)
	tags := recommend.GenerateRcmd(rt, userRt, len)
	fmt.Println("tags: ", tags)
	ac.DB.Table("articles")

	for _, tag := range tags {
		querys = append(querys, "tags LIKE ?", fmt.Sprintf("%%%s%%", tag))
	}
	ac.DB.Where(strings.Join(querys, " OR "))
	ac.DB.Order("hot desc").Limit(limit).Find(&articles)

	c.JSON(200, articles)
}

func (ac *ArticleController) getArticle(c *gin.Context) {
	id := c.Param("id")
	ac.DB.Table("articles")
	fmt.Println("article id: ", id)

	var article models.Article
	if err := ac.DB.Where("id = ?", id).Find(&article).Error; err != nil || article.Id == "0" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid article id"})
		return
	}

	fmt.Println("from db ", article.Id, article.Title)

	c.JSON(200, article)
	article.Clicks += 1
	if err := ac.DB.Model(&article).Update("clicks", article.Clicks).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}

	if err := ac.DB.Model(&article).Update("hot", float64(article.Clicks)*article.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}
}

func (ac *ArticleController) postArticle(c *gin.Context) {
	var reqArticle models.Article
	ac.DB.Table("articles")
	c.BindJSON(&reqArticle)
	reqArticle.AuthorId = nowUser.Id
	fmt.Println("title: ", reqArticle.Title)

	if err := ac.DB.Create(&reqArticle).Error; err != nil {
		c.JSON(500, gin.H{"error": "failed to create" + err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "register successfully",
	})
}

func (ac *ArticleController) searchArticle(c *gin.Context) {
	query := c.Query("q")
	ac.DB.Table("articles")
	var articles []models.Article
	if err := ac.DB.Where("title LIKE ? OR content LIKE ? OR tags LIKE ?", "%"+query+"%", "%"+query+"%", "%"+query+"%").Find(&articles).Error; err != nil {
		c.JSON(500, gin.H{"error": "查询文章失败"})
		return
	}

	c.JSON(200, articles)
}

func (ac *ArticleController) rateArticle(c *gin.Context) {
	id := c.Param("id")
	ac.DB.Table("articles")
	score, err := strconv.ParseFloat(c.Query("score"), 64)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid score"})
		return
	}

	var article models.Article
	if err := ac.DB.Where("id = ?", id).Find(&article).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid article id"})
		return
	}

	r := article.Rate*float64(article.Rater) + score
	article.Rater += 1
	article.Rate = r / float64(article.Rater)

	if err := ac.DB.Model(&article).Update("rater", article.Rater).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rate"})
		return
	}

	if err := ac.DB.Model(&article).Update("rate", article.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rate"})
		return
	}

	if err := ac.DB.Model(&article).Update("hot", float64(article.Clicks)*article.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}

	ac.DB.Table("ratings")
	// user := nowUser
	tags := strings.Split(article.Tags, ",")
	var rating models.Rating

	// ac.DB.Where("user_id = ?", user.Id).Find(&rating)
	ac.DB.Where("user_id = ?", "1").Find(&rating)
	for _, tag := range tags {
		switch tag {
		case "art":
			rating.Art = rating.Art*0.75 + score*0.25
		case "culture":
			rating.Culture = rating.Culture*0.75 + score*0.25
		case "education":
			rating.Education = rating.Education*0.75 + score*0.25
		case "exploration":
			rating.Exploration = rating.Exploration*0.75 + score*0.25
		case "history":
			rating.History = rating.History*0.75 + score*0.25
		case "nature":
			rating.Nature = rating.Nature*0.75 + score*0.25
		case "science":
			rating.Science = rating.Science*0.75 + score*0.25
		}
	}

	if err := ac.DB.Save(&rating).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rating"})
		return
	}

	c.JSON(200, gin.H{"msg": "update successfully"})
}

func (ac *ArticleController) compressArticle(c *gin.Context) {
	var article models.Article
	id := c.Param("id")
	ac.DB.Table("articles")
	if err := ac.DB.Where("id = ?", id).Find(&article).Error; err != nil {
		c.JSON(500, gin.H{"error": "invalid article id"})
		return
	}

	compress(article)

	c.JSON(200, gin.H{"msg": "compress successfully"})
}

func (ac *ArticleController) decompressArticle(c *gin.Context) {
	var article models.Article
	file, header, err := c.Request.FormFile("file")
	if err != nil {
		log.Fatal(err)
	}

	out, err := os.Create(header.Filename)
	if err != nil {
		log.Fatal(err)
	}
	defer out.Close()
	_, err = io.Copy(out, file)
	if err != nil {
		log.Fatal(err)
	}
	article = decompress(header.Filename)

	c.JSON(200, article)
}

func (ac *ArticleController) deleteArticle(c *gin.Context) {
	id := c.Param("id")
	ac.DB.Table("articles")
	if err := ac.DB.Where("id = ?", id).Delete(&models.Article{}).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot delete article"})
		return
	}

	c.JSON(200, gin.H{"msg": "delete article successfully"})
}
