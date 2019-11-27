from django.shortcuts import render, redirect
import json
from django.http import HttpResponse
import requests


def generate_key(request):
    if(request.method == 'POST'):
        r = requests.get("http://localhost:8888/api/1/common/generateAPIKey")
        if(r.status_code == 200):
            API_KEY = r.json()["API Key"]
            context = {
                'API_KEY': API_KEY,
            }
            print(context)
            sent_json = json.dumps({'API_KEY': API_KEY})
            return HttpResponse(sent_json, content_type='application/JSON')
        else:
            return HttpResponse("failiure")
    else:
        return render(request, "services/generateKey.html")

def upload_and_sign(request):
    return render(request, "services/uploadAndSign.html")

def verify_and_show(request):
    return render(request, "services/verifyAndShow.html")

def request_ela(request):
    return render(request, "services/requestELA.html")

def vote_supernodes(request):
    return render(request, "services/voteSupernodes.html")

def run_contract(request):
    return render(request, "services/runContract.html")

def save_did_data(request):
    return render(request, "services/saveData.html")

def retrieve_did_data(request):
    return render(request, "services/retrieveData.html")

def documentation(request):
    return render(request, "services/documentation.html")

def about_us(request):
    return render(request, "services/aboutUs.html")