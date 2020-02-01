from django.conf.urls import url
from . import views

app_name = "service"


urlpatterns = [

    url(r'generate_key', views.generate_key, name="generate_key"),
    url(r'upload_and_sign', views.upload_and_sign, name="upload_and_sign"),
    url(r'verify_and_show', views.verify_and_show, name="verify_and_show"),
    url(r'create_wallet', views.create_wallet, name="create_wallet"),
    url(r'view_wallet', views.view_wallet, name="view_wallet"),
    url(r'request_ela', views.request_ela, name="request_ela"),
    url(r'deploy_eth_contract', views.deploy_eth_contract,
        name="deploy_eth_contract"),
    url(r'watch_eth_contract', views.watch_eth_contract, name="watch_eth_contract"),
    url(r'run_eth_contract', views.run_eth_contract, name="run_eth_contract")
]
