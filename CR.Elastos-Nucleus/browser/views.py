import logging
import pprint
import math

from django.shortcuts import render
from django.contrib import messages
from elastos_adenine.node_rpc import NodeRpc

from console_main.settings import GRPC_SERVER_HOST, PRODUCTION, GRPC_SERVER_PORT
from console_main.views import login_required, track_page_visit, get_recent_services

from .time_convert import display_time_elapsed
from .forms import SelectNetworkForm

from datetime import datetime
from django.utils import timezone


@login_required
def mainchain(request):
    api_key = request.session['api_key']
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
            data = get_blockchain_node_data(request, api_key, did, network, "mainchain")
            for k, v in data.items():
                context[k] = v
            return render(request, "browser/mainchain.html", {'context': context, 'blocks': context['recent_blocks']})
    else:
        network = 'gmunet'
        data = get_blockchain_node_data(request, api_key, did, network, "mainchain")
        for k, v in data.items():
            context[k] = v
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/mainchain.html", {'context': context, 'blocks': context['recent_blocks']})


@login_required
def sidechain_did(request):
    api_key = request.session['api_key']
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
            data = get_blockchain_node_data(request, api_key, did, network, "did")
            for k, v in data.items():
                context[k] = v
            return render(request, "browser/sidechain_did.html", context)
    else:
        network = 'gmunet'
        data = get_blockchain_node_data(request, api_key, did, network, "did")
        for k, v in data.items():
            context[k] = v
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/sidechain_did.html", context)


@login_required
def sidechain_token(request):
    api_key = request.session['api_key']
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
            data = get_blockchain_node_data(request, api_key, did, network, "token")
            for k, v in data.items():
                context[k] = v
            return render(request, "browser/sidechain_token.html", context)
    else:
        network = 'gmunet'
        data = get_blockchain_node_data(request, api_key, did, network, "token")
        for k, v in data.items():
            context[k] = v
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/sidechain_token.html", context)


@login_required
def sidechain_eth(request):
    api_key = request.session['api_key']
    did = request.session['did']
    track_page_visit(did, 'ETH Sidechain Browser', 'browser:sidechain_eth', False)
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
            data = get_blockchain_node_data(request, api_key, did, network, "eth")
            for k, v in data.items():
                context[k] = v
            return render(request, "browser/sidechain_eth.html", context)
    else:
        network = 'gmunet'
        data = get_blockchain_node_data(request, api_key, did, network, "eth")
        for k, v in data.items():
            context[k] = v
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


def get_blockchain_node_data(request, api_key, did, network, chain):
    data = {}
    try:
        node_rpc = NodeRpc(GRPC_SERVER_HOST, GRPC_SERVER_PORT, PRODUCTION)
        data["current_height"] = node_rpc.get_current_height(api_key, did, network, chain)

        data["recent_blocks"] = {}
        for index in range(5):
            data["recent_blocks"]["block" + str(index + 1)] = node_rpc.get_block_info(api_key, did, network, chain, data["current_height"] - (index + 1))

            # Get number of transactions
            blockVout = data["recent_blocks"]["block" + str(index + 1)]["tx"]
            numTransactions = len((blockVout[0])["vout"])
            data["recent_blocks"]["block" + str(index + 1)]["numtransactions"] = numTransactions

            # Convert timestamp to datetime
            dt = datetime.fromtimestamp(data["recent_blocks"]["block" + str(index + 1)]["time"])
            new_datetime = timezone.make_aware(dt, timezone.utc)

            time_elapsed_in_seconds = math.floor((timezone.now() - new_datetime).seconds)

            last_visited = "{0} ago".format(display_time_elapsed(time_elapsed_in_seconds))
            data["recent_blocks"]["block" + str(index + 1)]["last_time_visited"] = last_visited

            # Get initials on miner
            data["recent_blocks"]["block" + str(index + 1)]["abbreviation"] = data["recent_blocks"]["block" + str(index + 1)]["minerinfo"][:2]

            # Get total ELA transferred per block
            elaTransferred = 0
            vout = (blockVout[0])["vout"]
            for child in vout:
                elaTransferred += float(child["value"])
            data["recent_blocks"]["block" + str(index + 1)]["totaltransactions"] = elaTransferred

        if chain == "mainchain":
            data["current_mining_info"] = node_rpc.get_current_mining_info(api_key, did, network)
            data["current_block_confirm"] = node_rpc.get_current_block_confirm(api_key, did, network)
            data["current_arbitrator_info"] = node_rpc.get_current_arbitrators_info(api_key, did, network)
            data["current_arbitrator_group"] = node_rpc.get_current_arbitrator_group(api_key, did, network)
    except Exception as e:
        logging.debug(f"did: {did} Method: browser:mainchain Error: {e}")
        messages.success(request, "Could not call a node rpc method. Please try again")
    finally:
        node_rpc.close()
    return data
