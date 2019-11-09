import random
import string
import json
import requests
import grpc
from grpc_adenine import settings
from grpc_adenine.database import (connection as db)
from grpc_adenine.stubs import did_pb2
from grpc_adenine.stubs import did_pb2_grpc
from grpc_adenine.database.user_api_relation import UserApiRelation
from grpc_adenine.implementations.utilities import validate_api_key

class Did(did_pb2_grpc.DidServicer):

	def Sign(self, request, context):

		#Validate the API Key
		api_key = request.api_key
		api_status = validate_api_key(api_key)
		if not api_status:
			return did_pb2.ApiResponse(message='', pub_key='', 
        							sig='', status_message='API Key could not be verified', 
        							status=401)

		#Signing a message
		api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SIGN
        headers = {'Content-type': 'application/json'}
        req_data = 	{
        				"privateKey": request.private_key,
        				"msg": request.message
        			} 
        myResponse = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
        return did_pb2.ApiResponse(message=myResponse['msg'], pub_key=myResponse['pub'], 
        							sig=myResponse['sig'], status_message='Success', 
        							status=myResponse['status'])
