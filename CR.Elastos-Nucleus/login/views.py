from django.http import HttpResponse
from django.shortcuts import render


def check_ela_auth(request):
    return HttpResponse("check_ela_auth")


def did_callback(request):
    return HttpResponse("did_callback")


def register(request):
    return render(request, 'login/register.html')


def login(request):
    return render(request, 'login/login.html')


def home(request):
    return render(request, 'login/home.html')


def logout(request):
    return HttpResponse("logout")
