function suggest_service() {
    var category  =  $('#id_service_category option:selected').text();
    var title  = $('#id_service_name').val()
    var description = $('#id_service_description').val()
    var reasoning = $('#id_service_reasoning').val()
    var csrftoken = getCookie('csrftoken');
        $.ajaxSetup({
            beforeSend: function (xhr, settings) {
                if (!csrfSafeMethod(settings.type) && !this.crossDomain) {
                    xhr.setRequestHeader("X-CSRFToken", csrftoken);
                }
            }
        });
        $.ajax({
            method: "POST",
            url: "/login/suggest_service",
            data:{
                'category':category,
                'title':title,
                'description':description,
                'reasoning':reasoning
            },
            success: function (data) {
                $('#suggest_service_modal').modal('hide')
            }

        })
}

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

function csrfSafeMethod(method) {
    // these HTTP methods do not require CSRF protection
    return (/^(GET|HEAD|OPTIONS|TRACE)$/.test(method));
}