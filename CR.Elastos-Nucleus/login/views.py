import json
import gc
import secrets

from datetime import datetime, timedelta

from django.contrib.auth import login
from fastecdsa.encoding.sec1 import SEC1Encoder
from fastecdsa import ecdsa, curve
from binascii import unhexlify
from decouple import config

from django.contrib import messages
from django.contrib.sites.shortcuts import get_current_site
from django.http import HttpResponse
from django.http import JsonResponse
from django.template.loader import render_to_string
from django.utils import timezone
from django.utils.http import urlencode, urlsafe_base64_decode
from django.views.decorators.csrf import csrf_exempt
from django.urls import reverse
from django.shortcuts import render, redirect
from django.utils.http import urlsafe_base64_encode
from django.core.mail import EmailMessage
from django.utils.encoding import force_bytes, force_text

from .models import DIDUser, DIDRequest
from .forms import DIDUserCreationForm, DIDUserChangeForm
from .tokens import account_activation_token


def login_required(function):
    def wrapper(request, *args, **kw):
        if not request.session.get('logged_in'):
            return redirect(reverse('landing'))
        return function(request, *args, **kw)

    wrapper.__doc__ = function.__doc__
    wrapper.__name__ = function.__name__
    return wrapper


def check_ela_auth(request):
    if 'elaState' not in request.session.keys():
        return JsonResponse({'authenticated': False}, status=403)
    state = request.session['elaState']
    try:
        recently_created_time = datetime.now() - timedelta(minutes=1)
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
            if user.is_active is False:
                redirect_url = "/"
                send_email(request, user.email, user)
                messages.success(request,
                                 "The email '%s' needs to be verified. Please check your email for confirmation link" % user.email)
            else:
                redirect_url = "/login/home"
                request.session['logged_in'] = True
                login(request, user, backend='django.contrib.auth.backends.ModelBackend')
                messages.success(request, "Logged in successfully!")
    except Exception as e:
        return JsonResponse({'error': str(e)}, status=400)
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
            user = form.save(commit=False)
            user.is_active = False
            user.save()
            to_email = form.cleaned_data.get('email')
            send_email(request, to_email, user)
            request.session['name'] = user.name
            request.session['email'] = user.email
            messages.success(request, "Please check your email to complete your registration")
            return redirect(reverse('index'))
    else:
        form = DIDUserCreationForm(initial={'name': request.session['name'], 'email': request.session['email'],
                                            'did': request.session['did']})
    return render(request, 'login/register.html', {'form': form})


@login_required
def edit_profile(request):
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
            return redirect(reverse('login:home'))
    else:
        if did_user.is_active is False:
            send_email(request, did_user.email, did_user)
            messages.success(request,
                             "The email '%s' needs to be verified. Please check your email for confirmation link" % did_user.email)
            return redirect(reverse('login:home'))
        form = DIDUserChangeForm(instance=request.user)
    return render(request, 'login/edit_profile.html', {'form': form})


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
        mail_subject, message, to=[to_email]
    )
    email.content_subtype = 'html'
    email.send()


def activate(request, uidb64, token):
    try:
        uid = force_text(urlsafe_base64_decode(uidb64))
        user = DIDUser.objects.get(did=uid)
    except(TypeError, ValueError, OverflowError, DIDUser.DoesNotExist):
        user = None
    if user is not None and account_activation_token.check_token(user, token):
        user.is_active = True
        user.save()
        request.session['logged_in'] = True
        messages.success(request, "Email has been confirmed!")
        return redirect(reverse('login:home'))
    else:
        return HttpResponse('Activation link is invalid!')


def sign_in(request):
    public_key = config('ELA_PUBLIC_KEY')
    did = config('ELA_DID')
    app_id = config('ELA_APP_ID')
    app_name = config('ELA_APP_NAME')

    random = secrets.randbelow(999999999999)
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


def sign_out(request):
    request.session.clear()
    gc.collect()
    messages.success(request, "You have been logged out!")
    return redirect(reverse('landing'))