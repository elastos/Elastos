$(document).ready(function () {
    $('#id_uploaded_file').change(function () {
        file_size = this.files[0].size
        file_size_mb = file_size/1000000 ;
        file_size_mb = file_size_mb.toFixed(2)
        if(file_size > 1000000){
            alert("You can only upload files up to 1 Mb , your file was "+ file_size_mb + " Mb")
            $('#id_uploaded_file').val('')
        }
    })
})