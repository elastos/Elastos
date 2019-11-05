from django.shortcuts import render , redirect
import json
from django.shortcuts import render
from django.conf import settings
from django.core.files.storage import FileSystemStorage
from django.http import HttpResponse
import requests

def generate_key(request):
    if(request.method == 'GET'):
        r = requests.get("http://localhost:8888/api/1/common/generateAPIKey" , )
        if(r.status_code == 200):
            data = r.json()["API Key"]
            return render(request, 'generateKey.html')
        else:
            return HttpResponse("failiure")
    else:
        return HttpResponse("It was a post request")


def upload(request):
    if request.method == 'POST' and request.FILES['myfile']:
        myfile = request.FILES['myfile']
        fs = FileSystemStorage()
        filename = fs.save(myfile.name, myfile)
        uploaded_file_url = fs.url(filename)
        return render(request, 'fileUpload.html', {
            'uploaded_file_url': uploaded_file_url
        })
    return render(request, 'fileUpload.html')
        
        







