from django.shortcuts import render , redirect
import json
from django.http import HttpResponse
import requests

def generate_key(request):
    if(request.method == 'GET'):
        r = requests.get("http://localhost:8888/api/1/common/generateAPIKey")
        if(r.status_code == 200):
            API_KEY = r.json()["API Key"]
            context = {
                'API_KEY':API_KEY,
            }
            print(context)
            return render(request , "HelloWorld/generateKey.html",context)
        else:
            return HttpResponse("failiure")
    else:
        return render(request , "HelloWorld/generateKey.html")


def file_upload(request):
    if(request.method == 'POST'):
        headers = {'api_key' : 'KHBOsth7b3WbOTVzZqGUEhOY8rPreYFM' }
        hash = requests.get("http://localhost:8888/api/1/console/upload" , headers = headers)
        return render(request , "HelloWorld/fileUpload.html")
    else:
        return render(request , "HelloWorld/fileUpload.html")

        







