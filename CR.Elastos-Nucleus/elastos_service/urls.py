from django.conf.urls import url
from . import  views

app_name = "elastos_service"

urlpatterns = [ 
    url(r'generateKey' , views.generate_key , name="generateKey"),
    url(r'fileUpload' , views.file_upload, name="fileUpload"),
]