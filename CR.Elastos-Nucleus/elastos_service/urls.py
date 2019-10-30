from django.conf.urls import url
from . import  views

urlpatterns = [ 
    url(r'generate' , views.generate_key , name="generate key"),

]