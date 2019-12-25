from django.shortcuts import render

from console_main.views import login_required

from .forms import SelectNetworkForm


@login_required
def mainchain(request):
    network = 'mainnet'
    context = { 'network': network }
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
    network = 'mainnet'
    context = { 'network': network }
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
    return render(request, "browser/sidechain_token.html")


@login_required
def sidechain_eth(request):
    network = 'mainnet'
    context = { 'network': network }
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
    return render(request, "browser/sidechain_neo.html")