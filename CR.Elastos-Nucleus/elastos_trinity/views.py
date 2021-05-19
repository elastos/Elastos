import logging
import requests
import namegenerator

from decouple import config
from django.shortcuts import render
from console_main.views import login_required, track_page_visit, get_recent_services
from elastos_trinity.dapp_store import DAppStore
from django.db.models.expressions import RawSQL

from .dapps_list import get_dapp_list, get_dapp_link, get_github_link, get_download_link, get_github_api_link


@login_required
def dapp_store_dashboard(request):
    context = {}
    did = request.session['did']
    track_page_visit(did, 'elastOS dApp Store Dashboard',
                     'elastos_trinity:dapp_store_dashboard', False)
    context['recent_services'] = get_recent_services(did)
    dapp_store = DAppStore()
    dapps_list = dapp_store.get_apps_list()
    dapp_store_url = config('ELASTOS_TRINITY_DAPPSTORE_URL')
    for dapp in dapps_list:
        createdAt = dapp["createdAt"][:10]
        strippedName = dapp["appName"].replace(" ", "")
        removedName = ''.join([i for i in strippedName if not i.isdigit()])
        uniqueName = removedName + namegenerator.gen()
        dapp["uniqueName"] = uniqueName
        dapp["createdAt"] = createdAt
        dapp["id"] = dapp["_id"]
        dapp["icon_url"] = f"{dapp_store_url}/apps/{dapp['id']}/icon"
        dapp["download_url"] = f"https://scheme.elastos.org/app?id={dapp['packageName']}"
    context['dapps_list'] = dapps_list
    context['top_downloads'] = sorted(
        dapps_list, key=lambda k: k['downloadsCount'], reverse=True)[:4]
    return render(request, "elastos_trinity/dapp_store_dashboard.html", context)


@login_required
def dapp_templates(request):
    context = {}
    did = request.session['did']
    if request.method == "POST":
        dapp_id = request.POST.get('dapp_id')
        dapp_url = get_dapp_link(dapp_id)
        github_link = get_github_link(dapp_id)
        download_link = get_download_link(dapp_id)
        get_github_api_link(dapp_id)
        angular_check = dapp_url + "angular.json"
        is_angular = False
        try:
            angular_request = requests.get(angular_check)
            if int(angular_request.status_code) == 200:
                is_angular = True
        except Exception as e:
            logging.debug(
                f"Method: dapp_templates Type: angular_check Error: {e}")

        if is_angular:
            request_string = dapp_url + "src/assets/manifest.json"
        else:
            request_string = dapp_url + "public/assets/manifest.json"

        try:
            manifest = requests.get(request_string)
            json_dict = manifest.json()
        except Exception as e:
            logging.debug(
                f"Method: dapp_templates Type: manifest_check Error: {e}")

        logo_location = json_dict['icons'][1].get('src')
        if is_angular:
            logo_string = dapp_url + "src/" + logo_location
        else:
            logo_string = dapp_url + "public/" + logo_location

        readme_url = dapp_url + "README.md"
        readme_html = ""
        try:
            readme = requests.get(readme_url)
            readme = readme.text
            headers = {
                "Content-Type": "text/plain"
            }
            readme_html = requests.post(
                'https://api.github.com/markdown/raw', headers=headers, data=readme)
            readme_html = readme_html.text
        except Exception as e:
            logging.debug(
                f"Method: dapp_templates Type: readme_check Error: {e}")

        github_data = get_github_api_link(dapp_id)
        try:
            github_dict = requests.get(github_data)
            github_dict = github_dict.json()
            created_at = get_date(github_dict['created_at'])
            updated_at = get_date(github_dict['updated_at'])
        except Exception as e:
            logging.debug(
                f"Method: dapp_templates Type: repo_data_check Error: {e}")

        context['template'] = json_dict
        context['logo_url'] = logo_string
        context['github_link'] = github_link
        context['download_link'] = download_link
        context['version'] = json_dict['version']
        context['whitelisted_urls'] = json_dict['urls']
        context['plugins'] = json_dict['plugins']
        context['readme'] = readme_html
        context['created_at'] = created_at
        context['updated_at'] = updated_at
        return render(request, 'elastos_trinity/daap_template_application.html', context)
    else:
        context = {}
        template_apps = get_dapp_list()
        json_files = {"angular": [], "react": []}
        for items in template_apps:
            angular_check = items + "angular.json"
            is_angular = False
            try:
                angular_request = requests.get(angular_check)
                if int(angular_request.status_code) == 200:
                    is_angular = True
            except Exception as e:
                logging.debug(
                    f"Method: dapp_templates Type: angular_check Error: {e}")
                continue
            if is_angular:
                request_string = items + "src/assets/manifest.json"
            else:
                request_string = items + "public/assets/manifest.json"
            try:
                manifest = requests.get(request_string)
                json_dict = manifest.json()
                dapp_url = get_dapp_link(json_dict['id'])
                logo_location = json_dict['icons'][1].get('src')
                if is_angular:
                    logo_string = dapp_url + "src/" + logo_location
                else:
                    logo_string = dapp_url + "public/" + logo_location
                json_dict['logo_url'] = logo_string
                if is_angular:
                    json_files["angular"].append(json_dict)
                else:
                    json_files["react"].append(json_dict)
            except Exception as e:
                logging.debug(
                    f"Method: dapp_templates Type: manifest_check Error: {e}")
                continue

        context['templates'] = json_files
        context['recent_services'] = get_recent_services(did)
        return render(request, 'elastos_trinity/dapp_template.html', context)


def get_date(datetime):
    datetime_list = datetime.split('T')
    return datetime_list[0]
