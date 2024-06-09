package controllers

import (
	"encoding/json"
	"fmt"
	config "gowebserver/WebServer/common"
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

	outFile, err := os.Create("ArticleId-" + strconv.Itoa(a.Id) + ".txt")
	if err != nil {
		panic(err)
	}
	defer outFile.Close()

	zipWriter := myzip.NewWriter(outFile)
	_, err = zipWriter.Write(jsonData)
	if err != nil {
		panic("compression failed:")
	}
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
		panic("decompression failed:")
	}
	defer zipReader.Close()
	json.NewDecoder(zipReader).Decode(&article)

	return article
}

func (ac *ArticleController) LoadArticle(e *gin.Engine) {
	e.GET("/article_manage.html", func(c *gin.Context) {
		c.HTML(200, "article_manage.html", gin.H{})
	})
	e.GET("/add_article.html", func(c *gin.Context) {
		c.HTML(200, "add_article.html", gin.H{})
	})
	e.GET("/diary_show.html", func(c *gin.Context) {
		c.HTML(200, "diary_show.html", gin.H{})
	})
	e.GET("/diary.html", func(c *gin.Context) {
		c.HTML(200, "diary.html", gin.H{})
	})
	e.GET("/recommend.html", func(c *gin.Context) {
		c.HTML(200, "recommend.html", gin.H{})
	})
	e.GET("/recommend_show.html", func(c *gin.Context) {
		c.HTML(200, "recommend_show.html", gin.H{})
	})

	e.GET("/api/article/:id/relate", ac.getRelate)
	e.GET("/api/article/recommend", ac.getRecommend)
	e.GET("/api/article/fine", ac.getFine)
	e.GET("/api/article/latest", ac.getLatest)
	e.GET("/api/article/:id", ac.getArticle)
	e.GET("/api/article/search", ac.searchArticle)
	e.GET("/api/article/filter", ac.filter)
	e.GET("/api/article/:id/jump", ac.jump)
	e.POST("/api/article", ac.postArticle)
	e.POST("/api/article/:id/img", ac.postImg)
	e.POST("/api/article/:id/rate", ac.rateArticle)
	e.POST("/api/article/:id/compress", ac.compressArticle)
	e.POST("/api/article/decompress", ac.decompressArticle)
	e.DELETE("/api/article/:id", ac.deleteArticle)
	e.POST("/api/article/:id/comment", func(c *gin.Context) {
		// id := c.Param("id")
		// commentContent := c.PostForm("content")
		// add comment ...
		c.JSON(http.StatusOK, gin.H{
			"message": "Comment added successfully",
		})
	})
}

func (ac *ArticleController) getRelate(c *gin.Context) {
	var articles []models.Article
	var limit = 4 // how many articles to post
	var querys []string
	id := c.Param("id")
	var reqArticle models.Article
	if err := ac.DB.Table("articles").Where("id = ?", id).Find(&reqArticle).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid article id" + err.Error()})
		return
	}

	tags := strings.Split(reqArticle.Tags, ",")
	fmt.Println("tags: ", tags)

	for _, tag := range tags {
		querys = append(querys, "tags LIKE ?", fmt.Sprintf("%%%s%%", tag))
	}
	ac.DB.Where(strings.Join(querys, " OR ")).Not("id = ?", id)
	ac.DB.Order("hot desc").Limit(limit).Find(&articles)

	c.JSON(200, articles)
}

func (ac *ArticleController) getLatest(c *gin.Context) {
	var articles []models.Article
	var limit = 4 // how many articles to post

	ac.DB.Table("articles")
	ac.DB.Order("updated_at desc").Limit(limit).Find(&articles)

	c.JSON(200, articles)
}

func (ac *ArticleController) getFine(c *gin.Context) {
	var articles []models.Article
	var limit = 4 // how many articles to post

	ac.DB.Table("articles")
	ac.DB.Order("rate desc").Limit(limit).Find(&articles)

	c.JSON(200, articles)
}

func (ac *ArticleController) jump(c *gin.Context) {
	var article models.Article
	var maps models.Map
	id := c.Param("id")
	i, err := strconv.Atoi(id)
	if err = ac.DB.Table("articles").Where("id = ?", i).Find(&article).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid article id" + err.Error()})
		return
	}
	mid := article.MapId
	if err = ac.DB.Table("maps").Where("id = ?", mid).Find(&maps).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid maps id" + err.Error()})
		return
	}

	c.JSON(200, maps)
}

func (ac *ArticleController) filter(c *gin.Context) {
	var articles []models.Article
	tag := c.Query("q")
	if err := ac.DB.Table("articles").Where("tags LIKE ?", "%"+tag+"%").Order("hot desc").Find(&articles).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid tag"})
		return
	}

	c.JSON(200, articles)
}

func (ac *ArticleController) getRecommend(c *gin.Context) {
	var articles []models.Article
	var limit = 4 // how many articles to post
	var rt models.Rating
	var userRt []models.Rating
	var querys []string

	ac.DB.Table("ratings").Where("user_id = ?", nowUser.Id).Find(&rt)
	// ac.DB.Table("ratings").Where("user_id = ?", "1").Find(&rt)
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
	if err := ac.DB.Where("id = ?", id).Find(&article).Error; err != nil || article.Id == 0 {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid article id"})
		return
	}

	fmt.Println("from db ", article.Id, article.Title)

	article.Clicks += 1
	if err := ac.DB.Model(&article).Update("clicks", article.Clicks).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}

	if err := ac.DB.Model(&article).Update("hot", float64(article.Clicks)*article.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}

	c.JSON(200, article)
}

func (ac *ArticleController) postImg(c *gin.Context) {
	file, err := c.FormFile("img")
	id := c.Param("id")
	if err != nil {
		c.JSON(500, gin.H{"error": "failed to open img " + err.Error()})
		return
	}
	fmt.Println("img: ", file.Filename)
	if err := c.SaveUploadedFile(file, config.AiPath+id+file.Filename+".jpg"); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	var img models.ArticleImg
	img.Path = config.AiPath + id + file.Filename
	img.ArticleId = id
	article_id, _ := strconv.Atoi(img.ArticleId)
	var article models.Article
	ac.DB.Table("articles")
	if err := ac.DB.Where("id = ?", article_id).Find(&article).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid article ID " + err.Error()})
		return
	}
	article.Img += "," + img.Path
	if err = ac.DB.Model(&article).Update("img", article.Img).Error; err != nil {
		c.JSON(500, gin.H{"error": "Failed to update img " + err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "post successfully",
	})
}

func (ac *ArticleController) postArticle(c *gin.Context) {
	var reqArticle models.Article
	var maps models.Map
	c.BindJSON(&reqArticle)
	reqArticle.AuthorId = nowUser.Id
	ac.DB.Table("maps").Where("id = ?", reqArticle.MapId).Find(&maps)
	reqArticle.Tags = maps.Tags
	fmt.Println("title: ", reqArticle.Title)
	ac.DB.Table("articles")
	if err := ac.DB.Create(&reqArticle).Error; err != nil {
		c.JSON(500, gin.H{"error": "failed to create" + err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "post successfully",
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
	article.Clicks += 1

	if err := ac.DB.Model(&article).Update("rater", article.Rater).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rater"})
		return
	}

	if err := ac.DB.Model(&article).Update("rate", article.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rate"})
		return
	}

	if err := ac.DB.Model(&article).Update("hot", float64(article.Clicks)*article.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update hot"})
		return
	}

	if err := ac.DB.Model(&article).Update("clicks", article.Clicks).Error; err != nil {
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
