import json

from random import randint
from datetime import datetime, timedelta

from django.http import HttpResponse
from django.shortcuts import render
from decouple import config
from django.utils.http import urlencode
from django.shortcuts import redirect

from rest_framework import status
from rest_framework.decorators import api_view
from rest_framework.response import Response

from .models import DIDUser, DIDRequest


@api_view(['GET'])
def check_ela_auth(request):
    state = request.session['elaState']
    if not state:
        return Response({'authenticated': False}, status=status.HTTP_404_NOT_FOUND)
    try:
        recently_created_time = datetime.now() - timedelta(minutes=1)
        did_request_query_result = DIDRequest.objects.get(state=state, created_at__gte=recently_created_time)
        data = json.loads(did_request_query_result.data)
        if not data["auth"]:
            return Response({'authenticated': False}, status=status.HTTP_404_NOT_FOUND)
        request.session['elaDidInfo'] = data
        
        did_user_query_result = DIDUser.objects.get(did=data["DID"])
        if not did_user_query_result:
            redirect_url = redirect('register')
            request.session['redirect_success'] = True
        else:
            redirect_url = redirect('home')
            request.session['logged_in'] = True
            request.session['name'] = data['Nickname']
            request.session['email'] = data['Email']
            request.session['did'] = data['DID']
    except Exception as e:
        return Response({'error': str(e)}, status=status.HTTP_404_NOT_FOUND)
    return Response({'redirect': redirect_url}, status=status.HTTP_200_OK)


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

    # Save token to the database didauth_requests
    token = {'state': random, 'data': {'auth': False}}
    DIDRequest.objects.create(state=token['state'], data=json.dumps(token['data']))
    # Purge old requests for housekeeping. If the time denoted by 'created_by'
    # is more than 2 minutes old, delete the row
    stale_time = datetime.now() - timedelta(minutes=2)
    DIDRequest.objects.filter(created_at__lte=stale_time).delete()

    request.session['elephant_url'] = elephant_url
    return render(request, 'login/sign_in.html')


def home(request):
    return render(request, 'login/home.html')


def logout(request):
    return HttpResponse("logout")
