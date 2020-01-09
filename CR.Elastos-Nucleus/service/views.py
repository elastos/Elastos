import json
import logging
import os

from django.core import serializers
from django.http import HttpResponse

from decouple import config
from django.core.files.base import ContentFile
from django.shortcuts import render, redirect
from django.utils.crypto import get_random_string

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

from .models import UploadFile, UserServiceSessionVars, SavedFileInformation


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
                common = Common()
                error_message = None
                output = {}
                if 'submit_get_api_key' in request.POST:
                    response = common.get_api_key_request(config('SHARED_SECRET_ADENINE'), did)
                    if response.status:
                        api_key = response.api_key
                        obj, created = UserServiceSessionVars.objects.update_or_create(did=did,
                                                                                       defaults={'did': did,
                                                                                                 'api_key': api_key})
                        obj.save()
                        populate_session_vars_from_database(request, did)
                        output['get_api_key'] = True
                    else:
                        error_message = response.status_message
                elif 'submit_generate_api_key' in request.POST:
                    response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), did)
                    if response.status:
                        api_key = response.api_key
                        obj, created = UserServiceSessionVars.objects.update_or_create(did=did,
                                                                                       defaults={'did': did,
                                                                                                 'api_key': api_key})
                        obj.save()
                        populate_session_vars_from_database(request, did)
                        output['generate_api_key'] = True
                    else:
                        error_message = response.status_message
                else:
                    error_message = "Invalid form submission. Please refresh the page and try generating a new API " \
                                    "key again "
                if error_message:
                    messages.success(request, error_message)
                    return redirect(reverse('service:generate_key'))
                else:
                    request.session['api_key'] = api_key
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
    if request.method == 'POST':
        if not request.session['upload_and_sign_submit']:
            # Purge old requests for housekeeping.
            UploadFile.objects.filter(did=did).delete()

            form = UploadAndSignForm(request.POST, request.FILES, initial={'did': did})
            if form.is_valid():
                network = form.cleaned_data.get('network')
                api_key = form.cleaned_data.get('api_key')
                private_key = form.cleaned_data.get('private_key')
                file_content = form.cleaned_data.get('file_content').encode()
                form.save()
                if file_content:
                    obj, created = UploadFile.objects.update_or_create(did=did)
                    obj.uploaded_file.save(get_random_string(length=32), ContentFile(file_content))
                try:
                    temp_file = UploadFile.objects.get(did=did)
                    file_path = temp_file.uploaded_file.path
                except Exception as e:
                    messages.success(request, "Please upload a file or fill out the 'File content' field")
                    return redirect(reverse('service:upload_and_sign'))
                try:
                    hive = Hive()
                    response = hive.upload_and_sign(api_key, private_key, file_path)
                    data = json.loads(response.output)
                    if response.status:
                        request.session['upload_and_sign_submit'] = True
                        message_hash = data['result']['msg']
                        public_key = data['result']['pub']
                        signature = data['result']['sig']
                        file_hash = data['result']['hash']
                        if network == 'gmunet':
                            SavedFileInformation.objects.update_or_create(did=did, file_name=temp_file.filename(),
                                                                          message_hash=message_hash,
                                                                          signature=signature, file_hash=file_hash)
                        return render(request, "service/upload_and_sign.html",
                                      {"message_hash": message_hash, "public_key": public_key, "signature": signature,
                                       "file_hash": file_hash, 'output': True, 'sample_code': sample_code,
                                       'recent_services': recent_services})
                    else:
                        messages.success(request, response.status_message)
                        return redirect(reverse('service:upload_and_sign'))
                except Exception as e:
                    logging.debug(f"did: {did} Method: upload_and_sign Error: {e}")
                    messages.success(request, "File could not be uploaded. Please try again")
                    return redirect(reverse('service:upload_and_sign'))
                finally:
                    temp_file.delete()
                    hive.close()
        else:
            return redirect(reverse('service:upload_and_sign'))
    else:
        request.session['upload_and_sign_submit'] = False
        form = UploadAndSignForm(initial={'did': did, 'api_key': request.session['api_key'],
                                          'private_key': request.session['private_key_mainchain']})
        return render(request, "service/upload_and_sign.html",
                      {'form': form, 'output': False, 'sample_code': sample_code, 'recent_services': recent_services})


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
                    "private_key": form.cleaned_data.get('private_key')
                }
                try:
                    hive = Hive()
                    response = hive.verify_and_show(api_key, request_input)
                    if response.status:
                        request.session['verify_and_show_submit'] = True
                        content = json.loads(response.output)["result"]["output"]
                        return render(request, 'service/verify_and_show.html',
                                      {'output': True, 'content': content, 'sample_code': sample_code,
                                       'recent_services': recent_services})
                    else:
                        messages.success(request, response.status_message)
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
                    wallet = Wallet()
                    response = wallet.create_wallet(api_key)
                    if response.status:
                        request.session['create_wallet_submit'] = True
                        content = json.loads(response.output)['result']
                        wallet_mainchain = content['mainchain']
                        wallet_did = content['sidechain']['did']
                        wallet_token = content['sidechain']['token']
                        wallet_eth = content['sidechain']['eth']
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
                                                                                                 'did_did': wallet_did[
                                                                                                     'did'],
                                                                                                 'address_eth':
                                                                                                     wallet_eth[
                                                                                                         'address'],
                                                                                                 'private_key_eth':
                                                                                                     wallet_eth[
                                                                                                         'private_key']})
                        obj.save()
                        populate_session_vars_from_database(request, did)
                        return render(request, "service/create_wallet.html",
                                      {'output': True, 'wallet_mainchain': wallet_mainchain,
                                       'wallet_did': wallet_did, 'wallet_token': wallet_token, 'wallet_eth': wallet_eth,
                                       'sample_code': sample_code, 'recent_services': recent_services})
                    else:
                        messages.success(request, response.status_message)
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
                wallet = Wallet()
                response = wallet.view_wallet(api_key, chain, addr)
                if response.status:
                    output[chain] = True
                    content = json.loads(response.output)['result']
                    address[chain] = content['address']
                    balance[chain] = content['balance']
                    return render(request, "service/view_wallet.html", {'output': output, 'form': form_to_display,
                                                                        'address': address, 'balance': balance,
                                                                        'sample_code': sample_code,
                                                                        'recent_services': recent_services})
                else:
                    messages.success(request, response.status_message)
                    return redirect(reverse('service:view_wallet'))
            except Exception as e:
                logging.debug(f"did: {did} Method: view_wallet Error: {e}")
                messages.success(request, "Could not view wallet at this time. Please try again")
                return redirect(reverse('service:view_wallet'))
            finally:
                wallet.close()
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
                wallet = Wallet()
                response = wallet.request_ela(api_key, chain, addr)
                if response.status:
                    output[chain] = True
                    content = json.loads(response.output)['result']
                    address[chain] = content['address']
                    deposit_amount[chain] = content['deposit_amount']
                    return render(request, "service/request_ela.html", {'output': output, 'form': form_to_display,
                                                                        'address': address,
                                                                        'deposit_amount': deposit_amount,
                                                                        'sample_code': sample_code,
                                                                        'recent_services': recent_services})
                else:
                    messages.success(request, response.status_message)
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
    if request.method == 'POST':
        if not request.session['deploy_eth_contract_submit']:
            # Purge old requests for housekeeping.
            UploadFile.objects.filter(did=did).delete()

            form = DeployETHContractForm(request.POST, request.FILES, initial={'did': did})
            if form.is_valid():
                network = form.cleaned_data.get('network')
                api_key = form.cleaned_data.get('api_key')
                eth_account_address = form.cleaned_data.get('eth_account_address')
                eth_private_key = form.cleaned_data.get('eth_private_key')
                eth_gas = form.cleaned_data.get('eth_gas')
                form.save()
                temp_file = UploadFile.objects.get(did=did)
                file_path = temp_file.uploaded_file.path
                try:
                    sidechain_eth = SidechainEth()
                    response = sidechain_eth.deploy_eth_contract(api_key, eth_account_address, eth_private_key, eth_gas,
                                                                 file_path)
                    data = json.loads(response.output)
                    if response.status:
                        request.session['deploy_eth_contract_submit'] = True
                        temp_file.delete()
                        contract_address = data['result']['contract_address']
                        contract_name = data['result']['contract_name']
                        contract_code_hash = data['result']['contract_code_hash']
                        return render(request, "service/deploy_eth_contract.html",
                                      {"contract_address": contract_address, "contract_name": contract_name,
                                       "contract_code_hash": contract_code_hash, 'output': True,
                                       'sample_code': sample_code,
                                       'recent_services': recent_services})
                    else:
                        messages.success(request, response.status_message)
                        return redirect(reverse('service:deploy_eth_contract'))
                except Exception as e:
                    logging.debug(f"did: {did} Method: deploy_eth_contract Error: {e}")
                    messages.success(request, "Could not deploy smart contract to Eth sidechain. Please try again")
                    return redirect(reverse('service:deploy_eth_contract'))
                finally:
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
                      {'form': form, 'output': False, 'sample_code': sample_code, 'recent_services': recent_services})


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
    if request.method == 'POST':
        if not request.session['watch_eth_contract_submit']:
            form = WatchETHContractForm(request.POST)
            if form.is_valid():
                network = form.cleaned_data.get('network')
                api_key = form.cleaned_data.get('api_key')
                contract_address = form.cleaned_data.get('contract_address')
                contract_name = form.cleaned_data.get('contract_name')
                contract_code_hash = form.cleaned_data.get('contract_code_hash')
                try:
                    sidechain_eth = SidechainEth()
                    response = sidechain_eth.watch_eth_contract(api_key, contract_address, contract_name,
                                                                contract_code_hash)
                    data = json.loads(response.output)
                    if response.status:
                        request.session['watch_eth_contract_submit'] = True
                        contract_address = data['result']['contract_address']
                        contract_functions = data['result']['contract_functions']
                        contract_source = data['result']['contract_source']
                        return render(request, "service/watch_eth_contract.html",
                                      {'output': True, 'contract_address': contract_address,
                                       'contract_name': contract_name,
                                       'contract_functions': contract_functions, 'contract_source': contract_source,
                                       'sample_code': sample_code,
                                       'recent_services': recent_services})
                    else:
                        messages.success(request, response.status_message)
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
        form = WatchETHContractForm(initial={'api_key': request.session['api_key']})
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


@login_required
def suggest_service(request):
    did = request.session['did']
    track_page_visit(did, 'Suggest a new service', 'service:suggest_service', True)
    recent_services = get_recent_services(did)
    return render(request, "service/suggest_service.html", {'recent_services': recent_services})
