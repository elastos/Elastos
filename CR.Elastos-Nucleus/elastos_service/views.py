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
            return render(request, "HelloWorld/generateKey.html", context)
        else:
            return HttpResponse("failiure")
    else:
        return render(request, "HelloWorld/generateKey.html")


def file_upload(request):
    if(request.method == 'POST'):
        headers = {'api_key': 'KHBOsth7b3WbOTVzZqGUEhOY8rPreYFM'}
        hash = requests.get(
            "http://localhost:8888/api/1/console/upload", headers=headers)
        return render(request, "HelloWorld/fileUpload.html")
    else:
        return render(request, "HelloWorld/fileUpload.html")


def show_file_content(request):
    return HttpResponse("Hello, world. You're at the Show File Content.")


def upload_and_scan(request):
    return HttpResponse("Hello, world. You're at the Upload and Scan.")


def verify_and_show_content(request):
    return HttpResponse("Hello, world. You're at the Verify and Show Content.")


def scan_message(request):
    return HttpResponse("Hello, world. You're at the Scan Message.")


def verify_message(request):
    return HttpResponse("Hello, world. You're at the Verify Message.")


def transfer_file(request):
    return HttpResponse("Hello, world. You're at the Transfer File.")


def documentation(request):
    return HttpResponse("Hello, world. You're at the Documentation.")


def about_us(request):
    return HttpResponse("Hello, world. You're at the About Us.")
