
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
