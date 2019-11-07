from django.http import HttpResponse


def landing_page(request):
    return HttpResponse("Hello, world. You're at the Landing Page.")


def sign_in(request):
    return HttpResponse("Hello, world. You're at the Sign In Page.")


def sign_up(request):
    return HttpResponse("Hello, world. You're at the Sign Up Page.")


def confirmation(request):
    return HttpResponse("Hello, world. You're at the Confirmation Page.")
