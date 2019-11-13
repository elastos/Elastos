from django.conf.urls import url
from . import views

app_name = "login"

urlpatterns = [
    url(r'checkElaAuth', views.check_ela_auth, name="checkElaAuth"),
    url(r'didCallback', views.did_callback, name="didCallback"),
    url(r'register', views.register, name="register"),
    url(r'login', views.login, name="login"),
    url(r'home', views.home, name="home"),
    url(r'logout', views.logout, name="logout")
]
