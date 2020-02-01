from django.shortcuts import render

from console_main.views import login_required, track_page_visit, get_recent_services


@login_required
def dapp_store_dashboard(request):
    did = request.session['did']
    track_page_visit(did, 'elastOS dApp Store Dashboard', 'elastos_trinity:dapp_store_dashboard', False)
    recent_services = get_recent_services(did)
    context = {
        'recent_services': recent_services
    }
    return render(request, "elastos_trinity/dapp_store_dashboard.html", context)

