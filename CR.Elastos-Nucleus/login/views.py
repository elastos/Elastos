from random import randint

from django.http import HttpResponse
from django.shortcuts import render
from decouple import config
from django.utils.http import urlencode


def check_ela_auth(request):
    return HttpResponse("check_ela_auth")


def did_callback(request):
    return HttpResponse("did_callback")


def register(request):
    return render(request, 'login/register.html')


def sign_in(request):
    public_key = config('ELA_PUBLIC_KEY')
    did = config('ELA_DID')
    app_id = config('ELA_APP_ID')
    app_name = config('ELA_APP_NAME')

    random = randint(10000000000, 999999999999)
    request.session['elaState'] = random

    url_params = {
        'CallbackUrl': config('APP_URL') + 'login/did_callback',
        'Description': 'Elastos DID Authentication',
        'AppID': app_id,
        'PublicKey': public_key,
        'DID': did,
        'RandomNumber': random,
        'AppName': app_name,
        'RequestInfo': 'Nickname,Email'
    }

    elephant_url = 'elaphant://identity?' + urlencode(url_params)

    # TODO: Save token to the database didauth_requests

    request.session['elephant_url'] = elephant_url
    return render(request, 'login/sign_in.html')


def home(request):
    return render(request, 'login/home.html')


def logout(request):
    return HttpResponse("logout")
