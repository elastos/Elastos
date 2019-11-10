from django.shortcuts import render, redirect
import json
from django.http import HttpResponse
import requests


def generate_key(request):
    if(request.method == 'GET'):
        r = requests.get("http://localhost:8888/api/1/common/generateAPIKey")
        if(r.status_code == 200):
            API_KEY = r.json()["API Key"]
            context = {
                'API_KEY': API_KEY,
            }
            print(context)
            return render(request, "Services/generateKey.html", context)
        else:
            return HttpResponse("failiure")
    else:
        return render(request, "Services/generateKey.html")


def file_upload(request):
    if(request.method == 'POST'):
        headers = {'api_key': 'KHBOsth7b3WbOTVzZqGUEhOY8rPreYFM'}
        hash = requests.get(
            "http://localhost:8888/api/1/console/upload", headers=headers)
        return render(request, "Services/fileUpload.html")
    else:
        return render(request, "Services/fileUpload.html")


def show_file_content(request):
    return render(request, "Services/showFileContent.html")


def upload_and_scan(request):
    return render(request, "Services/uploadAndScan.html")


def verify_and_show_content(request):
    return render(request, "Services/verifyAndShowContent.html")


def scan_message(request):
    return render(request, "Services/scanMessage.html")


def verify_message(request):
    return render(request, "Services/verifyMessage.html")


def transfer_file(request):
    return render(request, "Services/transferFile.html")


def documentation(request):
    return render(request, "Services/documentation.html")


def about_us(request):
    return render(request, "Services/aboutUs.html")
