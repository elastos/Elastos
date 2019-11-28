from django.conf.urls import url
from . import views

app_name = "service"

urlpatterns = [
    url(r'generateKey', views.generate_key, name="generateKey"),
    url(r'uploadAndSign', views.upload_and_sign, name="uploadAndSign"),
    url(r'verifyAndShow', views.verify_and_show, name="verifyAndShow"),
    url(r'requestELA', views.request_ela, name='requestELA'),
    url(r'voteSupernodes', views.vote_supernodes, name='voteSupernodes'),
    url(r'runContract', views.run_contract, name="runContract"),
    url(r'saveDidData', views.save_did_data, name="saveDidData"),
    url(r'retrieveDidData', views.retrieve_did_data, name="retrieveDidData"),
]
