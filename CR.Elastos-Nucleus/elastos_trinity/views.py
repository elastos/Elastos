from django.shortcuts import render

from console_main.views import login_required, track_page_visit, get_recent_services
from elastos_trinity.dapp_store import DAppStore


@login_required
def dapp_store_dashboard(request):
    context = {}
    did = request.session['did']
    track_page_visit(did, 'elastOS dApp Store Dashboard', 'elastos_trinity:dapp_store_dashboard', False)
    context['recent_services'] = get_recent_services(did)
    dapp_store = DAppStore()
    context['dapps_list'] = dapp_store.get_apps_list()
    return render(request, "elastos_trinity/dapp_store_dashboard.html", context)

