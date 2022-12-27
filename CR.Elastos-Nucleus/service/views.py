import json
import logging
import os

from django.core import serializers
from django.http import HttpResponse

from django.core.files.base import ContentFile, File
from django.shortcuts import render, redirect
from django.utils.crypto import get_random_string
from elastos_adenine import NodeRpc

from console_main.settings import MEDIA_ROOT, GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION, SHARED_SECRET_ADENINE
from console_main.views import login_required, populate_session_vars_from_database, track_page_visit, \
    get_recent_services
from django.contrib import messages
from django.urls import reverse

from elastos_adenine.common import Common
from elastos_adenine.hive import Hive
from elastos_adenine.sidechain_eth import SidechainEth
from elastos_adenine.wallet import Wallet

from .forms import GenerateAPIKeyForm
from .forms import UploadAndSignForm, VerifyAndShowForm
from .forms import CreateWalletForm, ViewWalletForm, RequestELAForm
from .forms import DeployETHContractForm, WatchETHContractForm

from .models import UploadFile, UserServiceSessionVars, SavedFileInformation, SavedETHContractInformation


@login_required
def generate_key(request):
    did = request.session['did']
    track_page_visit(did, "Generate API Key", "service:generate_key", True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/generate_key.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/generate_key.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    if request.method == 'POST':
        form = GenerateAPIKeyForm(request.POST, initial={'did': did})
        if form.is_valid():
            try:
                common = Common(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                error_message = None
                output = {}
                if 'submit_get_api_key' in request.POST:
                    response = common.get_api_key_request(SHARED_SECRET_ADENINE, did)
                    if response['status']:
                        data = json.loads(response['output'])
                        api_key = data['result']['api_key']
                        obj, created = UserServiceSessionVars.objects.update_or_create(did=did,
                                                                                       defaults={'did': did,
                                                                                                 'api_key': api_key})
                        obj.save()
                        populate_session_vars_from_database(request, did)
                        output['get_api_key'] = True
                        token = 'get'
                    else:
                        error_message = response['status_message']
                elif 'submit_generate_api_key' in request.POST:
                    response = common.generate_api_request(SHARED_SECRET_ADENINE, did)
                    if response['status']:
                        data = json.loads(response['output'])
                        api_key = data['result']['api_key']
                        obj, created = UserServiceSessionVars.objects.update_or_create(did=did,
                                                                                       defaults={'did': did,
                                                                                                 'api_key': api_key})
                        obj.save()
                        populate_session_vars_from_database(request, did)
                        output['generate_api_key'] = True
                        token = 'generate'
                    else:
                        error_message = response['status_message']
                else:
                    error_message = "Invalid form submission. Please refresh the page and try generating a new API " \
                                    "key again "
                if error_message:
                    messages.success(request, error_message)
                    return redirect(reverse('service:generate_key'))
                else:
                    request.session['api_key'] = api_key
                    track_page_visit(did, 'Generate API Key', "service:generate_key", True, True, token)
                    return render(request, "service/generate_key.html",
                                  {'output': output, 'api_key': api_key, 'sample_code': sample_code,
                                   'recent_services': recent_services})
            except Exception as e:
                logging.debug(f"did: {did} Method: generate_key Error: {e}")
                messages.success(request, "Could not generate an API key. Please try again")
                return redirect(reverse('service:generate_key'))
            finally:
                common.close()
    else:
        form = GenerateAPIKeyForm(initial={'did': did})
        return render(request, "service/generate_key.html",
                      {'form': form, 'sample_code': sample_code, 'recent_services': recent_services})


@login_required
def upload_and_sign(request):
    did = request.session['did']
    track_page_visit(did, 'Upload And Sign', 'service:upload_and_sign', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/upload_and_sign.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/upload_and_sign.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    if request.is_ajax():
        boolean = request.POST.get('delete')
        if boolean:
            SavedFileInformation.objects.filter(did=did).delete()
            return HttpResponse("Files have been deleted")
        return HttpResponse("files have not been deleted")
    elif request.method == 'POST':
        userFileCount = len(SavedFileInformation.objects.filter(did=did))
        if userFileCount >= 50:
            request.session['upload_and_sign_submit'] = True
            messages.warning(request, "you have reached the total number of files")
            form = UploadAndSignForm(initial={'did': did, 'api_key': request.session['api_key'],
                                              'private_key': request.session['private_key_mainchain']})

            return render(request, "service/upload_and_sign.html",
                          {'form': form, 'output': False, 'sample_code': sample_code,
                           'recent_services': recent_services, 'total_reached': True})

        if not request.session['upload_and_sign_submit']:
            # Purge old requests for housekeeping.
            UploadFile.objects.filter(did=did).delete()
            form = UploadAndSignForm(request.POST, request.FILES, initial={'did': did})
            if form.is_valid():
                network = form.cleaned_data.get('network')
                file_name = form.cleaned_data.get('file_name')
                " ".join(file_name.split())  # standardizing whitespaces
                api_key = form.cleaned_data.get('api_key')
                private_key = form.cleaned_data.get('private_key')
                file_content = form.cleaned_data.get('file_content').encode()
                userFile = SavedFileInformation.objects.filter(did=did, file_name=file_name)
                if len(userFile) != 0:
                    request.session['upload_and_sign_submit'] = True
                    messages.warning(request, "You have already upload a file with that name, please change the name "
                                              "of the file")
                    return redirect(reverse('service:upload_and_sign'))
                form.save()
                obj, created = UploadFile.objects.update_or_create(defaults={'did': did})
                true_file_name = obj.filename()
                if file_content:
                    remove_uploaded_file = False
                    user_uploaded_file = ContentFile(file_content)
                else:
                    remove_uploaded_file = True
                    user_uploaded_file = File(request.FILES['uploaded_file'])

                obj.uploaded_file.save(get_random_string(32), user_uploaded_file)
                try:
                    temp_file = UploadFile.objects.get(did=did)
                    file_path = temp_file.uploaded_file.path
                except Exception as e:
                    messages.success(request, "Please upload a file or fill out the 'File content' field")
                    return redirect(reverse('service:upload_and_sign'))
                try:
                    hive = Hive(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                    response = hive.upload_and_sign(api_key, did, network, private_key, file_path)
                    if response['status']:
                        request.session['upload_and_sign_submit'] = True
                        data = json.loads(response['output'])
                        message_hash = data['result']['msg']
                        public_key = data['result']['pub']
                        signature = data['result']['sig']
                        file_hash = data['result']['hash']
                        if network == 'gmunet':
                            SavedFileInformation.objects.update_or_create(did=did, file_name=file_name,
                                                                          message_hash=message_hash,
                                                                          signature=signature, file_hash=file_hash)

                        track_page_visit(did, 'Upload And Sign', "service:upload_and_sign", True, True)
                        return render(request, "service/upload_and_sign.html",
                                      {"message_hash": message_hash, "public_key": public_key, "signature": signature,
                                       "file_hash": file_hash, 'output': True, 'sample_code': sample_code,
                                       'recent_services': recent_services,
                                       'total_reached': False})
                    else:
                        messages.success(request, response['status_message'])
                        return redirect(reverse('service:upload_and_sign'))
                except Exception as e:
                    logging.debug(f"did: {did} Method: upload_and_sign Error: {e}")
                    messages.success(request, "File could not be uploaded. Please try again")
                    return redirect(reverse('service:upload_and_sign'))
                finally:
                    temp_file.delete()
                    if (remove_uploaded_file):
                        os.remove(os.path.join(MEDIA_ROOT, 'user_files', true_file_name))
                    hive.close()
        else:
            return redirect(reverse('service:upload_and_sign'))
    else:
        request.session['upload_and_sign_submit'] = False
        form = UploadAndSignForm(initial={'did': did, 'api_key': request.session['api_key'],
                                          'private_key': request.session['private_key_mainchain']})
        return render(request, "service/upload_and_sign.html",
                      {'form': form, 'output': False, 'sample_code': sample_code, 'recent_services': recent_services,
                       'total_reached': False})


@login_required
def verify_and_show(request):
    did = request.session['did']
    track_page_visit(did, 'Verify And Show', 'service:verify_and_show', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/verify_and_show.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/verify_and_show.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    if request.is_ajax():
        filename = request.POST.get('file_name')
        try:
            userFile = SavedFileInformation.objects.filter(did=did, file_name=filename)
            data = serializers.serialize('json', userFile)
            data = data[1:-1]
            return HttpResponse(data, content_type='application/json')
        except Exception as e:
            return redirect(reverse('service:verify_and_show'))
    elif request.method == 'POST':
        if not request.session['verify_and_show_submit']:
            form = VerifyAndShowForm(request.POST, did=did)
            if form.is_valid():
                network = form.cleaned_data.get('network')
                api_key = form.cleaned_data.get('api_key')
                request_input = {
                    "msg": form.cleaned_data.get('message_hash'),
                    "pub": form.cleaned_data.get('public_key'),
                    "sig": form.cleaned_data.get('signature'),
                    "hash": form.cleaned_data.get('file_hash'),
                    "privateKey": form.cleaned_data.get('private_key')
                }
                try:
                    hive = Hive(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                    response = hive.verify_and_show(api_key, did, network, request_input)

                    if response['status']:
                        request.session['verify_and_show_submit'] = True
                        file_content = response['file_content']
                        try:
                            content = file_content.decode()
                        except UnicodeDecodeError:
                            response = HttpResponse(file_content, content_type='application/octet-stream')
                            response['Content-Disposition'] = 'attachment; filename=file_from_hive'
                            return response
                        track_page_visit(did, 'Verify And Show', 'service:verify_and_show', True, True)
                        return render(request, 'service/verify_and_show.html',
                                      {'output': True, 'content': content, 'sample_code': sample_code,
                                       'recent_services': recent_services})
                    else:
                        messages.success(request, response['status_message'])
                        return redirect(reverse('service:verify_and_show'))
                except Exception as e:
                    logging.debug(f"did: {did} Method: verify_and_show Error: {e}")
                    messages.success(request, "File could not be verified nor shown. Please try again")
                    return redirect(reverse('service:verify_and_show'))
                finally:
                    hive.close()
        else:
            return redirect(reverse('service:verify_and_show'))
    else:
        request.session['verify_and_show_submit'] = False
        form = VerifyAndShowForm(
            initial={'api_key': request.session['api_key'], 'private_key': request.session['private_key_mainchain'],
                     'public_key': request.session['public_key_mainchain']}, did=did)
        return render(request, 'service/verify_and_show.html',
                      {'output': False, 'form': form, 'sample_code': sample_code, 'recent_services': recent_services})


@login_required
def create_wallet(request):
    did = request.session['did']
    track_page_visit(did, 'Create Wallet', 'service:create_wallet', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/create_wallet.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/create_wallet.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    if request.method == "POST":
        if not request.session['create_wallet_submit']:
            form = CreateWalletForm(request.POST)
            if form.is_valid():
                network = form.cleaned_data.get('network')
                api_key = form.cleaned_data.get('api_key')
                try:
                    wallet = Wallet(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                    response = wallet.create_wallet(api_key, did, network)
                    if response['status']:
                        request.session['create_wallet_submit'] = True
                        content = json.loads(response['output'])['result']
                        wallet_mainchain = content['mainchain']
                        wallet_did = content['sidechain']['did']
                        wallet_token = content['sidechain']['token']
                        wallet_eth = content['sidechain']['eth']
                        if network == 'gmunet':
                            obj, created = UserServiceSessionVars.objects.update_or_create(did=did,
                                                                                           defaults={'did': did,
                                                                                                     'api_key': api_key,
                                                                                                     'mnemonic_mainchain':
                                                                                                         wallet_mainchain[
                                                                                                             'mnemonic'],
                                                                                                     'public_key_mainchain':
                                                                                                         wallet_mainchain[
                                                                                                             'public_key'],
                                                                                                     'private_key_mainchain':
                                                                                                         wallet_mainchain[
                                                                                                             'private_key'],
                                                                                                     'address_mainchain':
                                                                                                         wallet_mainchain[
                                                                                                             'address'],
                                                                                                     'private_key_did':
                                                                                                         wallet_did[
                                                                                                             'private_key'],
                                                                                                     'public_key_did':
                                                                                                         wallet_did[
                                                                                                             'public_key'],
                                                                                                     'address_did':
                                                                                                         wallet_did[
                                                                                                             'address'],
                                                                                                     'did_did':
                                                                                                         wallet_did[
                                                                                                             'did'],
                                                                                                     'address_eth':
                                                                                                         wallet_eth[
                                                                                                             'address'],
                                                                                                     'private_key_eth':
                                                                                                         wallet_eth[
                                                                                                             'private_key']})
                            obj.save()
                        populate_session_vars_from_database(request, did)
                        track_page_visit(did, 'Create Wallet', "service:create_wallet", True, True)
                        return render(request, "service/create_wallet.html",
                                      {'output': True, 'wallet_mainchain': wallet_mainchain,
                                       'wallet_did': wallet_did, 'wallet_token': wallet_token, 'wallet_eth': wallet_eth,
                                       'sample_code': sample_code, 'recent_services': recent_services})
                    else:
                        messages.success(request, response['status_message'])
                        return redirect(reverse('service:create_wallet'))
                except Exception as e:
                    logging.debug(f"did: {did} Method: create_wallet Error: {e}")
                    messages.success(request, "Could not create wallet at this time. Please try again")
                    return redirect(reverse('service:create_wallet'))
                finally:
                    wallet.close()
        else:
            return redirect(reverse('service:create_wallet'))
    else:
        request.session['create_wallet_submit'] = False
        form = CreateWalletForm(initial={'api_key': request.session['api_key']})
        return render(request, 'service/create_wallet.html',
                      {'output': False, 'form': form, 'sample_code': sample_code, 'recent_services': recent_services})


@login_required
def view_wallet(request):
    did = request.session['did']
    track_page_visit(did, 'View Wallet', 'service:view_wallet', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/view_wallet.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/view_wallet.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    form_to_display = {
        'mainchain': ViewWalletForm(initial={'api_key': request.session['api_key'], 'chain': 'mainchain',
                                             'address': request.session['address_mainchain']}),
        'did': ViewWalletForm(
            initial={'api_key': request.session['api_key'], 'chain': 'did', 'address': request.session['address_did']}),
        'token': ViewWalletForm(initial={'api_key': request.session['api_key'], 'chain': 'token',
                                         'address': request.session['address_mainchain']}),
        'eth': ViewWalletForm(
            initial={'api_key': request.session['api_key'], 'chain': 'eth', 'address': request.session['address_eth']})
    }
    output = {
        'mainchain': False,
        'did': False,
        'token': False,
        'eth': False
    }
    if request.method == "POST":
        address = {
            'mainchain': '',
            'did': '',
            'token': '',
            'eth': ''
        }
        balance = {
            'mainchain': 0,
            'did': 0,
            'token': 0,
            'eth': 0
        }
        if 'submit_mainchain' in request.POST:
            chain = 'mainchain'
            form = ViewWalletForm(request.POST, initial={'chain': chain})
        elif 'submit_did' in request.POST:
            chain = 'did'
            form = ViewWalletForm(request.POST, initial={'chain': chain})
        elif 'submit_token' in request.POST:
            chain = 'token'
            form = ViewWalletForm(request.POST, initial={'chain': chain})
        elif 'submit_eth' in request.POST:
            chain = 'eth'
            form = ViewWalletForm(request.POST, initial={'chain': chain})

        if form.is_valid():
            network = form.cleaned_data.get('network')
            api_key = form.cleaned_data.get('api_key')
            addr = form.cleaned_data.get('address')
            try:
                node_rpc = NodeRpc(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                current_balance = node_rpc.get_current_balance(api_key, did, network, chain, addr)
                if current_balance:
                    output[chain] = True
                    address[chain] = addr
                    balance[chain] = current_balance
                    track_page_visit(did, 'View Wallet', 'service:view_wallet', True, True, chain)
                    return render(request, "service/view_wallet.html", {'output': output, 'form': form_to_display,
                                                                        'address': address, 'balance': balance,
                                                                        'sample_code': sample_code,
                                                                        'recent_services': recent_services})
                else:
                    messages.success(request, "Balance could not be retrieved")
                    return redirect(reverse('service:view_wallet'))
            except Exception as e:
                logging.debug(f"did: {did} Method: view_wallet Error: {e}")
                messages.success(request, "Could not view wallet at this time. Please try again")
                return redirect(reverse('service:view_wallet'))
            finally:
                node_rpc.close()
    else:
        return render(request, 'service/view_wallet.html',
                      {'output': output, 'form': form_to_display, 'sample_code': sample_code,
                       'recent_services': recent_services})


@login_required
def request_ela(request):
    did = request.session['did']
    track_page_visit(did, 'Request ELA', 'service:request_ela', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/request_ela.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/request_ela.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    form_to_display = {
        'mainchain': RequestELAForm(initial={'api_key': request.session['api_key'], 'chain': 'mainchain',
                                             'address': request.session['address_mainchain']}),
        'did': RequestELAForm(
            initial={'api_key': request.session['api_key'], 'chain': 'did', 'address': request.session['address_did']}),
        'token': RequestELAForm(initial={'api_key': request.session['api_key'], 'chain': 'token',
                                         'address': request.session['address_mainchain']}),
        'eth': RequestELAForm(
            initial={'api_key': request.session['api_key'], 'chain': 'eth', 'address': request.session['address_eth']})
    }
    output = {
        'mainchain': False,
        'did': False,
        'token': False,
        'eth': False
    }
    if request.method == "POST":
        address = {
            'mainchain': '',
            'did': '',
            'token': '',
            'eth': ''
        }
        deposit_amount = {
            'mainchain': 0,
            'did': 0,
            'token': 0,
            'eth': 0
        }
        if 'submit_mainchain' in request.POST:
            chain = 'mainchain'
            form = RequestELAForm(request.POST, initial={'chain': chain})
        elif 'submit_did' in request.POST:
            chain = 'did'
            form = RequestELAForm(request.POST, initial={'chain': chain})
        elif 'submit_token' in request.POST:
            chain = 'token'
            form = RequestELAForm(request.POST, initial={'chain': chain})
        elif 'submit_eth' in request.POST:
            chain = 'eth'
            form = RequestELAForm(request.POST, initial={'chain': chain})

        if form.is_valid():
            network = form.cleaned_data.get('network')
            api_key = form.cleaned_data.get('api_key')
            addr = form.cleaned_data.get('address')
            try:
                wallet = Wallet(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                response = wallet.request_ela(api_key, did, chain, addr)
                if response['status']:
                    output[chain] = True
                    content = json.loads(response['output'])['result']
                    address[chain] = content['address']
                    deposit_amount[chain] = content['deposit_amount']
                    track_page_visit(did, 'Request ELA', 'service:request_ela', True, True, chain)
                    return render(request, "service/request_ela.html", {'output': output, 'form': form_to_display,
                                                                        'address': address,
                                                                        'deposit_amount': deposit_amount,
                                                                        'sample_code': sample_code,
                                                                        'recent_services': recent_services})
                else:
                    messages.success(request, response['status_message'])
                    return redirect(reverse('service:request_ela'))
            except Exception as e:
                logging.debug(f"did: {did} Method: request_ela Error: {e}")
                messages.success(request, "Could not view wallet at this time. Please try again")
                return redirect(reverse('service:request_ela'))
            finally:
                wallet.close()
    else:
        return render(request, 'service/request_ela.html',
                      {'output': output, 'form': form_to_display, 'sample_code': sample_code,
                       'recent_services': recent_services})


@login_required
def deploy_eth_contract(request):
    did = request.session['did']
    track_page_visit(did, 'Deploy ETH Contract', 'service:deploy_eth_contract', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/deploy_eth_contract.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/deploy_eth_contract.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    did = request.session['did']
    if request.is_ajax():
        if request.POST.get('delete'):
            try:
                SavedETHContractInformation.objects.filter(did=did).delete()
                data = "{'deleted':'true'}"
                return HttpResponse(data, content_type='application/json')
            except Exception as e:
                logging.debug(e)
                data = "{'deleted':'false'}"
                return HttpResponse(data, content_type='application/json')

    if request.method == 'POST':
        if not request.session['deploy_eth_contract_submit']:
            if len(SavedETHContractInformation.objects.filter(did=did)) >= 50:
                request.session['deploy_eth_contract_submit'] = False
                form = DeployETHContractForm(initial={'did': did, 'api_key': request.session['api_key'],
                                                      'eth_account_address': request.session['address_eth'],
                                                      'eth_private_key': request.session['private_key_eth'],
                                                      'eth_gas': 2000000})
                return render(request, "service/deploy_eth_contract.html",
                              {'form': form, 'output': False, 'sample_code': sample_code,
                               'recent_services': recent_services, 'total_reached': True})

            # Purge old requests for housekeeping.
            UploadFile.objects.filter(did=did).delete()
            form = DeployETHContractForm(request.POST, request.FILES, initial={'did': did})
            if form.is_valid():
                file_name = form.cleaned_data.get('file_name')
                " ".join(file_name.split())  # standardizing whitespaces
                network = form.cleaned_data.get('network')
                api_key = form.cleaned_data.get('api_key')
                eth_account_address = form.cleaned_data.get('eth_account_address')
                eth_private_key = form.cleaned_data.get('eth_private_key')
                eth_gas = form.cleaned_data.get('eth_gas')
                form.save()
                obj, created = UploadFile.objects.update_or_create(defaults={'did': did})
                true_file_name = obj.filename()
                try:
                    file = File(request.FILES['uploaded_file'])
                    if len((SavedETHContractInformation.objects.filter(did=did, file_name=file_name))) != 0:
                        messages.success(request, "You have already uploaded a file with that name, please use a "
                                                  "different name")
                        return redirect(reverse('service:deploy_eth_contract'))
                    obj.uploaded_file.save(get_random_string(length=32), file)
                    temp_file = UploadFile.objects.get(did=did)
                    file_path = temp_file.uploaded_file.path
                    remove_file = True
                except Exception as e:
                    messages.success(request, "Please upload a .sol file or fill out the 'File content' field")
                    return redirect(reverse('service:deploy_eth_contract'))
                try:
                    sidechain_eth = SidechainEth(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                    response = sidechain_eth.deploy_eth_contract(api_key, did, network, eth_account_address,
                                                                 eth_private_key,
                                                                 eth_gas,
                                                                 file_path)
                    if response['status']:
                        request.session['deploy_eth_contract_submit'] = True
                        data = json.loads(response['output'])
                        contract_address = data['result']['contract_address']
                        contract_name = data['result']['contract_name']
                        contract_code_hash = data['result']['contract_code_hash']
                        SavedETHContractInformation.objects.update_or_create(file_name=file_name,
                                                                             contract_address=contract_address,
                                                                             contract_name=contract_name,
                                                                             contract_code_hash=contract_code_hash,
                                                                             did=did)
                        track_page_visit(did, 'Deploy ETH Contract', 'service:deploy_eth_contract', True, True)
                        return render(request, "service/deploy_eth_contract.html",
                                      {"contract_address": contract_address, "contract_name": contract_name,
                                       "contract_code_hash": contract_code_hash, 'output': True,
                                       'sample_code': sample_code,
                                       'recent_services': recent_services,
                                       'total_reached': False})
                    else:
                        messages.success(request, response['status_message'])
                        return redirect(reverse('service:deploy_eth_contract'))
                except Exception as e:
                    logging.debug(f"did: {did} Method: deploy_eth_contract Error: {e}")
                    messages.success(request, "Could not deploy smart contract to Eth sidechain. Please try again")
                    return redirect(reverse('service:deploy_eth_contract'))
                finally:
                    temp_file.delete()
                    if remove_file:
                        os.remove(os.path.join(MEDIA_ROOT, 'user_files', true_file_name))
                    sidechain_eth.close()
        else:
            return redirect(reverse('service:deploy_eth_contract'))
    else:
        request.session['deploy_eth_contract_submit'] = False
        form = DeployETHContractForm(initial={'did': did, 'api_key': request.session['api_key'],
                                              'eth_account_address': request.session['address_eth'],
                                              'eth_private_key': request.session['private_key_eth'],
                                              'eth_gas': 2000000})
        return render(request, "service/deploy_eth_contract.html",
                      {'form': form, 'output': False, 'sample_code': sample_code, 'recent_services': recent_services,
                       'total_reached': False})


@login_required
def watch_eth_contract(request):
    did = request.session['did']
    track_page_visit(did, 'Watch ETH Contract', 'service:watch_eth_contract', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/watch_eth_contract.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/watch_eth_contract.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    if request.is_ajax():
        file_name = request.POST.get('file_name')
        if file_name is not None:
            contract_info = SavedETHContractInformation.objects.filter(did=did, file_name=file_name)
            data = serializers.serialize('json', contract_info)
            data = data[1:-1]
            return HttpResponse(data, content_type='application/json')
    if request.method == 'POST':
        if not request.session['watch_eth_contract_submit']:
            form = WatchETHContractForm(request.POST, did=did)
            if form.is_valid():
                network = form.cleaned_data.get('network')
                api_key = form.cleaned_data.get('api_key')
                contract_address = form.cleaned_data.get('contract_address')
                contract_name = form.cleaned_data.get('contract_name')
                contract_code_hash = form.cleaned_data.get('contract_code_hash')
                try:
                    sidechain_eth = SidechainEth(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
                    response = sidechain_eth.watch_eth_contract(api_key, did, network, contract_address, contract_name,
                                                                contract_code_hash)
                    if response['status']:
                        request.session['watch_eth_contract_submit'] = True
                        data = json.loads(response['output'])
                        contract_address = data['result']['contract_address']
                        contract_functions = data['result']['contract_functions']
                        contract_source = data['result']['contract_source']
                        track_page_visit(did, 'Watch ETH Contract', 'service:watch_eth_contract', True, True)
                        return render(request, "service/watch_eth_contract.html",
                                      {'output': True, 'contract_address': contract_address,
                                       'contract_name': contract_name,
                                       'contract_functions': contract_functions, 'contract_source': contract_source,
                                       'sample_code': sample_code,
                                       'recent_services': recent_services})
                    else:
                        messages.success(request, response['status_message'])
                        return redirect(reverse('service:watch_eth_contract'))
                except Exception as e:
                    logging.debug(f"did={did} Method: watch_eth_contract Error: {e}")
                    messages.success(request, "Could not view smart contract code at this time. Please try again")
                    return redirect(reverse('service:watch_eth_contract'))
                finally:
                    sidechain_eth.close()
        else:
            return redirect(reverse('service:watch_eth_contract'))
    else:
        request.session['watch_eth_contract_submit'] = False
        form = WatchETHContractForm(initial={'api_key': request.session['api_key']}, did=did)
        return render(request, 'service/watch_eth_contract.html',
                      {'output': False, 'form': form, 'sample_code': sample_code, 'recent_services': recent_services})


@login_required
def run_eth_contract(request):
    did = request.session['did']
    track_page_visit(did, 'Run ETH Contract', 'service:watch_eth_contract', True)
    recent_services = get_recent_services(did)
    sample_code = {}
    module_dir = os.path.dirname(__file__)
    with open(os.path.join(module_dir, 'sample_code/python/run_eth_contract.py'), 'r') as myfile:
        sample_code['python'] = myfile.read()
    with open(os.path.join(module_dir, 'sample_code/go/run_eth_contract.go'), 'r') as myfile:
        sample_code['go'] = myfile.read()
    return render(request, "service/run_eth_contract.html",
                  {'sample_code': sample_code, 'recent_services': recent_services})
