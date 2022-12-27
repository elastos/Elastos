dapp_dict = {
    "tech.tuum.dapptemplatebasicangular": "https://raw.githubusercontent.com/tuum-tech/dapp_template_basic_angular/master/",
    "tech.tuum.dapptemplatebasicreact": "https://raw.githubusercontent.com/tuum-tech/dapp_template_basic_react/master/"
}


def get_dapp_list():
    dapp_list = []
    for key, val in dapp_dict.items():
        dapp_list.append(val)
    return dapp_list


def get_dapp_link(dapp_id):
    return dapp_dict.get(dapp_id)


def get_github_link(dapp_id):
    github_base = 'https://github.com/'
    raw_list = get_dapp_link(dapp_id)
    raw_list = raw_list.split('/')
    raw_list = raw_list[3:6]
    raw_list = raw_list[:-1]
    for i in range(0, len(raw_list)):
        raw_list[i] = raw_list[i] + "/"
    for i in raw_list:
        github_base += i
    return github_base


def get_github_api_link(dapp_id):
    github_api = "https://api.github.com/repos/"
    raw_list = get_dapp_link(dapp_id).split('/')
    raw_list = raw_list[3:5]
    for i in range(0, len(raw_list) - 1):
        raw_list[i] = raw_list[i] + "/"
    for i in raw_list:
        github_api += i
    return github_api


def get_download_link(dapp_id):
    github_base = 'https://github.com/'
    raw_list = get_dapp_link(dapp_id)
    raw_list = raw_list.split('/')
    branch = raw_list[5]
    raw_list = raw_list[3:6]
    raw_list = raw_list[:-1]
    for i in range(0, len(raw_list)):
        raw_list[i] = raw_list[i] + "/"
    for i in raw_list:
        github_base += i
    github_base += "archive/" + branch + ".zip"
    return github_base


def add_template_app(dapp_id, url):
    dapp_dict[dapp_id] = url
