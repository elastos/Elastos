
$(function(ready) {
    $('#id_select_file').change(function () {
        var fileName = $('#id_select_file option:selected').text();
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
            url: 'service/watch_eth_contract',
            data: {
                'file_name': fileName
            },
            success: data => {
                try {
                    console.log(data)
                    var contract_name = data['fields']['contract_name'];
                    var contract_address = data['fields']['contract_address'];
                    var contract_code_hash = data['fields']['contract_code_hash'];
                    $('#id_contract_address').val(contract_address)
                    $('#id_contract_name').val(contract_name)
                    $('#id_contract_code_hash').val(contract_code_hash)
                } catch (err) {
                    console.log("Object did not read")
                }
            }

        })
    });
});
