if(total_reached.toLowerCase() === "false"){
            $(document).ready(function () {
                $("#total_files_modal").modal('show');
                $('#id_uploaded_file').change(function () {
                    file_size = this.files[0].size;
                    file_size_mb = file_size / 1000000;
                    file_size_mb = file_size_mb.toFixed(2)
                    if (file_size > 1000000) {
                        alert("You can only upload files up to 5 Mb , your file was " + file_size_mb + " Mb")
                        $('#id_uploaded_file').val('')
                    }
                })
            });
}
else{
    $(document).ready(function () {
                $('#id_uploaded_file').change(function () {
                    file_size = this.files[0].size;
                    file_size_mb = file_size/1000000 ;
                    file_size_mb = file_size_mb.toFixed(2)
                    if(file_size > 1000000){
                        alert("You can only upload files up to 5 Mb , your file was "+ file_size_mb + " Mb")
                        $('#id_uploaded_file').val('')
                    }
                })
            })

}



