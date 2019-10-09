from django.urls import path
from . import views

urlpatterns = [
    path(r'hello', views.helloWorld, name = 'helloWorld' ),
]