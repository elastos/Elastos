from django.shortcuts import render, redirect
from django.urls import reverse


def login_required(function):
    def wrapper(request, *args, **kw):
        if not request.session.get('logged_in'):
            return redirect(reverse('landing'))
        return function(request, *args, **kw)

    wrapper.__doc__ = function.__doc__
    wrapper.__name__ = function.__name__
    return wrapper


def landing(request):
    context = {"logged_in": False}
    return render(request, 'landing.html', context)