$(function () {
    langgua(getUrlParam("langua")||"ch")
})
function langgua(lang){
    $.ajax({
        url:lang+'.json',
        type:'get',
        dataType : "jsonp",//jsonp数据类型
        jsonp: "jsonpCallback",//服务端用于接收callback调用的function名的参数
        success:function(data){
            if (lang == 'en') {
                $('header').find('div').eq(2).find("i").eq(1).css("color","#4A8AFF").prev().css("color","#333");
            }else{
                $('header').find('div').eq(2).find("i").eq(0).css("color","#4A8AFF").next().css("color","#333");
            }
            $('header').find('div').eq(1).text(data.text);
            $('.content_box').eq(0).find("p").eq(0).text(data.text1);
            $('.content_box').eq(0).find("p").eq(1).text(data.text5);
            $('.content_box').eq(0).find("p").eq(2).text(data.text6);
            $('.content_box').eq(0).find("p").eq(3).text(data.text7);
            $('.content_box').eq(0).find("p").eq(4).text(data.text8);
            $('.content_box').eq(1).find("p").eq(0).text(data.text2);
            $('.content_box').eq(1).find("p").eq(1).text(data.text9);
            $('#download').text(data.text10)
        }
    })
}
//判断iPhone|iPad|iPod|iOS
if (/(iPhone|iPad|iPod|iOS)/i.test(navigator.userAgent)) {
    $('#download').attr('href','https://itunes.apple.com/us/app/elastos-wallet/id1453348077?ls=1&mt=8');
} else if (/(Android)/i.test(navigator.userAgent)) {  //判断Android
    $('#download').attr('href','https://download.elastos.org/app/elastoswallet/elastos_wallet.apk');
} else {  // PC端
    $('#download').attr('href','https://download.elastos.org/app/elastoswallet/elastos_wallet.apk');
}
//获取url中的参数
function getUrlParam(name) {
    var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)"); //构造一个含有目标参数的正则表达式对象
    var r = window.location.search.substr(1).match(reg);  //匹配目标参数
    if (r != null) return unescape(r[2]); return null; //返回参数值
}
