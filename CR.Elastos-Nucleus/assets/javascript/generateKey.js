function getCookie(name) {
    var cookieValue = null;
    if (document.cookie && document.cookie !== '') {
        var cookies = document.cookie.split(';');
        for (var i = 0; i < cookies.length; i++) {
            var cookie = cookies[i].trim();
            // Does this cookie string begin with the name we want?
            if (cookie.substring(0, name.length + 1) === (name + '=')) {
                cookieValue = decodeURIComponent(cookie.substring(name.length + 1));
                break;
            }
        }
    }
    return cookieValue;
}

function getAPI() {
    var token = getCookie('csrftoken');
   $.ajax({
        headers: { "X-CSRFToken": token},
        method:"POST",
        url:'generateKey',
        data:{

        },
        dataType:'json',
        success: function (data) {
            console.log("comes in here");
            $("#apiKey").attr('value' , data['API_KEY'])
        }
   });
}