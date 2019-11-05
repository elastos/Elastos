from django.conf.urls import url
from . import  views

urlpatterns = [ 
    url(r'generateKey/' , views.generate_key , name="generateKey"),
    url(r'fileUpload/' , views.upload , name="upload"),
]
