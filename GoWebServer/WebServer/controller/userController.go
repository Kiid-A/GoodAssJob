package controllers

import (
	"fmt"
	"gowebserver/WebServer/db"
	models "gowebserver/WebServer/model"
	"net/http"
	"strings"

	"github.com/gin-gonic/gin"
	"gorm.io/gorm"
)

type UserController struct {
	DB *gorm.DB
}

func GetUserController() *UserController {
	return &UserController{
		DB: db.InitUserDB(),
	}
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

	username := c.Query("username")
	passwd := c.Query("passwd")

	fmt.Println("guest ", username, passwd)

	var user models.User
	if err := uc.DB.Where("username = ?", username).First(&user).Error; err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Invalid email or password"})
		return
	}

	fmt.Println("from db ", user.Username, user.Passwd)

	if flag := strings.Compare(passwd, user.Passwd); flag != 0 {
		c.JSON(http.StatusUnprocessableEntity, gin.H{
			"code":    422,
			"message": "密码错误",
		})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "登录成功",
	})
}

func (uc *UserController) register(c *gin.Context) {
	// c.HTML(201, "register.html", gin.H{})

	username := c.Query("username")
	passwd := c.Query("passwd")

	var user models.User
	uc.DB.Where("username = ?", username).First(&user)
	if user.ID != 0 {
		c.JSON(http.StatusUnprocessableEntity, gin.H{
			"code":    422,
			"message": "用户已存在",
		})
		return
	}

	newUser := &models.User{
		Username: username,
		Passwd:   passwd,
	}

	uc.DB.Create(newUser)

	c.JSON(http.StatusOK, gin.H{
		"code":    200,
		"message": "register successfully",
	})
}
