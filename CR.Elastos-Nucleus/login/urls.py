from django.urls import path
from django.conf.urls import url

from . import views

app_name = "login"

urlpatterns = [
    path('check_ela_auth', views.check_ela_auth, name="check_ela_auth"),
    path('did_callback', views.did_callback, name="did_callback"),
    path('register', views.register, name="register"),
    url(r'^activate/(?P<uidb64>[0-9A-Za-z_\-]+)/(?P<token>[0-9A-Za-z]{1,13}-[0-9A-Za-z]{1,20})/$',
        views.activate, name="activate"),
    path('sign_in', views.sign_in, name="sign_in"),
    path('home', views.home, name="home"),
    path('sign_out', views.sign_out, name="sign_out")
]
