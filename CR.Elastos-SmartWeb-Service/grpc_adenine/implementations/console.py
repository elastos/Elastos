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

class Console(adenine_io_pb2_grpc.AdenineIoServicer):

        def UploadAndSign(self, request, context):

                #Validate the API Key
                api_key = request.api_key
                #api_status = validate_api_key(api_key)
                #if not api_status:
                #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)
                
                #reading the file content
                request_input = json.loads(request.input)
                file_contents = request_input['file']

                #upload file to hive
                api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.HIVE_ADD
                headers = {'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'}
                myResponse1 = requests.get(api_url_base, files={'file':file_contents}, headers=headers).json()

                status_message = ''
                status = ''
                if not myResponse1:
                        status_message = 'Error: File could not be uploaded'
                        status = False
                        return adenine_io_pb2.Response(output="", status_message=status_message, status=status)

                #signing the hash key
                private_key = request_input['private_key']
                did_api_url = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_SIGN
                headers = {'Content-type': 'application/json'}
                req_data =      {
                                        "privateKey":private_key,
                                        "msg":myResponse1['Hash']
                                }
                myResponse2 = requests.post(did_api_url, data=json.dumps(req_data), headers=headers).json()
                myResponse2['result']['hash'] = myResponse1['Hash']

                if myResponse2['status'] == 200:
                        status_message = 'Success'
                        status = True
                else:
                        status_message = 'Error'
                        status = False

                return adenine_io_pb2.Response(output=json.dumps(myResponse2), status_message=status_message, status=status)

        def VerifyAndShow(self, request, context):

                #Validate the API Key
                api_key = request.api_key
                #api_status = validate_api_key(api_key)
                #if not api_status:
                #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)
                
                #verify the hash key
                api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_VERIFY
                headers = {'Content-type': 'application/json'}
                request_input = json.loads(request.input)
                
                signed_message = request_input['msg']
                file_hash = request_input['hash']
                
                json_data = {
                                "msg": request_input['msg'],
                                "pub": request_input['pub'],
                                "sig": request_input['sig']
                        }
                myResponse1 = requests.post(api_url_base, data=json.dumps(json_data), headers=headers).json()
                if myResponse1['result'] != True:
                        return adenine_io_pb2.Response(output="", status_message='Hask key could not be verified', status=False)

                #verify the given input message using private key
                api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_SIGN
                headers = {'Content-type': 'application/json'}
                req_data = {
                                "privateKey":request_input['private_key'],
                                "msg":request_input['hash']
                        }
                myResponse2 = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()
                
                if myResponse2['status'] != 200:
                        return adenine_io_pb2.Response(output="", status_message='Hash Key and message could not be verified', status=False)

                if myResponse2['result']['msg'] != signed_message:
                        return adenine_io_pb2.Response(output="", status_message='Hash Key and message could not be verified', status=False)

                #show content
                api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.SHOW_CONTENT + "{}"
                myResponse = requests.get(api_url_base.format(file_hash))
                return adenine_io_pb2.Response(output=myResponse.text, status_message='Success', status=True)

