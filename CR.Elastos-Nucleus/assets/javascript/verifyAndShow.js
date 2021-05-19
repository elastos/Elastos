
$(function(ready) {
    $('#id_select_files').change(function () {
        var fileName = $('#id_select_files option:selected').text();
        var csrftoken = getCookie('csrftoken');
        $.ajaxSetup({
            beforeSend: function (xhr, settings) {
                if (!csrfSafeMethod(settings.type) && !this.crossDomain) {
                    xhr.setRequestHeader("X-CSRFToken", csrftoken);
                }
            }
        });
        $.ajax({
            type: "POST",
            url: 'service/verify_and_show',
            data: {
                'file_name': fileName
            },
            success: function (data) {
                try {
                    var message_hash = data['fields']['message_hash']
                    var signature = data['fields']['signature']
                    var file_hash = data['fields']['file_hash']
                    $('#id_message_hash').val(message_hash)
                    $('#id_signature').val(signature)
                    $('#id_file_hash').val(file_hash)
                } catch (err) {
                    console.log("Object did not read")
                }
            }

        })
    });
});
