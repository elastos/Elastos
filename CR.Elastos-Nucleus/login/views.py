import gc
import logging

import json
import csv
import math

from django.utils import timezone
from django.apps import apps
from django.conf import settings

from django.core.mail import EmailMessage
from decouple import config

from django.contrib import messages
from django.http import HttpResponse
from django.template.loader import render_to_string
from django.utils.http import urlsafe_base64_decode
from django.urls import reverse
from django.shortcuts import render, redirect
from django.utils.encoding import force_text

from console_main.views import login_required, populate_session_vars_from_database, track_page_visit, \
    get_recent_services, send_email
from console_main.models import TrackUserPageVisits

from .models import DIDUser
from .forms import DIDUserCreationForm, DIDUserChangeForm
from service.forms import SuggestServiceForm
from .tokens import account_activation_token
from .your_activity_dict import get_activity_model, display_time_elapsed
from service.models import UserServiceSessionVars, SavedFileInformation


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
            messages.success(
                request, "Please check your email to complete your registration")
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
                messages.success(
                    request, "Please check your email to finish modifying your profile info")
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


@login_required
def feed(request):
    suggest_form = SuggestServiceForm()
    did = request.session['did']
    recent_services = get_recent_services(did)
    recent_pages = TrackUserPageVisits.objects.filter(did=did , activity_completed=False).order_by('-last_visited')[:9]
    most_visited_pages = TrackUserPageVisits.objects.filter(did=did).order_by('-number_visits')[:5]
    activity_pages = TrackUserPageVisits.objects.filter(did = did , activity_completed=True).order_by('-last_visited')[:9]
    your_activity_list = []
    all_apps = settings.ALL_APPS
    for items in activity_pages:
        model_found = False
        view_name = items.view.split(':')[1]  # get the view name
        your_activity_model = get_activity_model(view_name)
        time_elapsed_in_seconds = math.floor((timezone.now() - items.last_visited).seconds)
        last_visited = "{0} ago".format(display_time_elapsed(time_elapsed_in_seconds))
        if your_activity_model is None:
            your_activity_list.append({
                'display_string': 'You just visited "{0}" page'.format(items.name), 'last_visited': last_visited
            })
        else:
            for app in all_apps:
                app_models = apps.get_app_config(app).get_models()
                for model in app_models:
                    try:
                        if model.__name__ == your_activity_model:
                            obj_model = model.objects.filter(did=did).last()
                            if items.additional_field != '':
                                try:
                                    obj_dict = obj_model.your_activity()[view_name][items.additional_field]
                                    obj_dict['last_visited'] = last_visited
                                    your_activity_list.append(obj_dict)
                                    model_found = True
                                    break
                                except KeyError as e:
                                    your_activity_list.append({
                                        'display_string': 'You just visited "{0}" page'.format(items.name), 'last_visited': last_visited
                                    })
                                    logging.debug(e)
                                    break
                            else:
                                obj_dict = obj_model.your_activity()[view_name]
                                obj_dict['last_visited'] = last_visited
                                your_activity_list.append(obj_dict)
                                model_found = True
                                break
                    except Exception as e:
                        your_activity_list.append({
                            'display_string': 'You just visited "{0}" page'.format(items.name), 'last_visited': last_visited
                        })
                if model_found:
                    break
    return render(request, 'login/feed.html', {'recent_pages': recent_pages, 'recent_services': recent_services,
                                               'most_visited_pages': most_visited_pages, 'suggest_form': suggest_form,
                                               'your_activity': your_activity_list})


@login_required
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
def suggest_service(request):
    did = request.session['did']
    if request.is_ajax():
        category = request.POST.get('category')
        title = request.POST.get('title')
        description = request.POST.get('description')
        reasoning = request.POST.get('reasoning')
        content = render_to_string('service/suggest_service_email.html', {
            'service_category': category,
            'service_name': title,
            'service_description': description,
            'service_reasoning': reasoning
        })
        email = EmailMessage(subject="Suggested Service",
                             body=content,
                             from_email='"Nucleus Console Support Team" <support@nucleusconsole.com>',
                             to=['support@nucleusconsole.com'])
        email.content_subtype = 'html'
        try:
            email.send()
            messages.success(request, "Service suggestion was submitted")
        except Exception as e:
            logging.debug(f"did={did} Method: suggest_service Error: {e}")
            messages.success(
                request, "Service suggestion could not be submitted at this time. Please try again")
        finally:
            return redirect(reverse('login:feed'))


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
                # ahead to check if there's any entry with
                model.objects.filter(did=request.session['did'])
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
