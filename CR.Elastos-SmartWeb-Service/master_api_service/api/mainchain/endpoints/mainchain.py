import logging
import requests
from requests import Request, Session
import json
from flask import jsonify
from flask import request
from flask_api import FlaskAPI, status, exceptions
from flask_restplus import Resource
from master_api_service import settings
from master_api_service.api.restplus import api

log = logging.getLogger(__name__)

ns = api.namespace('1/mainchain', description='Has mainchain node services')

@ns.route('/getblockhash', methods = ['POST'])
class GetBlockHash(Resource):

    def post(self):
        """
        Returns the getblockhash
        """
        req_data = request.get_json()
        headers = {
           'Accepts': 'application/json',
           'Content-Type': 'application/json'
        }
        session = Session()
        session.headers.update(headers)
        URL_BLOCKCONFIRM = settings.MAINCHAIN_RPC_URL
        d = {"method": "getblockhash", "params": req_data}
        response = session.post(URL_BLOCKCONFIRM, data=json.dumps(d))
        data = json.loads(response.text)
        return data

@ns.route('/getbestblockhash')
class GetBestBlockHash(Resource):

    def get(self):
        """
        Returns the getbestblockhash
        """
        headers = {
           'Accepts': 'application/json',
           'Content-Type': 'application/json'
        }
        session = Session()
        session.headers.update(headers)
        URL_BLOCKCONFIRM = settings.MAINCHAIN_RPC_URL
        d = {"method": "getbestblockhash"}
        response = session.post(URL_BLOCKCONFIRM, data=json.dumps(d))
        data = json.loads(response.text)
        return data
