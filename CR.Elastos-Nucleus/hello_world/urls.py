from django.urls import path
from . import views

urlpatterns = [
    path(r'hello', views.hello_World, name = 'helloWorld'),
]