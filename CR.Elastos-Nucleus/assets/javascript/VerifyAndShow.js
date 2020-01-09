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

$('#id_select_files').change(function () {
    var fileName = $('#id_select_files option:selected').text()
    var csrftoken = getCookie('csrftoken');
    $.ajaxSetup({
        beforeSend: function(xhr, settings) {
            if (!csrfSafeMethod(settings.type) && !this.crossDomain) {
                xhr.setRequestHeader("X-CSRFToken", csrftoken);
            }
         }
    });
    $.ajax({
        type: "POST",
        url: 'service/verify_and_show',
        data:{
            'file_name':fileName
             },
        success:function (data) {
            try{
                var message_hash = data['fields']['message_hash']
                var signature = data['fields']['signature']
                var file_hash = data['fields']['file_hash']
                $('#id_message_hash').val(message_hash)
                $('#id_signature').val(signature)
                $('#id_file_hash').val(file_hash)
            }
            catch(err){
                console.log("Object did not read")
            }
        }

    })
})
