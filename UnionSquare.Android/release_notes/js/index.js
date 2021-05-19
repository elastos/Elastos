// $(function () {
//     langgua(getUrlParam("langua")||"ch")
// })

//判断iPhone|iPad|iPod|iOS
if (/(iPhone|iPad|iPod|iOS)/i.test(navigator.userAgent)) {
    $('#download').attr('href','https://itunes.apple.com/us/app/elastos-wallet/id1453348077?ls=1&mt=8');
} else if (/(Android)/i.test(navigator.userAgent)) {  //判断Android
    $('#download').attr('href','https://download.elastos.org/app/ela-wallet/ela-wallet.apk');
} else {  // PC端
    $('#download').attr('href','https://download.elastos.org/app/elastoswallet/elastos_wallet.apk');
}

//获取url中的参数
function getUrlParam(name) {
    var reg = new RegExp("(^|&)" + name + "=([^&]*)(&|$)"); //构造一个含有目标参数的正则表达式对象
    var r = window.location.search.substr(1).match(reg);  //匹配目标参数
    if (r != null) return unescape(r[2]); return null; //返回参数值
}
