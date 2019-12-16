import json

from decouple import config
from django.http import JsonResponse
from django.shortcuts import render, redirect

from console_main.views import login_required
from django.contrib import messages
from django.urls import reverse

from elastos_adenine.common import Common
from elastos_adenine.hive import Hive
from elastos_adenine.sidechain_eth import SidechainEth
from elastos_adenine.wallet import Wallet

from .forms import UploadAndSignForm, VerifyAndShowForm
from .forms import CreateWalletForm, ViewWalletForm, RequestELAForm
from .forms import DeployETHContractForm, WatchETHContractForm
from .models import UploadFile


@login_required
def generate_key(request):
    if request.method == 'POST':
        common = Common()
        did = request.session['did']
        response = common.generate_api_request(config('SHARED_SECRET_ADENINE'), did)
        if response.status:
            api_key = response.api_key
            return JsonResponse({'API_KEY': api_key}, status=200)
        else:
            messages.success(request, "Could not generate an API key. Please try again")
            return redirect(reverse('service:generate_key'))
    else:
        return render(request, "service/generate_key.html")


@login_required
def upload_and_sign(request):
    did = request.session['did']
    if request.method == 'POST':
        # Purge old requests for housekeeping.
        UploadFile.objects.filter(did=did).delete()

        form = UploadAndSignForm(request.POST, request.FILES, initial={'did': did})
        if form.is_valid():
            api_key = form.cleaned_data.get('api_key')
            private_key = form.cleaned_data.get('private_key')
            form.save()
            temp_file = UploadFile.objects.get(did=did)
            file_path = temp_file.uploaded_file.path
            try:
                hive = Hive()
                response = hive.upload_and_sign(api_key, private_key, file_path)
                data = json.loads(response.output)
                if response.status:
                    temp_file.delete()
                    message_hash = data['result']['msg']
                    public_key = data['result']['pub']
                    signature = data['result']['sig']
                    file_hash = data['result']['hash']
                    return render(request, "service/upload_and_sign.html",
                                  {"message_hash": message_hash, "public_key": public_key, "signature": signature,
                                   "file_hash": file_hash, 'output': True})
                else:
                    messages.success(request, "File could not be uploaded. Please try again")
                    return redirect(reverse('service:upload_and_sign'))
            except Exception as e:
                messages.success(request, "File could not be uploaded. Please try again")
                return redirect(reverse('service:upload_and_sign'))
            finally:
                hive.close()
    else:
        form = UploadAndSignForm(initial={'did': did})
        return render(request, "service/upload_and_sign.html", {'form': form, 'output': False})


@login_required
def verify_and_show(request):
    if request.method == 'POST':
        form = VerifyAndShowForm(request.POST)
        if form.is_valid():
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
                    content = response.output
                    return render(request, 'service/verify_and_show.html', {'output': True, 'content': content})
                else:
                    messages.success(request, "File could not be verified nor shown. Please try again")
                    return redirect(reverse('service:verify_and_show'))
            except Exception as e:
                messages.success(request, "File could not be verified nor shown. Please try again")
                return redirect(reverse('service:verify_and_show'))
            finally:
                hive.close()
    else:
        form = VerifyAndShowForm()
        return render(request, 'service/verify_and_show.html', {'output': False, 'form': form})


@login_required
def create_wallet(request):
    if request.method == "POST":
        form = CreateWalletForm(request.POST)
        if form.is_valid():
            api_key = form.cleaned_data.get('api_key')
            try:
                wallet = Wallet()
                response = wallet.create_wallet(api_key)
                if response.status:
                    content = json.loads(response.output)['result']
                    wallet_mainchain = content['mainchain']
                    wallet_did = content['sidechain']['did']
                    wallet_token = content['sidechain']['token']
                    wallet_eth = content['sidechain']['eth']
                    return render(request, "service/create_wallet.html", { 'output': True, 'wallet_mainchain': wallet_mainchain,
                        'wallet_did': wallet_did, 'wallet_token': wallet_token, 'wallet_eth': wallet_eth })
                else:
                    messages.success(request, "Could not create wallet at this time. Please try again")
                    return redirect(reverse('service:create_wallet'))
            except Exception as e:
                messages.success(request, "Could not create wallet at this time. Please try again")
                return redirect(reverse('service:create_wallet'))
            finally:
                wallet.close()
    else:
        form = CreateWalletForm()
        return render(request, 'service/create_wallet.html', {'output': False, 'form': form})


@login_required
def view_wallet(request):
    form_to_display = {
        'mainchain': ViewWalletForm(initial={'chain': 'mainchain'}),
        'did': ViewWalletForm(initial={'chain': 'did'}),
        'token': ViewWalletForm(initial={'chain': 'token'}),
        'eth': ViewWalletForm(initial={'chain': 'eth'})
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
                    return render(request, "service/view_wallet.html", { 'output': output, 'form': form_to_display,
                        'address': address, 'balance': balance })
                else:
                    messages.success(request, "Could not view wallet at this time. Please try again")
                    return redirect(reverse('service:view_wallet'))
            except Exception as e:
                messages.success(request, "Could not view wallet at this time. Please try again")
                return redirect(reverse('service:view_wallet'))
            finally:
                wallet.close()
    else:
        return render(request, 'service/view_wallet.html', {'output': output, 'form': form_to_display})


@login_required
def request_ela(request):
    form_to_display = {
        'mainchain': RequestELAForm(initial={'chain': 'mainchain'}),
        'did': RequestELAForm(initial={'chain': 'did'}),
        'token': RequestELAForm(initial={'chain': 'token'}),
        'eth': RequestELAForm(initial={'chain': 'eth'})
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
                    return render(request, "service/request_ela.html", { 'output': output, 'form': form_to_display,
                        'address': address, 'deposit_amount': deposit_amount })
                else:
                    messages.success(request, "Could not view wallet at this time. Please try again")
                    return redirect(reverse('service:request_ela'))
            except Exception as e:
                messages.success(request, "Could not view wallet at this time. Please try again")
                return redirect(reverse('service:request_ela'))
            finally:
                wallet.close()
    else:
        return render(request, 'service/request_ela.html', {'output': output, 'form': form_to_display})


@login_required
def deploy_eth_contract(request):
    did = request.session['did']
    if request.method == 'POST':
        # Purge old requests for housekeeping.
        UploadFile.objects.filter(did=did).delete()

        form = DeployETHContractForm(request.POST, request.FILES, initial={'did': did})
        if form.is_valid():
            api_key = form.cleaned_data.get('api_key')
            eth_account_address = form.cleaned_data.get('eth_account_address')
            eth_private_key = form.cleaned_data.get('eth_private_key')
            eth_gas = form.cleaned_data.get('eth_gas')
            form.save()
            temp_file = UploadFile.objects.get(did=did)
            file_path = temp_file.uploaded_file.path
            try:
                sidechain_eth = SidechainEth()
                response = sidechain_eth.deploy_eth_contract(api_key, eth_account_address, eth_private_key, eth_gas, file_path)
                data = json.loads(response.output)
                if response.status:
                    temp_file.delete()
                    contract_address = data['result']['contract_address']
                    contract_name = data['result']['contract_name']
                    contract_code_hash = data['result']['contract_code_hash']
                    return render(request, "service/deploy_eth_contract.html",
                                  {"contract_address": contract_address, "contract_name": contract_name,
                                   "contract_code_hash": contract_code_hash, 'output': True})
                else:
                    messages.success(request, "Could not deploy smart contract to Eth sidechain. Please try again")
                    return redirect(reverse('service:deploy_eth_contract'))
            except Exception as e:
                messages.success(request, "Could not deploy smart contract to Eth sidechain. Please try again")
                return redirect(reverse('service:deploy_eth_contract'))
            finally:
                sidechain_eth.close()
    else:
        form = DeployETHContractForm(initial={'did': did})
        return render(request, "service/deploy_eth_contract.html", {'form': form, 'output': False})


@login_required
def watch_eth_contract(request):
    if request.method == 'POST':
        form = WatchETHContractForm(request.POST)
        if form.is_valid():
            api_key = form.cleaned_data.get('api_key')
            contract_address = form.cleaned_data.get('contract_address')
            contract_name = form.cleaned_data.get('contract_name')
            contract_code_hash = form.cleaned_data.get('contract_code_hash')
            try:
                sidechain_eth = SidechainEth()
                response = sidechain_eth.watch_eth_contract(api_key, contract_address, contract_name, contract_code_hash)
                data = json.loads(response.output)
                if response.status:
                    contract_address = data['result']['contract_address']
                    contract_functions = data['result']['contract_functions']
                    contract_source = data['result']['contract_source']
                    return render(request, "service/watch_eth_contract.html", {'output': True, 'contract_address': contract_address, 'contract_name': contract_name,
                                                                               'contract_functions': contract_functions, 'contract_source': contract_source})
                else:
                    messages.success(request, "Could not view smart contract code at this time. Please try again")
                    return redirect(reverse('service:watch_eth_contract'))
            except Exception as e:
                messages.success(request, "Could not view smart contract code at this time. Please try again")
                return redirect(reverse('service:watch_eth_contract'))
            finally:
                sidechain_eth.close()
    else:
        form = WatchETHContractForm()
        return render(request, 'service/watch_eth_contract.html', {'output': False, 'form': form})


@login_required
def run_eth_contract(request):
    return render(request, "service/run_eth_contract.html")

