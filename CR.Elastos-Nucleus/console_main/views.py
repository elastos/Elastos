import logging

from django.contrib.auth import login
from django.db.models import F
from django.shortcuts import render, redirect
from django.urls import reverse

from decouple import config
from django.utils import timezone
from django.db import models

from .models import TrackUserPageVisits
from login.models import DIDUser
from service.models import UserServiceSessionVars


def login_required(function):
    def wrapper(request, *args, **kw):
        if not request.session.get('logged_in'):
            return redirect(reverse('landing'))
        return function(request, *args, **kw)

    wrapper.__doc__ = function.__doc__
    wrapper.__name__ = function.__name__
    return wrapper


def landing(request):
    did_login = config('DIDLOGIN', default=False, cast=bool)
    recent_services = None
    if not did_login:
        email = config('SUPERUSER_USER')
        try:
            user = DIDUser.objects.get(email=email)
        except(TypeError, ValueError, OverflowError, DIDUser.DoesNotExist):
            user = DIDUser()
            user.email = email
        user.name = "Test User"
        user.set_password(config('SUPERUSER_PASSWORD'))
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
        populate_session_vars_from_database(request, user.did)
        recent_services = get_recent_services(user.did)
    return render(request, 'landing.html', {'recent_services': recent_services})


def populate_session_vars_from_database(request, did):
    api_key = ''

    mnemonic_mainchain = ''
    private_key_mainchain = ''
    public_key_mainchain = ''
    address_mainchain = ''

    private_key_did = ''
    public_key_did = ''
    address_did = ''
    did_did = ''

    address_eth = ''
    private_key_eth = ''
    if UserServiceSessionVars.objects.filter(did=did):
        obj = UserServiceSessionVars.objects.get(did=did)
        api_key = obj.api_key

        mnemonic_mainchain = obj.mnemonic_mainchain
        private_key_mainchain = obj.private_key_mainchain
        public_key_mainchain = obj.public_key_mainchain
        address_mainchain = obj.address_mainchain

        private_key_did = obj.private_key_did
        public_key_did = obj.public_key_did
        address_did = obj.address_did
        did_did = obj.did_did

        address_eth = obj.address_eth
        private_key_eth = obj.private_key_eth
    request.session['api_key'] = api_key

    request.session['mnemonic_mainchain'] = mnemonic_mainchain
    request.session['private_key_mainchain'] = private_key_mainchain
    request.session['public_key_mainchain'] = public_key_mainchain
    request.session['address_mainchain'] = address_mainchain

    request.session['private_key_did'] = private_key_did
    request.session['public_key_did'] = public_key_did
    request.session['address_did'] = address_did
    request.session['did_did'] = did_did

    request.session['address_eth'] = address_eth
    request.session['private_key_eth'] = private_key_eth


def track_page_visit(did, name, view, is_service):
    try:
        track_obj = TrackUserPageVisits.objects.get(did=did, name=name, view=view, is_service=is_service)
        track_obj.name = name
        track_obj.view = view
        track_obj.last_visited = timezone.now()
        track_obj.number_visits = F('number_visits') + 1
        track_obj.is_service = is_service
        track_obj.save()
    except models.ObjectDoesNotExist:
        track_obj = TrackUserPageVisits.objects.create(did=did, name=name, view=view, number_visits=1, is_service=is_service)
        track_obj.save()
    except Exception as e:
        logging.debug(e)


def get_recent_services(did):
    recent_services = TrackUserPageVisits.objects.filter(did=did, is_service=True).order_by('-last_visited')[:5]
    return recent_services











