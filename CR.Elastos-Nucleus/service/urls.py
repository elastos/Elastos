from django.conf.urls import url
from . import views

app_name = "service"


urlpatterns = [

    url(r'generate_key', views.generate_key, name="generate_key"),
    url(r'upload_and_sign', views.upload_and_sign, name="upload_and_sign"),
    url(r'verify_and_show', views.verify_and_show, name="verify_and_show"),
    url(r'request_ela', views.request_ela, name="request_ela"),
    url(r'vote_supernodes', views.vote_supernodes, name="vote_supernodes"),
    url(r'run_eth_contract', views.run_eth_contract, name="run_eth_contract"),
    url(r'save_did_data', views.save_did_data, name="save_did_data"),
    url(r'retrieve_did_data', views.retrieve_did_data, name="retrieve_did_data"),

]
