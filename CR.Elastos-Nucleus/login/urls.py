from django.urls import path

from . import views

app_name = "login"

urlpatterns = [
    path('check_ela_auth', views.check_ela_auth, name="check_ela_auth"),
    path('did_callback', views.did_callback, name="did_callback"),
    path('register', views.register, name="register"),
    path('sign_in', views.sign_in, name="sign_in"),
    path('home', views.home, name="home"),
    path('sign_out', views.sign_out, name="sign_out")
]
