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

ns = api.namespace('1/service/sidechain/did', description='Has did services')

@ns.route('/createDid')
class CreateDid(Resource):

    def get(self):
        """
        Returns the DID created
        """
        api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_GEN_DID
        myResponse = requests.get(api_url_base).json()
        response = jsonify(myResponse)
        if(response.status_code == 200):
            return response
        else:
            return response.status_code

@ns.route('/setDidInfo', methods = ['POST'])
class SetDidInfo(Resource):

    def post(self):
        """
        Set DID information
        """
        api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SET_DID_INFO
        headers = {'Content-type': 'application/json'}
        req_data = request.get_json()
        json_data = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return json_data

@ns.route('/sign', methods = ['POST'])
class Sign(Resource):

    def post(self):
        """
        Sign any message using your private key
        """
        api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SIGN
        headers = {'Content-type': 'application/json'}
        req_data = request.get_json()
        json_data = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return json_data

@ns.route('/verify', methods = ['POST'])
class Verify(Resource):

    def post(self):
        """
        Verify the message that was signed using your Private Key
        """
        api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_VERIFY
        headers = {'Content-type': 'application/json'}
        req_data = request.get_json()
        json_data = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return json_data


  
