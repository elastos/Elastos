import logging
import requests
import random
import string
import datetime
import json
from flask_api import status
from flask import request
from flask import Response
from flask_restplus import Resource
from smartweb_service import settings
from smartweb_service.api.restplus import api
from smartweb_service.api.common.common_service import validate_api_key
from smartweb_service.api.common.common_service import getTime

log = logging.getLogger(__name__)

ns = api.namespace('1/hive', description='Implements hive storage services')

@ns.route('/add')
class Add(Resource):

	def post(self):
		"""
		Returns Hash key of the content added.
		"""
		api_key = request.headers.get('api_key')
		api_status = validate_api_key(api_key)
		if not api_status:
			data = {"error message":"API Key could not be verified","status":401, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=401,
				mimetype='application/json'
			)

		api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.HIVE_ADD
		headers = {'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'}
		req_data = request.form.to_dict()
		myResponse = requests.get(api_url_base, files=req_data, headers=headers).json()
		return Response(json.dumps(myResponse), 
				status=200,
        		mimetype='application/json'
        	)

@ns.route('/showContent/<string:hash_key>')
class ShowContent(Resource):

	def get(self, hash_key):
		"""
		Returns content of the hash key.
		"""
		api_key = request.headers.get('api_key')
		api_status = validate_api_key(api_key)
		if not api_status:
			data = {"error message":"API Key could not be verified","status":401, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=401,
				mimetype='application/json'
			)

		api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.SHOW_CONTENT + "{}"
		myResponse = requests.get(api_url_base.format(hash_key))
		return Response(myResponse, 
			status=myResponse.status_code,
			mimetype='application/json'
		)


  
