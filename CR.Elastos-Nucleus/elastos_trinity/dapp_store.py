import json

from decouple import config
from requests import Session


class DAppStore:
    def __init__(self):
        headers = {
            'Accepts': 'application/json',
            'Content-Type': 'application/json'
        }
        self.session = Session()
        self.session.headers.update(headers)
        self.timeout = 30

    def get_apps_list(self):
        url = config('ELASTOS_TRINITY_DAPPSTORE_URL') + "/apps/list"
        response = self.session.get(url, timeout=self.timeout)
        data = json.loads(response.text)
        return data
