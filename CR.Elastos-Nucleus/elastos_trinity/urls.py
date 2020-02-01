from django.conf.urls import url
from . import views

app_name = "elastos_trinity"


urlpatterns = [

    url(r'dapp_store_dashboard', views.dapp_store_dashboard, name="dapp_store_dashboard"),

]
