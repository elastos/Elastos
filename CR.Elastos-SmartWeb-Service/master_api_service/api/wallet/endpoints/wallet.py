import logging
import requests

from flask import request
from flask_restplus import Resource
from master_api_service import settings
from master_api_service.api.restplus import api

log = logging.getLogger(__name__)

ns = api.namespace('wallet', description='Has wallet services')


@ns.route('/createWallet')
class Wallet(Resource):

    def get(self):
        """
        Returns the wallet created.
        """
        api_url_base = "http://localhost:10012/api/v1/asset/balances/EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U"
        json_data = requests.get(api_url_base).json()
        return json_data

@ns.route('/getBalance')
class Wallet(Resource):

    def get(self):
        """
        Returns the wallet created.
        """
        sample_url = settings.WALLET_SERVICE_URL + settings.WALLET_API_BALANCE
        api_url_base = "http://localhost:10012/api/v1/asset/balances/EQ4QhsYRwuBbNBXc8BPW972xA9ANByKt6U"
        json_data = requests.get(api_url_base).json()
        return json_data

  
