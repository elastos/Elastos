import json
import requests
from decouple import config

from web3 import Web3, HTTPProvider
from web3.middleware import geth_poa_middleware
from solc import compile_standard

from grpc_adenine import settings
from grpc_adenine.stubs import hive_pb2
from grpc_adenine.stubs import hive_pb2_grpc


class Hive(hive_pb2_grpc.HiveServicer):

    def Sign(self, request, context):

        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        PRIVATE_NET_IP_ADDRESS = config('PRIVATE_NET_IP_ADDRESS')
        DID_SERVICE_URL = config('DID_SERVICE_URL')
        did_api_url = PRIVATE_NET_IP_ADDRESS + DID_SERVICE_URL + settings.DID_SERVICE_API_SIGN

        # Signing a message
        headers = {'Content-type': 'application/json'}
        myResponse = requests.post(did_api_url, data=request.input, headers=headers).json()

        if myResponse['status'] == 200:
            status_message = 'Success'
            status = True
        else:
            status_message = 'Error'
            status = False

        return hive_pb2.Response(output=json.dumps(myResponse['result']), status_message=status_message,
                                 status=status)

    def UploadAndSign(self, request, context):

        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        # reading the file content
        request_input = json.loads(request.input)
        file_contents = request_input['file']

        # upload file to hive
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.HIVE_API_ADD_FILE
        headers = {
            'Content-Disposition': 'multipart/form-data;boundary=--------------------------608819652137318562927303'}
        response = requests.get(api_url_base, files={'file': file_contents}, headers=headers).json()
        file_hash = response['Hash']

        if not response:
            status_message = 'Error: File could not be uploaded'
            status = False
            return hive_pb2.Response(output="", status_message=status_message, status=status)

        # signing the hash key
        private_key = request_input['private_key']
        did_api_url = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
        headers = {'Content-type': 'application/json'}
        req_data = {
            "privateKey": private_key,
            "msg": response['Hash']
        }
        response = requests.post(did_api_url, data=json.dumps(req_data), headers=headers).json()
        response['result']['hash'] = file_hash

        if response['status'] == 200:
            status_message = 'Success'
            status = True
        else:
            status_message = 'Error'
            status = False

        return hive_pb2.Response(output=json.dumps(response), status_message=status_message, status=status)

    def VerifyAndShow(self, request, context):

        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        # verify the hash key
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_VERIFY
        headers = {'Content-type': 'application/json'}
        request_input = json.loads(request.input)

        signed_message = request_input['msg']

        json_data = {
            "msg": signed_message,
            "pub": request_input['pub'],
            "sig": request_input['sig']
        }
        response = requests.post(api_url_base, data=json.dumps(json_data), headers=headers).json()
        if not response['result']:
            return hive_pb2.Response(output="", status_message='Hash key could not be verified', status=False)

        # verify the given input message using private key
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
        headers = {'Content-type': 'application/json'}
        req_data = {
            "privateKey": request_input['private_key'],
            "msg": request_input['hash']
        }
        response = requests.post(api_url_base, data=json.dumps(req_data), headers=headers).json()

        if response['status'] != 200:
            return hive_pb2.Response(output="", status_message='Hash Key and message could not be verified',
                                     status=False)

        if response['result']['msg'] != signed_message:
            return hive_pb2.Response(output="", status_message='Hash Key and message could not be verified',
                                     status=False)

        # show content
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.HIVE_API_RETRIEVE_FILE + "{}"
        myResponse = requests.get(api_url_base.format(request_input['hash']))
        return hive_pb2.Response(output=myResponse.text, status_message='Success', status=True)
