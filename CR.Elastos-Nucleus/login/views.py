import json

from random import randint
from datetime import datetime, timedelta

from django.contrib.auth.decorators import login_required
from django.http import HttpResponse
from django.http import JsonResponse
from django.utils.http import urlencode
from django.views.decorators.csrf import csrf_exempt

from django.urls import reverse
from django.shortcuts import render, redirect
from .forms import DIDUserCreationForm

from fastecdsa.encoding.sec1 import SEC1Encoder
from fastecdsa import ecdsa, curve
from binascii import unhexlify

from decouple import config

from .models import DIDUser, DIDRequest


def check_ela_auth(request):
    state = request.session['elaState']
    if not state:
        return JsonResponse({'authenticated': False}, status=404)
    try:
        recently_created_time = datetime.now() - timedelta(minutes=1)
        did_request_query_result = DIDRequest.objects.get(state=state, created_at__gte=recently_created_time)
        data = json.loads(did_request_query_result.data)
        if not data["auth"]:
            return JsonResponse({'authenticated': False}, status=404)

        request.session['name'] = data['Nickname']
        request.session['email'] = data['Email']
        request.session['did'] = data['DID']
        if DIDUser.objects.filter(did=data["DID"]).exists() is False:
            redirect_url = "/login/register"
            request.session['redirect_success'] = True
        else:
            redirect_url = "/login/home"
            request.session['logged_in'] = True
    except Exception as e:
        return JsonResponse({'error': str(e)}, status=404)
    return JsonResponse({'redirect': redirect_url}, status=200)


@csrf_exempt
def did_callback(request):
    if request.method == 'POST':
        response = json.loads(request.body)
        if request.content_type == "application/json" or 'Data' not in response.keys():
            HttpResponse(status=400)
        data = json.loads(response['Data'])
        sig = response['Sign']
        client_public_key = data['PublicKey']

        r, s = int(sig[:64], 16), int(sig[64:], 16)
        public_key = SEC1Encoder.decode_public_key(unhexlify(client_public_key), curve.P256)
        valid = ecdsa.verify((r, s), response['Data'], public_key)
        if not valid:
            return JsonResponse({'message': 'Unauthorized'}, status=401)
        try:
            recently_created_time = datetime.now() - timedelta(minutes=1)
            did_request_query_result = DIDRequest.objects.get(state=data["RandomNumber"],
                                                              created_at__gte=recently_created_time)
            if not did_request_query_result:
                return JsonResponse({'message': 'Unauthorized'}, status=401)
            data["auth"] = True
            DIDRequest.objects.filter(state=data["RandomNumber"]).update(data=json.dumps(data))
        except Exception as e:
            JsonResponse({'error': str(e)}, status=404)

    return JsonResponse({'result': True}, status=200)


def register(request):
    if 'redirect_success' not in request.session.keys():
        return redirect(reverse('index'))
    if request.method == 'POST':
        form = DIDUserCreationForm(request.POST,
                                   initial={'name': request.session['name'], 'email': request.session['email'],
                                            'did': request.session['did']})
        if form.is_valid():
            form.save()
            return redirect(reverse('login:home'))
    else:
        form = DIDUserCreationForm(initial={'name': request.session['name'], 'email': request.session['email'],
                                            'did': request.session['did']})
    return render(request, 'login/register.html', {'form': form})


def sign_in(request):
    public_key = config('ELA_PUBLIC_KEY')
    did = config('ELA_DID')
    app_id = config('ELA_APP_ID')
    app_name = config('ELA_APP_NAME')

    random = randint(10000000000, 999999999999)
    request.session['elaState'] = random

    url_params = {
        'CallbackUrl': config('APP_URL') + '/login/did_callback',
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


@login_required
def home(request):
    return render(request, 'login/home.html')


def logout(request):
    return HttpResponse("logout")
