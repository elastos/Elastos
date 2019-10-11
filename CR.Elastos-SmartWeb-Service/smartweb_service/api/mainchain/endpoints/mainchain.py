import logging
import requests
from requests import Request, Session
import json
from flask import jsonify
from flask import request
from flask import Response
from flask_api import FlaskAPI, status, exceptions
from flask_restplus import Resource
from smartweb_service import settings
from smartweb_service.api.restplus import api
from smartweb_service.api.common.common_service import validate_api_key
from smartweb_service.api.common.common_service import getTime

log = logging.getLogger(__name__)

ns = api.namespace('1/mainchain', description='Has mainchain node services')

@ns.route('/getblockhash', methods = ['POST'])
class GetBlockHash(Resource):

    def post(self):
        """
        Returns the getblockhash
        """
        api_key = request.headers.get('api_key')
        api_status = validate_api_key(api_key)
        if not api_status:
            data = {"error message":"API Key could not be verified","status":401, "timestamp":getTime(),"path":request.url}
            return Response(json.dumps(data), 
                status=401,
                mimetype='application/json'
            )

        req_data = request.get_json()
        headers = {
           'Accepts': 'application/json',
           'Content-Type': 'application/json'
        }
        session = Session()
        session.headers.update(headers)
        URL_BLOCKCONFIRM = settings.MAINCHAIN_RPC_URL
        d = {"method": "getblockhash", "params": req_data}
        myResponse = session.post(URL_BLOCKCONFIRM, data=json.dumps(d))
        data = json.loads(response.text)
        return Response(json.dumps(data), 
                status=200,
                mimetype='application/json'
            )

@ns.route('/getbestblockhash')
class GetBestBlockHash(Resource):

    def get(self):
        """
        Returns the getbestblockhash
        """
        api_key = request.headers.get('api_key')
        api_status = validate_api_key(api_key)
        if not api_status:
            data = {"error message":"API Key could not be verified","status":401, "timestamp":getTime(),"path":request.url}
            return Response(json.dumps(data), 
                status=401,
                mimetype='application/json'
            )

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
        return Response(json.dumps(data), 
                status=200,
                mimetype='application/json'
            )
