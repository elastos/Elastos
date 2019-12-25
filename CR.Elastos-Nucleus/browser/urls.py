from django.conf.urls import url
from . import views

app_name = "browser"


urlpatterns = [

    url(r'mainchain', views.mainchain, name="mainchain"),
    url(r'sidechain_did', views.sidechain_did, name="sidechain_did"),
    url(r'sidechain_token', views.sidechain_token, name="sidechain_token"),
    url(r'sidechain_eth', views.sidechain_eth, name="sidechain_eth"),
    url(r'sidechain_neo', views.sidechain_neo, name="sidechain_neo"),

]
