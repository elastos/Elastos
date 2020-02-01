from django.urls import path
from django.conf.urls import url

from . import views

app_name = "login"

urlpatterns = [
    path('register', views.register, name="register"),
    path('edit_profile', views.edit_profile, name="edit_profile"),
    url(r'^activate/(?P<uidb64>[0-9A-Za-z_\-]+)/(?P<token>[0-9A-Za-z]{1,13}-[0-9A-Za-z]{1,20})/$',
        views.activate, name="activate"),
    path('feed', views.feed, name="feed"),
    path('sign_out', views.sign_out, name="sign_out"),
    path('suggest_service', views.suggest_service, name='suggest_service'),
    path('get_user_data', views.get_user_data, name='get_user_data'),
    path('remove_user_data', views.remove_user_data, name='remove_user_data'),
]
