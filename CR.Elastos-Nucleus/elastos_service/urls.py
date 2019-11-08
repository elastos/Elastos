from django.conf.urls import url
from . import views

app_name = "elastos_service"

urlpatterns = [
    url(r'generateKey', views.generate_key, name="generateKey"),
    url(r'fileUpload', views.file_upload, name="fileUpload"),
    url(r'showfileContent', views.show_file_content, name='showFileContent'),
    url(r'uploadandScan', views.upload_and_scan, name="uploadAndScan"),
    url(r'verifyandshowContent', views.verify_and_show_content,
        name="verifyAndShowContent"),
    url(r'scanMessage', views.scan_message, name='scanMessage'),
    url(r'verifyMessage', views.verify_message, name='verifyMessage'),
    url(r'transferFile', views.transfer_file, name="transferFile"),
    url(r'documentation', views.documentation, name="documentation"),
    url(r'aboutUs', views.about_us, name='aboutUs'),
]
