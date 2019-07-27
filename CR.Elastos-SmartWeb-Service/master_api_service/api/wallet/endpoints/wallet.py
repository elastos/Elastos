import logging
import requests
import json
from flask import jsonify
from flask import request
from flask_api import FlaskAPI, status, exceptions
from flask_restplus import Resource
from master_api_service import settings
from master_api_service.api.restplus import api

log = logging.getLogger(__name__)

ns = api.namespace('wallet/service', description='Has wallet services')

@ns.route('/createWallet')
class Wallet(Resource):

    def get(self):
        """
        Returns the wallet created
        """
        api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_CREATE
        json_data = requests.get(api_url_base).json()
        print("--->status "+json_data.status_code)
        return json_data

@ns.route('/getBalance')
class Wallet(Resource):

    def get(self):
        """
        Returns the balance of the provided public address
        """
        api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_BALANCE
        json_data = requests.get(api_url_base).json()
        return json_data

@ns.route('/getDPOSVote', methods = ['POST'])
class Wallet(Resource):

    def post(self):
        """
        Uses private key to vote your producers
        """
        api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_DPOS_VOTE
        headers = {'Content-type': 'application/json'}
        req_data = request.get_json()
        json_data = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return json_data

@ns.route('/getTransactions')
class Wallet(Resource):

    def get(self):
        """
        Get a list of transactions
        """
        api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_TRANSACTIONS
        headers = {'Content-type': 'application/json'}
        req_data = request.get_json()
        json_data = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return json_data

@ns.route('/getTransactionHistory')
class Wallet(Resource):

    def get(self):
        """
        Get transaction history
        """
        api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_TRANSACTION_HISTORY
        json_data = requests.get(api_url_base).json()
        return json_data

@ns.route('/TransferELA', methods = ['POST'])
class Wallet(Resource):

    def post(self):
        """
        Transfer ELA
        """
        api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_TRANSFER
        headers = {'Content-type': 'application/json'}
        req_data = request.get_json()
        json_data = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return json_data

@ns.route('/getMnemonic')
class Wallet(Resource):

    def get(self):
        """
        Generates a mnemonic
        """
        api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_MNEMONIC
        json_data = requests.get(api_url_base).json()
        return json_data

  
