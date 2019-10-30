from django.shortcuts import render , redirect
import json
from django.http import HttpResponse
import requests

def generate_key(request):
    if(request.method == 'GET'):
        r = requests.get("http://localhost:8888/api/1/common/generateAPIKey" , )
        if(r.status_code == 200):
            data = r.json()["API Key"]
            return HttpResponse(data)
        else:
            return HttpResponse("failiure")
    else:
        return HttpResponse("It was a post request")


def upload(request):
    if(request.method == 'GET'):
        return HttpResponse("Success")
        
        







