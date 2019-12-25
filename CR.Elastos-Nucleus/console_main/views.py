from django.contrib import messages
from django.contrib.auth import login
from django.shortcuts import render, redirect
from django.urls import reverse

from decouple import config

from login.models import DIDUser
from service.models import UserAPIKeys


def login_required(function):
    def wrapper(request, *args, **kw):
        if not request.session.get('logged_in'):
            return redirect(reverse('landing'))
        return function(request, *args, **kw)

    wrapper.__doc__ = function.__doc__
    wrapper.__name__ = function.__name__
    return wrapper


def landing(request):
    development = config('DEVELOPMENT', default=False, cast=bool)
    context = {'development': development}
    if development:
        email = "test@nucleusconsole.com"
        try:
            user = DIDUser.objects.get(email=email)
        except(TypeError, ValueError, OverflowError, DIDUser.DoesNotExist):
            user = DIDUser()
            user.email = email
            user.name = "Test User"
            user.set_password("admin")
            user.did = "test"
            user.is_active = True
            user.is_staff = True
            user.is_superuser = True
            user.save()
        login(request, user, backend='django.contrib.auth.backends.ModelBackend')
        request.session['name'] = user.name
        request.session['email'] = user.email
        request.session['did'] = user.did
        request.session['logged_in'] = True
    return render(request, 'landing.html', context)


def populate_session_vars_from_database(request, did):
    api_key = ''
    if UserAPIKeys.objects.filter(did=did):
        obj = UserAPIKeys.objects.get(did=did)
        api_key = obj.api_key
    request.session['api_key'] = api_key
