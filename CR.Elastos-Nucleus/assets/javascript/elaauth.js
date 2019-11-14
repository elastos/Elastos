window.addEventListener('DOMContentLoaded', function() {
    let retryTimer;
    function ajax_get(url, callback) {
        var xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState === 4 && xmlhttp.status === 200) {
                console.log('responseText:' + xmlhttp.responseText);
                try {
                    var data = JSON.parse(xmlhttp.responseText);
                } catch(err) {
                    console.log(err.message + " in " + xmlhttp.responseText);
                    return;
                }
                callback(data);
            }
        };

        xmlhttp.open("GET", url, true);
        xmlhttp.send();
    }
    function __ela_checkAuth() {
        ajax_get('/check_ela_auth', function(data) {
            if (data && 'redirect' in data) {
                clearTimeout(retryTimer);
                window.location.href = data['redirect'];
            }
        });
        retryTimer = setTimeout(__ela_checkAuth, 2000);
    }
    __ela_checkAuth();
}, false);