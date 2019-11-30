import random
import string
import json
import requests
import grpc
from decouple import config

from grpc_adenine import settings
from grpc_adenine.database import (connection as db)
from grpc_adenine.stubs import adenine_io_pb2
from grpc_adenine.stubs import adenine_io_pb2_grpc
from grpc_adenine.implementations.utilities import validate_api_key

class Did(adenine_io_pb2_grpc.AdenineIoServicer):

	def Sign(self, request, context):

		#Validate the API Key
                api_key = request.api_key
                #api_status = validate_api_key(api_key)
                #if not api_status:
                #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

                PRIVATE_NET_IP_ADDRESS = config('PRIVATE_NET_IP_ADDRESS')
                DID_SERVICE_URL = config('DID_SERVICE_URL')
                did_api_url = PRIVATE_NET_IP_ADDRESS + DID_SERVICE_URL + settings.DID_SERVICE_SIGN
                
		#Signing a message
                headers = {'Content-type': 'application/json'}
                myResponse = requests.post(did_api_url, data=request.input, headers=headers).json()
                
                status_message = ''
                status = ''
                if myResponse['status'] == 200:
                        status_message = 'Success'
                        status = True
                else:
                        status_message = 'Error'
                        status = False

                return adenine_io_pb2.Response(output=json.dumps(myResponse['result']), status_message=status_message, status=status)
