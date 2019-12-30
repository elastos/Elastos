from django.shortcuts import render

from console_main.views import login_required, track_page_visit, get_recent_services

from .forms import SelectNetworkForm


@login_required
def mainchain(request):
    did = request.session['did']
    track_page_visit(did, 'Mainchain Browser', 'browser:mainchain', False)
    recent_services = get_recent_services(did)
    network = 'mainnet'
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
            return render(request, "browser/mainchain.html", context)
    else:
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/mainchain.html", context)


@login_required
def sidechain_did(request):
    did = request.session['did']
    track_page_visit(did, 'DID Sidechain Browser', 'browser:sidechain_did', False)
    recent_services = get_recent_services(did)
    network = 'mainnet'
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
            return render(request, "browser/sidechain_did.html", context)
    else:
        form = SelectNetworkForm(initial={'network': network})
        context['form'] = form
        return render(request, "browser/sidechain_did.html", context)


@login_required
def sidechain_token(request):
    # did = request.session['did']
    # track_page_visit(did, 'Token Sidechain Browser', 'browser:sidechain_token', False)
    # recent_services = get_recent_services(did)
    network = 'mainnet'
    context = {
        'network': network
        #   'recent_services': recent_services
    }
    return render(request, "browser/sidechain_token.html", context)


@login_required
def sidechain_eth(request):
    did = request.session['did']
    track_page_visit(did, 'ETH Sidechain Browser', 'browser:sidechain_eth', False)
    recent_services = get_recent_services(did)
    network = 'mainnet'
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
    network = 'mainnet'
    context = {
        'network': network
        #   'recent_services': recent_services
    }
    return render(request, "browser/sidechain_neo.html", context)
