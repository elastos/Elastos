from django.shortcuts import render


def landing(request):
    context = {"name": "landing"}
    return render(request, 'landing.html', context)