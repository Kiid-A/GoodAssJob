package controllers

import (
	"fmt"
	models "gowebserver/WebServer/model"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

var nowUser *models.User

func GetNowUser() *models.User {
	return nowUser
}

type UserController struct {
	DB *gorm.DB
}

func (uc *UserController) LoadUser(e *gin.Engine) {
	e.GET("/login", func(c *gin.Context) {
		c.HTML(200, "login.html", gin.H{})
	})
	e.GET("/register", func(c *gin.Context) {
		c.HTML(200, "register.html", gin.H{})
	})

	e.GET("/user", uc.userIndex)
	e.POST("/login", uc.login)
	e.POST("/register", uc.register)

	// e.GET("/user/", register)
}

func (uc *UserController) userIndex(c *gin.Context) {
	c.HTML(200, "login.html", gin.H{})
}

func (uc *UserController) login(c *gin.Context) {
	// c.HTML(201, "login.html", gin.H{})
	uc.DB.Table("users")
	username := c.Query("username")
	passwd := c.Query("passwd")

	fmt.Println("guest ", username, passwd)

	var user models.User
	uc.DB.Where("username = ?", username).First(&user)
	if user.ID != 0 {
		c.JSON(500, gin.H{"error": "username already exist"})
	}

	if flag := strings.Compare(passwd, user.Passwd); flag != 0 {
		c.JSON(http.StatusUnprocessableEntity, gin.H{
			"code":    422,
			"message": "密码错误",
		})
		return
	}

	nowUser = &user

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "登录成功",
	})
}

func (uc *UserController) register(c *gin.Context) {
	// c.HTML(201, "register.html", gin.H{})
	uc.DB.Table("users")
	var reqUser models.User
	c.BindJSON(&reqUser)

	if err := uc.DB.Create(&reqUser).Error; err != nil {
		c.JSON(500, "cannot create user "+err.Error())
		return
	}
	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "register successfully",
	})

	rating := models.Rating{
		UserId: reqUser.Id,
	}
	uc.DB.Table("ratings")

	if err := uc.DB.Create(&rating).Error; err != nil {
		c.JSON(500, "cannot create rating "+err.Error())
		return
	}

	nowUser = &reqUser
}
