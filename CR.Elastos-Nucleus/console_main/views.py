from django.shortcuts import render


def landing(request):
    context = {"logged_in": False}
    return render(request, 'landing.html', context)