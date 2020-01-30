import json
import gc
import logging
import secrets
import time

import csv
import urllib

from django.apps import apps
from django.conf import settings

from datetime import timedelta, datetime

from django.contrib.auth import login
from django.utils import timezone
from fastecdsa.encoding.sec1 import SEC1Encoder
from fastecdsa import ecdsa, curve
from binascii import unhexlify
from decouple import config

from django.contrib import messages
from django.contrib.sites.shortcuts import get_current_site
from django.http import HttpResponse
from django.http import JsonResponse
from django.template.loader import render_to_string
from django.utils.http import urlsafe_base64_decode
from django.views.decorators.csrf import csrf_exempt
from django.urls import reverse
from django.shortcuts import render, redirect
from django.utils.http import urlsafe_base64_encode
from django.core.mail import EmailMessage
from django.utils.encoding import force_bytes, force_text

from console_main.views import login_required, populate_session_vars_from_database, track_page_visit, \
    get_recent_services
from console_main.models import TrackUserPageVisits

from .models import DIDUser, DIDRequest
from .forms import DIDUserCreationForm, DIDUserChangeForm
from service.forms import SuggestServiceForm
from .tokens import account_activation_token


def check_ela_auth(request):
    if 'elaState' not in request.session.keys():
        return JsonResponse({'authenticated': False}, status=403)
    state = request.session['elaState']
    try:
        recently_created_time = timezone.now() - timedelta(minutes=1)
        did_request_query_result = DIDRequest.objects.get(state=state, created_at__gte=recently_created_time)
        data = json.loads(did_request_query_result.data)
        if not data["auth"]:
            return JsonResponse({'authenticated': False}, status=403)
        request.session['name'] = data['Nickname']
        request.session['email'] = data['Email']
        request.session['did'] = data['DID']
        if DIDUser.objects.filter(did=data["DID"]).exists() is False:
            redirect_url = "/login/register"
            request.session['redirect_success'] = True
        else:
            user = DIDUser.objects.get(did=data["DID"])
            request.session['name'] = user.name
            request.session['email'] = user.email
            request.session['did'] = user.did
            if user.is_active is False:
                redirect_url = "/"
                send_email(request, user.email, user)
                messages.success(request,
                                 "The email '%s' needs to be verified. Please check your email for confirmation link" % user.email)
            else:
                redirect_url = "/login/feed"
                login(request, user, backend='django.contrib.auth.backends.ModelBackend')
                request.session['logged_in'] = True
                populate_session_vars_from_database(request, request.session['did'])
                messages.success(request, "Logged in successfully!")
    except Exception as e:
        return JsonResponse({'error': str(e)}, status=400)
    return JsonResponse({'redirect': redirect_url}, status=200)


@csrf_exempt
def did_callback(request):
    if request.method == 'POST':
        response = json.loads(request.body)
        if 'Data' not in response.keys():
            return HttpResponse(status=400)
        data = json.loads(response['Data'])
        sig = response['Sign']
        client_public_key = data['PublicKey']

        r, s = int(sig[:64], 16), int(sig[64:], 16)
        public_key = SEC1Encoder.decode_public_key(unhexlify(client_public_key), curve.P256)
        valid = ecdsa.verify((r, s), response['Data'], public_key)
        if not valid:
            return JsonResponse({'message': 'Unauthorized'}, status=401)
        try:
            recently_created_time = timezone.now() - timedelta(minutes=1)
            did_request_query_result = DIDRequest.objects.get(state=data["RandomNumber"],
                                                              created_at__gte=recently_created_time)
            if not did_request_query_result:
                return JsonResponse({'message': 'Unauthorized'}, status=401)
            data["auth"] = True
            DIDRequest.objects.filter(state=data["RandomNumber"]).update(data=json.dumps(data))
        except Exception as e:
            logging.debug(f" Method: did_callback Error: {e}")
            JsonResponse({'error': str(e)}, status=404)

    return JsonResponse({'result': True}, status=200)


# TODO: DOES NOT WORK YET
@csrf_exempt
def did_callback_elastos(request):
    if request.method == 'POST':
        response = urllib.parse.unquote(request.body.decode())
        if 'result=' not in response:
            return JsonResponse({'error': 'Could not parse response'}, status=400)

        result = response.replace('result=', '')
        data = json.loads(result)

        did = data['did']
        data['DID'] = did
        credentials = data['presentation']['verifiableCredential']
        for cred in credentials:
            if did + "#name" == cred['credentialId']:
                data['Nickname'] = cred['credentialSubject']['name']
            elif did + "#email" == cred['credentialId']:
                data['Email'] = cred['credentialSubject']['email']
            exp_time = urllib.parse.unquote(cred['expirationDate'])
            data['RandomNumber'] = int(time.mktime(datetime.strptime(exp_time, "%Y-%m-%dT%H:%M:%S.000Z").timetuple()))

        try:
            recently_created_time = timezone.now() - timedelta(minutes=1)
            did_request_query_result = DIDRequest.objects.get(state=data["RandomNumber"],
                                                              created_at__gte=recently_created_time)
            if not did_request_query_result:
                return JsonResponse({'message': 'Unauthorized'}, status=401)
            data["auth"] = True
            DIDRequest.objects.filter(state=data["RandomNumber"]).update(data=json.dumps(data))
        except Exception as e:
            logging.debug(f"Method: did_callback Error: {e}")
            JsonResponse({'error': str(e)}, status=404)

    return JsonResponse({'result': True}, status=200)


def register(request):
    if 'redirect_success' not in request.session.keys():
        return redirect(reverse('landing'))
    if request.method == 'POST':
        form = DIDUserCreationForm(request.POST,
                                   initial={'name': request.session['name'], 'email': request.session['email'],
                                            'did': request.session['did']})
        if form.is_valid():
            user = form.save(commit=False)
            user.is_active = False
            user.save()
            to_email = form.cleaned_data.get('email')
            send_email(request, to_email, user)
            request.session['name'] = user.name
            request.session['email'] = user.email
            messages.success(request, "Please check your email to complete your registration")
            return redirect(reverse('landing'))
    else:
        form = DIDUserCreationForm(initial={'name': request.session['name'], 'email': request.session['email'],
                                            'did': request.session['did']})
    return render(request, 'login/register.html', {'form': form})


@login_required
def edit_profile(request):
    did = request.session['did']
    track_page_visit(did, 'Edit Profile', 'login:edit_profile', False)
    recent_services = get_recent_services(did)
    did_user = DIDUser.objects.get(did=request.session['did'])
    if request.method == 'POST':
        form = DIDUserChangeForm(request.POST, instance=request.user)

        if form.is_valid():

            user = form.save(commit=False)
            # This means the user changed their email address
            if user.email != did_user.email:
                user.is_active = False
                user.save()
                to_email = form.cleaned_data.get('email')
                send_email(request, to_email, user)
                messages.success(request, "Please check your email to finish modifying your profile info")
            else:
                user.save()
            return redirect(reverse('login:feed'))
    else:
        if did_user.is_active is False:
            send_email(request, did_user.email, did_user)
            messages.success(request,
                             "The email '%s' needs to be verified. Please check your email for confirmation link" % did_user.email)
            return redirect(reverse('login:feed'))
        form = DIDUserChangeForm(instance=request.user)
    return render(request, 'login/edit_profile.html', {'form': form, 'recent_services': recent_services})


def send_email(request, to_email, user):
    current_site = get_current_site(request)
    mail_subject = 'Activate your Nucleus Console account'
    message = render_to_string('login/account_activation_email.html', {
        'user': user,
        'domain': current_site.domain,
        'uid': urlsafe_base64_encode(force_bytes(user.did)),
        'token': account_activation_token.make_token(user),
    })
    email = EmailMessage(
        mail_subject, message, from_email='"Nucleus Console Support Team" <support@nucleusconsole.com>', to=[to_email]
    )
    email.content_subtype = 'html'
    try:
        email.send()
        return HttpResponse("Success")
    except Exception as e:
        logging.debug(f"Method: send_email Error: {e}")
        return HttpResponse("Failure")


def activate(request, uidb64, token):
    try:
        uid = force_text(urlsafe_base64_decode(uidb64))
        user = DIDUser.objects.get(did=uid)
    except(TypeError, ValueError, OverflowError, DIDUser.DoesNotExist):
        user = None
    if user is not None and account_activation_token.check_token(user, token):
        user.is_active = True
        user.save()
        populate_session_vars_from_database(request, uid)
        request.session['logged_in'] = True
        messages.success(request, "Email has been confirmed!")
        return redirect(reverse('login:feed'))
    else:
        return HttpResponse('Activation link is invalid!')


def sign_in(request):
    random = secrets.randbelow(999999999999)
    request.session['elaState'] = random

    elephant_url = get_elaphant_sign_in_url(request, random)
    elastos_url = get_elastos_sign_in_url(request, random)

    request.session['elephant_url'] = elephant_url
    request.session['elastos_url'] = elastos_url

    # Purge old requests for housekeeping. If the time denoted by 'created_by'
    # is more than 2 minutes old, delete the row
    stale_time = timezone.now() - timedelta(minutes=2)
    DIDRequest.objects.filter(created_at__lte=stale_time).delete()

    # Save token to the database didauth_requests
    token = {'state': random, 'data': {'auth': False}}
    DIDRequest.objects.create(state=token['state'], data=json.dumps(token['data']))

    return render(request, 'login/sign_in.html')


# TODO: DOES NOT WORK YET
def get_elastos_sign_in_url(request, random):
    exp = int(round(time.time() + 300))

    url_params = {
        'appid': random,
        'Iss': 'did:elastos:iWsK3X8aBpLqFVXAd2KUpMpFGJYEYyfjUi#primary',
        'exp': exp,
        'callbackurl': config('DIDLOGIN_APP_URL') + '/login/did_callback_elastos',
        'claims': {
            'name': True,
            'email': True
        }
    }

    url = 'elastos://credaccess?' + urllib.parse.urlencode(url_params)

    return url


def get_elaphant_sign_in_url(request, random):
    did = config('DIDLOGIN_ELAPHANT_DID')
    app_id = config('DIDLOGIN_ELAPHANT_APP_ID')
    public_key = config('DIDLOGIN_ELAPHANT_PUBLIC_KEY')
    app_name = config('DIDLOGIN_ELAPHANT_APP_NAME')

    url_params = {
        'CallbackUrl': config('DIDLOGIN_APP_URL') + '/login/did_callback',
        'Description': 'Elastos DID Authentication',
        'AppID': app_id,
        'PublicKey': public_key,
        'DID': did,
        'RandomNumber': random,
        'AppName': app_name,
        'RequestInfo': 'Nickname,Email'
    }

    url = 'elaphant://identity?' + urllib.parse.urlencode(url_params)

    return url


@login_required
def feed(request):
    suggest_form = SuggestServiceForm()
    did = request.session['did']
    recent_services = get_recent_services(did)
    recent_pages = TrackUserPageVisits.objects.filter(did=did).order_by('-last_visited')[:5]
    most_visited_pages = TrackUserPageVisits.objects.filter(did=did).order_by('-number_visits')[:5]

    return render(request, 'login/feed.html', {'recent_pages': recent_pages, 'recent_services': recent_services,
                                               'most_visited_pages': most_visited_pages, 'suggest_form': suggest_form})


def sign_out(request):
    request.session.clear()
    gc.collect()
    did_login = config('DIDLOGIN', default=False, cast=bool)
    if not did_login:
        messages.success(request, "You have disabled DID LOGIN. Unable to log out! Please re-run the server with "
                                  "DIDLOGIN set to True")
    else:
        messages.success(request, "You have been logged out!")
    return redirect(reverse('landing'))


@login_required
def get_user_data(request):
    exempt_fields = ['password']
    response = HttpResponse(content_type='text/csv')
    response['Content-Disposition'] = 'attachment; filename="user_data.csv"'
    writer = csv.writer(response)
    all_apps = settings.ALL_APPS
    for items in all_apps:
        app_models = apps.get_app_config(items).get_models()
        for model in app_models:
            try:
                model.objects.filter(did=request.session['did'])  # ahead to check if there's any entry with
                # the given did
                writer.writerow([model.user_name()])
                fields = [f.name for f in model._meta.get_fields()]
                writer.writerow(fields)
                user_objects = model.objects.filter(did=request.session['did'])
                for obj in user_objects:
                    l = []
                    for field in model._meta.get_fields():
                        val = str(field.value_from_object(obj))
                        if val not in exempt_fields:
                            l.append(val)
                        else:
                            l.append('N/A')
                    writer.writerow(l)
                writer.writerow([])
            except Exception as e:
                continue

    exempt_cookies = ['_auth_user_id', '_auth_user_backend', '_auth_user_hash']
    writer.writerow(['Tracked Cookies'])
    writer.writerow(['tracked info', ':', 'tracked value'])
    for key, value in request.session.items():
        if key not in exempt_cookies:
            writer.writerow([key, ':', value])

    return response


@login_required
def remove_user_data(request):
    all_apps = settings.ALL_APPS
    for items in all_apps:
        app_models = apps.get_app_config(items).get_models()
        for model in app_models:
            try:
                model.objects.filter(did=request.session['did']).delete()
            except Exception as e:
                continue

    request.session.clear()
    messages.success(request, "You have been logged out!")
    return redirect(reverse('landing'))
