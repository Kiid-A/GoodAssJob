package controllers

import (
	"encoding/json"
	"fmt"
	config "gowebserver/WebServer/common"
	"gowebserver/WebServer/common/navi"
	recommend "gowebserver/WebServer/common/rcmd"
	models "gowebserver/WebServer/model"
	"net/http"
	"os"
	"strconv"
	"strings"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

type MapController struct {
	DB *gorm.DB
}

func (mc *MapController) LoadMap(e *gin.Engine) {
	e.GET("/api/map/:id/relate", mc.getRelate)
	e.GET("/api/map/recommend", mc.getRecommend)
	e.GET("/api/map/fine", mc.getFine)
	e.GET("/api/map/latest", mc.getLatest)
	e.GET("/api/map/:id", mc.getMap)
	e.GET("/api/map/search", mc.searchMap)
	e.GET("/api/map/:id/jump", mc.jump)
	e.POST("/api/map", mc.postMap)
	e.POST("/api/map/:id/search", mc.searchSpot)
	e.POST("/api/map/:id/img", mc.postImg)
	e.POST("/api/map/:id/json", mc.postJSON)
	e.POST("/api/map/:id/rate", mc.rateMap)
	e.POST("/api/map/:id/navi", mc.navi)
	e.POST("/api/map/:id/scan", mc.scan)
}

func (mc *MapController) searchSpot(c *gin.Context) {
	q := c.Query("q")
	id := c.Param("id")

	mapsName := config.MapPath + "map-" + id + ".json"

	bytes, err := os.ReadFile(mapsName)
	if err != nil {
		c.JSON(500, gin.H{"err": "Invalid map_id"})
		return
	}
	m := &[]navi.Point{}
	err = json.Unmarshal(bytes, m)
	if err != nil {
		c.JSON(500, gin.H{"err": "Failed to parse json"})
		return
	}

	ret := &[]navi.Point{}
	for _, p := range *m {
		if strings.Contains(p.Name, q) {
			*ret = append(*ret, p)
		}
	}

	c.JSON(200, ret)
}

func (mc *MapController) getRelate(c *gin.Context) {
	var articles []models.Article
	var limit = 4 // how many articles to post
	var querys []string
	id := c.Param("id")
	var reqMap models.Article
	if err := mc.DB.Table("maps").Where("id = ?", id).Find(&reqMap).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid map id" + err.Error()})
		return
	}

	tags := strings.Split(reqMap.Tags, ",")
	fmt.Println("tags: ", tags)

	for _, tag := range tags {
		querys = append(querys, "tags LIKE ?", fmt.Sprintf("%%%s%%", tag))
	}
	mc.DB.Table("articles").Where(strings.Join(querys, " OR "))
	mc.DB.Order("hot desc").Limit(limit).Find(&articles)

	c.JSON(200, articles)
}

func (mc *MapController) getLatest(c *gin.Context) {
	var maps []models.Map
	var limit = 4 // how many articles to post

	mc.DB.Table("maps")
	mc.DB.Order("updated_at desc").Limit(limit).Find(&maps)

	c.JSON(200, maps)
}

func (mc *MapController) getFine(c *gin.Context) {
	var maps []models.Map
	var limit = 4 // how many articles to post

	mc.DB.Table("maps")
	mc.DB.Order("rate desc").Limit(limit).Find(&maps)

	c.JSON(200, maps)
}

func (mc *MapController) postJSON(c *gin.Context) {
	var maps models.Map
	file, err := c.FormFile("json")
	id := c.Param("id")
	if err != nil {
		c.JSON(500, gin.H{"error": "failed to open json " + err.Error()})
		return
	}

	fmt.Println("json: ", file.Filename)
	mc.DB.Table("maps")

	if err := mc.DB.Where("id = ?", id).Find(&maps).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid article ID " + err.Error()})
		return
	}

	if err := c.SaveUploadedFile(file, maps.Url); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "post successfully",
	})
}

func (mc *MapController) jump(c *gin.Context) {
	var limit int = 5
	var articles []models.Article
	id := c.Param("id")
	if err := mc.DB.Table("articles").Where("map_id = ?", id).Order("hot desc").Limit(limit).Find(&articles).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid map_id " + err.Error()})
		return
	}

	c.JSON(200, articles)
}

func (mc *MapController) postMap(c *gin.Context) {
	var maps models.Map
	var cnt int64
	c.BindJSON(&maps)
	mc.DB.Table("maps").Count(&cnt)
	maps.Id = strconv.Itoa(int(cnt + 1))
	maps.Url = config.MapPath + "graph-" + maps.Id + ".json"
	if err := mc.DB.Create(&maps).Error; err != nil {
		c.JSON(500, gin.H{"error": "failed to create" + err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "post successfully",
	})
}

func (mc *MapController) postImg(c *gin.Context) {
	file, err := c.FormFile("img")
	id := c.Param("id")
	if err != nil {
		c.JSON(500, gin.H{"error": "failed to open img " + err.Error()})
		return
	}
	fmt.Println("img: ", file.Filename)
	if err := c.SaveUploadedFile(file, config.MiPath+id+file.Filename+".jpg"); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": err.Error()})
		return
	}

	var img models.MapImg
	img.Path = config.MiPath + id + file.Filename
	img.MapId = id
	map_id, _ := strconv.Atoi(img.MapId)
	var maps models.Map
	mc.DB.Table("maps")
	if err := mc.DB.Where("id = ?", map_id).Find(&maps).Error; err != nil {
		c.JSON(500, gin.H{"error": "Invalid article ID " + err.Error()})
		return
	}
	maps.Img += "," + img.Path
	if err = mc.DB.Model(&maps).Update("img", maps.Img).Error; err != nil {
		c.JSON(500, gin.H{"error": "Failed to update img " + err.Error()})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "post successfully",
	})
}

func (mc *MapController) getRecommend(c *gin.Context) {
	var maps []models.Map
	var limit = 4 // how many articles to post
	var rt models.Rating
	var userRt []models.Rating
	var querys []string

	mc.DB.Table("ratings").Where("user_id = ?", nowUser.Id).Find(&rt)
	// mc.DB.Table("ratings").Where("user_id = ?", "1").Find(&rt)
	mc.DB.Table("ratings").Find(&userRt)
	len := len(userRt)
	tags := recommend.GenerateRcmd(rt, userRt, len)
	fmt.Println("tags: ", tags)
	mc.DB.Table("maps")

	for _, tag := range tags {
		querys = append(querys, "tags LIKE ?", fmt.Sprintf("%%%s%%", tag))
	}
	mc.DB.Where(strings.Join(querys, " OR "))
	mc.DB.Order("hot desc").Limit(limit).Find(&maps)

	c.JSON(200, maps)
}

func (mc *MapController) getMap(c *gin.Context) {
	id := c.Param("id")
	mc.DB.Table("maps")
	fmt.Println("map id: ", id)

	var maps models.Map
	if err := mc.DB.Where("id = ?", id).Find(&maps).Error; err != nil || maps.Id == "0" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid maps id"})
		return
	}

	fmt.Println("from db ", maps.Id, maps.Name)

	mapsName := config.MapPath + "map-" + maps.Id + ".json"

	bytes, err := os.ReadFile(mapsName)
	if err != nil {
		c.JSON(500, gin.H{"err": "Invalid map_id"})
		return
	}
	m := &[]navi.Point{}
	err = json.Unmarshal(bytes, m)
	if err != nil {
		c.JSON(500, gin.H{"err": "Failed to parse json"})
		return
	}

	c.JSON(200, gin.H{
		"map":   maps,
		"spots": m,
	})
	maps.Clicks += 1
	if err := mc.DB.Model(&maps).Update("clicks", maps.Clicks).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}

	if err := mc.DB.Model(&maps).Update("hot", float64(maps.Clicks)*maps.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}
}

func (mc *MapController) searchMap(c *gin.Context) {
	query := c.Query("q")
	mc.DB.Table("maps")
	var maps []models.Map
	if err := mc.DB.Where("name LIKE ? OR summary LIKE ? OR tags LIKE ?", "%"+query+"%", "%"+query+"%", "%"+query+"%").Find(&maps).Error; err != nil {
		c.JSON(500, gin.H{"error": "failed to search maps"})
		return
	}

	c.JSON(200, maps)
}

func (mc *MapController) rateMap(c *gin.Context) {
	id := c.Param("id")
	mc.DB.Table("maps")
	score, err := strconv.ParseFloat(c.Query("score"), 64)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid score"})
		return
	}

	var maps models.Map
	if err := mc.DB.Where("id = ?", id).Find(&maps).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid map id"})
		return
	}

	r := maps.Rate*float64(maps.Rater) + score
	maps.Rater += 1
	maps.Rate = r / float64(maps.Rater)

	if err := mc.DB.Model(&maps).Update("rater", maps.Rater).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rate"})
		return
	}

	if err := mc.DB.Model(&maps).Update("rate", maps.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rate"})
		return
	}

	if err := mc.DB.Model(&maps).Update("hot", float64(maps.Clicks)*maps.Rate).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update clicks"})
		return
	}

	mc.DB.Table("ratings")
	user := nowUser
	tags := strings.Split(maps.Tags, ",")
	var rating models.Rating

	mc.DB.Where("user_id = ?", user.Id).Find(&rating)
	// mc.DB.Where("user_id = ?", "1").Find(&rating)
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

	if err := mc.DB.Save(&rating).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "cannot update rating"})
		return
	}

	c.JSON(200, gin.H{"msg": "update successfully"})
}

func (mc *MapController) navi(c *gin.Context) {
	// graph-id
	// start: idx, temp: []idx, target: idx
	// return: timefirst{time: double, dist: double, pts: []idx}
	//  	   distfirst{same}
	//         facility: {toilet: idx, resturant: idx}
	var naviReq models.NaviReq
	c.BindJSON(&naviReq)
	var err error
	naviReq.GraphId, err = strconv.Atoi(c.Param("id"))
	if err != nil {
		c.JSON(500, gin.H{"error": "navi err"})
	}
	results := navi.Navi(naviReq)

	c.JSON(200, results)
}

func (mc *MapController) scan(c *gin.Context) {
	var scanReq models.ScanReq
	c.BindJSON(&scanReq)
	var err error
	scanReq.GraphId, err = strconv.Atoi(c.Param("id"))
	if err != nil {
		c.JSON(500, gin.H{"error": "scan err"})
	}
	results := navi.Scan(scanReq)

	c.JSON(200, results)
}
