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
from .forms import DeployETHContractForm
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

        private_key = request.POST['private_key']
        api_key = request.POST['api_key']
        form = UploadAndSignForm(request.POST, request.FILES, initial={'did': did})
        if form.is_valid():
            form.save()
            temp_file = UploadFile.objects.get(did=did)
            file_path = temp_file.uploaded_file.path
            console = Hive()
            response = console.upload_and_sign(api_key, private_key, file_path)
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

    else:
        form = UploadAndSignForm(initial={'did': did})
        return render(request, "service/upload_and_sign.html", {'form': form, 'output': False})


@login_required
def verify_and_show(request):
    if request.method == 'POST':
        output = True
        msg = request.POST['message_hash']
        private_key = request.POST['private_key']
        public_key = request.POST['public_key']
        file_hash = request.POST['file_hash']
        signature = request.POST['signature']
        api_key = request.POST['api_key']
        request_input = {
            "msg": msg,
            "pub": public_key,
            "sig": signature,
            "hash": file_hash,
            "private_key": private_key
        }
        console = Hive()
        response = console.verify_and_show(api_key, request_input)
        if response.status:
            content = response.output
            return render(request, 'service/verify_and_show.html', {'output': output, 'content': content})
        else:
            messages.success(request, "File could not be verified nor shown. Please try again")
            return redirect(reverse('service:verify_and_show'))
    else:
        output = False
        form = VerifyAndShowForm()
        return render(request, 'service/verify_and_show.html', {'output': output, 'form': form})


@login_required
def create_wallet(request):
    if request.method == "POST":
        try:
            wallet = Wallet()
            api_key = request.POST['api_key']
            eth_password = request.POST['eth_password']
            response = wallet.create_wallet(api_key, eth_password)
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

        eth_account_address = request.POST['eth_account_address']
        eth_account_password = request.POST['eth_account_password']
        api_key = request.POST['api_key']
        form = DeployETHContractForm(request.POST, request.FILES, initial={'did': did})
        if form.is_valid():
            form.save()
            temp_file = UploadFile.objects.get(did=did)
            file_path = temp_file.uploaded_file.path
            console = SidechainEth()
            response = console.deploy_eth_contract(api_key, eth_account_address, eth_account_password, file_path)
            data = json.loads(response.output)
            if response.status:
                temp_file.delete()
                contract_address = data['result']['contract_address']
                return render(request, "service/deploy_eth_contract.html",
                              {"contract_address": contract_address, 'output': True})
            else:
                messages.success(request, "File could not be uploaded. Please try again")
                return redirect(reverse('service:deploy_eth_contract'))

    else:
        form = DeployETHContractForm(initial={'did': did})
        return render(request, "service/deploy_eth_contract.html", {'form': form, 'output': False})


@login_required
def watch_eth_contract(request):
    return render(request, "service/watch_eth_contract.html")


@login_required
def run_eth_contract(request):
    return render(request, "service/run_eth_contract.html")

