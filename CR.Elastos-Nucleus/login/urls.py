from django.urls import path
from django.conf.urls import url
from . import views

urlpatterns = [
    path('', views.landing_page, name='landing_page'),
    path('signin/', views.sign_in, name='sign_in'),
    path('signup/', views.sign_up, name='sign_up'),
    path('confirmation/', views.confirmation, name='confirmation'),
]
