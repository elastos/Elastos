import logging

from django.shortcuts import render, redirect
from django.contrib import messages
from django.urls import reverse

from elastos_adenine.node_rpc import NodeRpc

from console_main.settings import GRPC_SERVER_HOST, PRODUCTION, GRPC_SERVER_PORT
from console_main.views import login_required, track_page_visit, get_recent_services

from .forms import SelectNetworkForm

"""
node_rpc = NodeRpc(host, port, production)
print("--> Get current height")
current_height = node_rpc.get_current_height(network, "mainchain")
print("Current Height - mainchain: ", current_height)
current_height = node_rpc.get_current_height(network, "did")
print("Current Height - did sidechain: ", current_height)
current_height = node_rpc.get_current_height(network, "token")
print("Current Height - token sidechain: ", current_height)
print("--> Get current balance")
current_balance = node_rpc.get_current_balance(network, "mainchain", ela_to_use)
print("Current balance - mainchain:", current_balance)
current_balance = node_rpc.get_current_balance(network, "did", ela_to_use)
print("Current balance - did sidechain:", current_balance)
current_balance = node_rpc.get_current_balance(network, "token", ela_to_use)
print("Current balance - token sidechain:", current_balance)
print("--> Get current block details")
current_block_details = node_rpc.get_current_block_details(network, "mainchain")
print("Current block details - mainchain: ", current_block_details)
current_block_details = node_rpc.get_current_block_details(network, "did")
print("Current block details - did sidechain: ", current_block_details)
current_block_details = node_rpc.get_current_block_details(network, "token")
print("Current block details - token sidechain: ", current_block_details)
"""


def get_blockchain_node_data(request, network, chain, did):
    data = {}
    try:
        node_rpc = NodeRpc(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
        data["current_height"] = node_rpc.get_current_height(network, chain)
        data["current_block_details"] = node_rpc.get_current_block_details(network, chain)
        if chain == "mainchain":
            data["current_mining_info"] = node_rpc.get_current_mining_info(network)
            data["current_block_confirm"] = node_rpc.get_current_block_confirm(network)
            data["current_arbitrator_info"] = node_rpc.get_current_arbitrators_info(network)
            data["current_arbitrator_group"] = node_rpc.get_current_arbitrator_group(network)
    except Exception as e:
        logging.debug(f"did: {did} Method: browser:mainchain Error: {e}")
        messages.success(request, "Could not call a node rpc method. Please try again")
    finally:
        node_rpc.close()
    return data


@login_required
def mainchain(request):
    did = request.session['did']
    track_page_visit(did, 'Mainchain Browser', 'browser:mainchain', False)
    recent_services = get_recent_services(did)
    context = {
        'recent_services': recent_services
    }

    if request.method == 'POST':
        form = SelectNetworkForm(request.POST)
        if form.is_valid():
            network = form.cleaned_data.get('network')
            context['form'] = SelectNetworkForm(initial={'network': network})
            context['network'] = network
            data = get_blockchain_node_data(request, network, "mainchain", did)
            for k, v in data.items():
                context[k] = v
            return render(request, "browser/mainchain.html", context)
    else:
        network = 'gmunet'
        data = get_blockchain_node_data(request, network, "mainchain", did)
        for k, v in data.items():
            context[k] = v
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/mainchain.html", context)


@login_required
def sidechain_did(request):
    did = request.session['did']
    track_page_visit(did, 'DID Sidechain Browser', 'browser:sidechain_did', False)
    recent_services = get_recent_services(did)
    context = {
        'recent_services': recent_services
    }

    if request.method == 'POST':
        form = SelectNetworkForm(request.POST)
        if form.is_valid():
            network = form.cleaned_data.get('network')
            context['form'] = SelectNetworkForm(initial={'network': network})
            context['network'] = network
            data = get_blockchain_node_data(request, network, "did", did)
            for k, v in data.items():
                context[k] = v
            return render(request, "browser/sidechain_did.html", context)
    else:
        network = 'gmunet'
        data = get_blockchain_node_data(request, network, "did", did)
        for k, v in data.items():
            context[k] = v
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/sidechain_did.html", context)


@login_required
def sidechain_token(request):
    did = request.session['did']
    track_page_visit(did, 'Token Sidechain Browser', 'browser:sidechain_token', False)
    recent_services = get_recent_services(did)
    context = {
        'recent_services': recent_services
    }

    if request.method == 'POST':
        form = SelectNetworkForm(request.POST)
        if form.is_valid():
            network = form.cleaned_data.get('network')
            context['form'] = SelectNetworkForm(initial={'network': network})
            context['network'] = network
            data = get_blockchain_node_data(request, network, "token", did)
            for k, v in data.items():
                context[k] = v
            return render(request, "browser/sidechain_token.html", context)
    else:
        network = 'gmunet'
        data = get_blockchain_node_data(request, network, "token", did)
        for k, v in data.items():
            context[k] = v
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/sidechain_token.html", context)


@login_required
def sidechain_eth(request):
    did = request.session['did']
    track_page_visit(did, 'ETH Sidechain Browser', 'browser:sidechain_eth', False)
    recent_services = get_recent_services(did)
    network = 'gmunet'
    context = {
        'network': network,
        'recent_services': recent_services
    }
    if request.method == 'POST':
        form = SelectNetworkForm(request.POST)
        if form.is_valid():
            network = form.cleaned_data.get('network')
            context['form'] = SelectNetworkForm(initial={'network': network})
            context['network'] = network
            return render(request, "browser/sidechain_eth.html", context)
    else:
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/sidechain_eth.html", context)


@login_required
def sidechain_neo(request):
    # did = request.session['did']
    # track_page_visit(did, 'NEO Sidechain Browser', 'browser:sidechain_neo', False)
    # recent_services = get_recent_services(did)
    network = 'gmunet'
    context = {
        'network': network
        #   'recent_services': recent_services
    }
    return render(request, "browser/sidechain_neo.html", context)
