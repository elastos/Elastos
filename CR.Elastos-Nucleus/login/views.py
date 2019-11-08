from django.http import HttpResponse
from django.shortcuts import render


def landing_page(request):
    return render(request, 'Landing Pages/landingPage.html')


def sign_in(request):
    return HttpResponse("Hello, world. You're at the Sign In Page.")


def sign_up(request):
    return HttpResponse("Hello, world. You're at the Sign Up Page.")


def confirmation(request):
    return HttpResponse("Hello, world. You're at the Confirmation Page.")
