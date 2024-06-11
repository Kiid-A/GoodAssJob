
// document.getElementById('username')
// document.getElementById('passwd')
// console.log(document.getElementById('submitLogin'))

// 多打印
// 传统取值方式
// document.getElementById('submitLogin').onclick = function () {
//   console.dir(document.getElementById('username').value)
// }

if ($('#submitLogin')) {
  $('#submitLogin').click(function () {
    let username = $('#username').val()
    let passwd = $('#passwd').val()
    console.log(username, passwd)
    // API 应用程序接口
    $.ajax({
      url: `${baseUrl}/api/login?username=${username}&passwd=${passwd}`, // 存放即将与后端通信的接口 ****
      type: 'POST', // 请求类型 POST (data) GET (query params) PUT (data) DELETE 
      success: function (res) {
        console.log(res)
        if (res.code == 200) {
          alert('登录成功')
          window.location.href = './user_info.html'
        }
      }
    })
  })
}

if ($('#submitSignup')) {
  $('#submitSignup').click(function () {
    let username = $('#username').val()
    let passwd = $('#passwd').val()
    let confirmPasswd = $('#confirmPasswd').val()

    if (!username) {
      alert("请填写 username")
      return
    }
    if (!passwd) {
      alert("请填写 passwd")
      return
    }
    if (confirmPasswd != passwd) {
      alert("输入的密码不一致")
      return
    }
    let payload = JSON.stringify({
      username,
      passwd
    })
    $.ajax({
      url: `${baseUrl}/api/register`,
      type: 'POST',
      dataType: 'json',
      contentType: 'application/json; charset=utf-8',
      data: payload,
      success: function (res) {
        console.log(res)
      }
    })
  })

}


if ($('#submitArticle')) {
  $('#submitArticle').click(function () {
    let title = $('#title').val()
    let tags = $('#tags').val()
    let map_id = $('#map_id').val()
    let content = window.articleContent
    let article = JSON.stringify({
      title,
      tags,
      content,
      map_id
    })
    $.ajax({
      url: `${baseUrl}/api/article`,
      type: 'POST',
      dataType: 'json',
      contentType: 'application/json; charset=utf-8',
      data: article,
      success: function (res) {
        console.log(res)
        if (res.code == 200) {
          alert(res.message)
          window.location.reload()
        }
      }
    })
  })
}

// http://192.168.194.155:9999/api/login?username=root&passwd=1234

// www.baidu.com

// www.sougou.com

// 不同域名代表不同域
// Access-Control-Allow-Origin: *

// put /user/modifyInfo query
if (document.getElementById('infoSubmit')) {
  document.getElementById('infoSubmit').onclick = function () {
    let name = document.getElementById('name').value
    let gender = document.getElementById('gender').value
    $.ajax({
      url: `${baseUrl}/user/modifyInfo?name=${name}&gender=${gender}`,
      type: 'PUT',
      success: function (res) {
        console.log(res)
      }
    })
  }
}


// delete /api/article/{id} Path 参数
if (document.getElementById('infoSubmit')) {
  document.getElementById('infoSubmit').onclick = function () {
    let id = 'xxxxxxxx'
    $.ajax({
      url: `${baseUrl}/api/article/${id}`,
      type: 'DELEtE',
      success: function (res) {
        console.log(res)
      }
    })
  }
}

// 点击单选按钮后触发，即，我们选择“5”时，触发一个事件
if ($("input[name=rate]")) {
  $("input[name=rate]").click(function () {
    var rate = $(this).val();
    console.log(rate)
    $.ajax({
      url: `${baseUrl}/api/article/${id}/${rate}`,
      type: 'POST',
      success: function (res) {
        console.log(res)
      }
    })
  });
}

// 特殊情况，与DELETE请求参数类型类似
// get detail 获取内容详情
// url: `${baseUrl}/api/abcdefg/${id}`,
// url: `${baseUrl}/api/abcdefg?id=${id}`,


// get /api/article/recommend 
// 推荐的游记
function getData() {
  $.ajax({
    url: `${baseUrl}/api/article/recommend?isRecom=1`, // 存放即将与后端通信的接口 ****
    type: 'GET', // 请求类型 POST (data) GET (query params) PUT (data) DELETE 
    success: function (res) {
      console.log(res)
    }
  })
}


function getData() {
  $.ajax({
    url: `${baseUrl}/api/article/recommend?isRecom=0`, // 存放即将与后端通信的接口 ****
    type: 'GET', // 请求类型 POST (data) GET (query params) PUT (data) DELETE 
    success: function (res) {
      console.log(res)
    }
  })
}

function getData() {
  $.ajax({
    url: `${baseUrl}/api/article/recommend?isHot=1`, // 存放即将与后端通信的接口 ****
    type: 'GET', // 请求类型 POST (data) GET (query params) PUT (data) DELETE 
    success: function (res) {
      // console.log()
      let data = {
        code: 200,
        data: [
          {
            id: 1,
            title: 'this is title',
            desc: 'this is desc',
            createTime: '2024-06-02'
          }
        ],
        mes: '操作成功'
      }
    }
  })
}


// 新增文章的时候，需要设置type 有哪几个选项，1. 推荐，2. hot，不要手动设置，可以根据访问量来界定datadb 是否要将这条数据的isHot 设置为1
// isHot 存在多条：首先：只调用最前面的，5条；第一，访问量，