import logging
import requests
import random
import string
import datetime
import json

from flask import request
from flask import Response
from flask_restplus import Resource
from master_api_service import settings
from master_api_service.api.restplus import api
from master_api_service.api.common.common_service import validate_api_key
from master_api_service.api.common.common_service import getTime

log = logging.getLogger(__name__)

ns = api.namespace('1/console', description='Implements console functionalities')

@ns.route('/upload')
class Upload(Resource):

	def get(self):
		"""
		Upload a file to hive.
		"""
		api_key = request.headers.get('api_key')
		api_status = validate_api_key(api_key)
		if not api_status:
			data = {"error message":"API Key could not be verified","status":401, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=401,
				mimetype='application/json'
			)

		#reading the file content
		request_file = request.files['file']
		if not request_file:
			data = {"error message":"No file attached","status":404, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=404,
				mimetype='application/json'
			)

		file_contents = request_file.stream.read().decode("utf-8")

		#upload file to hive
		api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.HIVE_ADD
		headers = {'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'}
		myResponse = requests.get(api_url_base, files={'file':file_contents}, headers=headers).json()
		return Response(json.dumps(myResponse), 
				status=200,
        		mimetype='application/json'
        	)

@ns.route('/uploadAndSign')
class UploadAndSign(Resource):

	def get(self):
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

		#reading the file content
		request_file = request.files['file']
		if not request_file:
			data = {"error message":"No file attached","status":404, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=404,
				mimetype='application/json'
			)

		file_contents = request_file.stream.read().decode("utf-8")

		#upload file to hive
		api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.HIVE_ADD
		headers = {'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'}
		myResponse1 = requests.get(api_url_base, files={'file':file_contents}, headers=headers).json()
		if not myResponse1:
			data = {"error message":"File could not be uploaded","status":404, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=404,
				mimetype='application/json'
			)


		#signing the hash key
		private_key = request.headers.get('private_key')
		api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SIGN
		headers = {'Content-type': 'application/json'}
		req_data = 	{
      					"privateKey":private_key,
      					"msg":myResponse1['Hash']
  					}
		myResponse2 = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
		myResponse2['result']['hash'] = myResponse1['Hash']
		return Response(json.dumps(myResponse2), 
				status=myResponse2['status'],
				mimetype='application/json'
			)

@ns.route('/verifyAndShow')
class VerifyAndShow(Resource):

	def get(self):
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

		#verify the hash key
		api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_VERIFY
		headers = {'Content-type': 'application/json'}
		req_data = request.get_json()
		signed_message = req_data['msg']
		file_hash = req_data['hash']
		json_data = {
						"msg": req_data['msg'],
						"pub": req_data['pub'],
    					"sig": req_data['sig']
					}
		myResponse1 = requests.post(api_url_base, data=json.dumps(json_data), headers=headers).json()
		if not myResponse1['result']:
			data = {"error message":"Hask key could not be verified","status":404, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=404,
				mimetype='application/json'
			)

		#verify the given input message using private key
		private_key = request.headers.get('private_key')
		api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SIGN
		headers = {'Content-type': 'application/json'}
		req_data = 	{
      					"privateKey":private_key,
      					"msg":req_data['hash']
  					}
		myResponse2 = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
		if myResponse2['result']['msg'] != signed_message:
			data = {"error message":"Hash Key and messsage could not be verified","status":401, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
				status=401,
				mimetype='application/json'
			)

        #show content
		api_url_base = settings.GMU_NET_IP_ADDRESS + settings.HIVE_PORT + settings.SHOW_CONTENT + "{}"
		myResponse = requests.get(api_url_base.format(file_hash))
		return Response(myResponse, 
				status=200,
				mimetype='application/json'
			)

@ns.route('/transferELADemo')
class TransferELADemo(Resource):

	def get(self):
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

		#create a wallet
		api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_CREATE
		myResponse1 = requests.get(api_url_base).json()
		if myResponse1['status'] != 200:
			data = {"error message":"Wallet could not be created","status":404, "timestamp":getTime(),"path":request.url}
			return Response(json.dumps(data), 
					status=404,
					mimetype='application/json'
				)

        #transfer ELA
		api_url_base = settings.WALLET_SERVICE_URL + settings.WALLET_API_TRANSFER
		headers = {'Content-type': 'application/json'}
		req_data = {
				      "sender":[
				          {
				              "address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
				              "privateKey":"109a5fb2b7c7abd0f2fa90b0a295e27de7104e768ab0294a47a1dd25da1f68a8"
				          }
				      ],
				      "memo":"测试",
				      "receiver":[
				          {
				              "address":myResponse1['result']['address'],
				              "amount":"100"
				          }
				      ]
				  }
		myResponse2 = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
		json_output = 	{
							"sender":[
				          	{
				            	"address":"EUSa4vK5BkKXpGE3NoiUt695Z9dWVJ495s",
				            	"transferred_amount":"100"
				          	}
				      		],
				      		"receiver":[
				        	{
				        		"privateKey":myResponse1['result']['privateKey'],
				        		"publicKey":myResponse1['result']['publicKey'],
				            	"address":myResponse1['result']['address']
				          	}
				      		],
				      		"transaction_id": myResponse2['result'],
    						"status": myResponse2['status']
						}
		return Response(json.dumps(json_output), 
				status=myResponse2['status'],
				mimetype='application/json'
			)


  
