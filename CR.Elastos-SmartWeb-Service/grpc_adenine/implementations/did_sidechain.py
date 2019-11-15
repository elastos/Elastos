import random
import string
import json
import requests
import grpc
from grpc_adenine import settings
from grpc_adenine.database import (connection as db)
from grpc_adenine.stubs import adenine_io_pb2
from grpc_adenine.stubs import adenine_io_pb2_grpc
from grpc_adenine.implementations.utilities import validate_api_key

class Did(adenine_io_pb2_grpc.AdenineIoServicer):

	def Sign(self, request, context):

		#Validate the API Key
                api_key = request.api_key
                api_status = validate_api_key(api_key)
                if not api_status:
                                return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

		#Signing a message
                api_url_base = settings.DID_SERVICE_URL + settings.DID_SERVICE_SIGN
                headers = {'Content-type': 'application/json'}
                
                myResponse = requests.post(api_url_base, data=json.dumps(request.input), headers=headers).json()
                response_data = {
                                        "message": myResponse['result']['msg'], 
                                        "pub_key": myResponse['result']['pub'], 
                                        "sig": myResponse['result']['sig']
                                } 
                return adenine_io_pb2.Response(output=response_data, status_message=myResponse['status'], status=True)
