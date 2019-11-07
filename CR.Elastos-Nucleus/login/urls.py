from django.urls import path
from django.conf.urls import url
from . import views

urlpatterns = [
    path(r'landing/', views.landing_page, name='landing_page'),
    path(r'signin/', views.sign_in, name='sign_in'),
    path(r'signup/', views.sign_up, name='sign_up'),
    path(r'confirmation/', views.confirmation, name='confirmation'),
]
