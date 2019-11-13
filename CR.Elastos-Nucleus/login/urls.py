from django.urls import path

from . import views

app_name = "login"

urlpatterns = [
    path('checkElaAuth', views.check_ela_auth, name="checkElaAuth"),
    path('didCallback', views.did_callback, name="didCallback"),
    path('register', views.register, name="register"),
    path('login', views.login, name="login"),
    path('home', views.home, name="home"),
    path('logout', views.logout, name="logout")
]
