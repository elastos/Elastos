import json
from decouple import config
from requests import Session

from grpc_adenine import settings
from grpc_adenine.stubs import hive_pb2
from grpc_adenine.stubs import hive_pb2_grpc
from grpc_adenine.implementations.rate_limiter import RateLimiter


class Hive(hive_pb2_grpc.HiveServicer):

    def __init__(self):
        headers_general = {
            'Accepts': 'application/json',
            'Content-Type': 'application/json'
        }
        headers_hive = {'Content-Disposition': 'multipart/form-data;boundary'
                                               '=--------------------------608819652137318562927303'}
        self.session = Session()
        self.headers = {
            "general": headers_general,
            "hive": headers_hive
        }

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
        response = self.session.post(did_api_url, data=request.input, headers=self.headers['general'])
        data = json.loads(response.text)

        if data['status'] == 200:
            status_message = 'Success'
            status = True
        else:
            status_message = 'Error'
            status = False

        return hive_pb2.Response(output=json.dumps(data['result']), status_message=status_message,
                                 status=status)

    def UploadAndSign(self, request, context):

        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        #rate limiter
        service_name = 'Sign'
        rate_limiter = RateLimiter()
        result = rate_limiter.get_last_access_count(request.api_key, service_name)

        if result is not False:
            if(result["diff"]<86400):
                if(settings.UPLOAD_AND_SIGN_LIMIT>result["access_count"]):
                    rate_limiter.add_access_count(result["user_api_id"], service_name, 'increment')
                else:
                    return hive_pb2.Response(output="", status_message='Number of daily access limit exceeded', status=False)
            else:
                rate_limiter.add_access_count(result["user_api_id"], service_name, 'reset')
        else:
            rate_limiter.add_new_access_entry(request.api_key, service_name)

        # reading the file content
        request_input = json.loads(request.input)
        file_contents = request_input['file']

        # upload file to hive
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.HIVE_API_ADD_FILE
        response = self.session.get(api_url_base, files={'file': file_contents}, headers=self.headers['hive'])
        data = json.loads(response.text)
        file_hash = data['Hash']

        if not data:
            status_message = 'Error: File could not be uploaded'
            status = False
            return hive_pb2.Response(output="", status_message=status_message, status=status)

        # signing the hash key
        private_key = request_input['private_key']
        did_api_url = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
        req_data = {
            "privateKey": private_key,
            "msg": file_hash
        }
        response = self.session.post(did_api_url, data=json.dumps(req_data), headers=self.headers['general'])
        data = json.loads(response.text)
        data['result']['hash'] = file_hash

        if data['status'] == 200:
            status_message = 'Success'
            status = True
        else:
            status_message = 'Error'
            status = False

        return hive_pb2.Response(output=json.dumps(data), status_message=status_message, status=status)

    def VerifyAndShow(self, request, context):

        # Validate the API Key
        api_key = request.api_key
        # api_status = validate_api_key(api_key)
        # if not api_status:
        #       return adenine_io_pb2.Response(output='', status_message='API Key could not be verified', status=False)

        # verify the hash key
        request_input = json.loads(request.input)

        signed_message = request_input['msg']
        json_data = {
            "msg": signed_message,
            "pub": request_input['pub'],
            "sig": request_input['sig']
        }
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_VERIFY
        response = self.session.post(api_url_base, data=json.dumps(json_data), headers=self.headers['general'])
        data = json.loads(response.text)
        if not data['result']:
            return hive_pb2.Response(output="", status_message='Hash key could not be verified', status=False)

        # verify the given input message using private key
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
        req_data = {
            "privateKey": request_input['private_key'],
            "msg": request_input['hash']
        }
        response = self.session.post(api_url_base, data=json.dumps(req_data), headers=self.headers['general'])
        data = json.loads(response.text)
        if data['status'] != 200:
            return hive_pb2.Response(output="", status_message='Hash Key and message could not be verified',
                                     status=False)

        if data['result']['msg'] != signed_message:
            return hive_pb2.Response(output="", status_message='Hash Key and message could not be verified',
                                     status=False)

        # show content
        api_url_base = config('PRIVATE_NET_IP_ADDRESS') + config('HIVE_PORT') + settings.HIVE_API_RETRIEVE_FILE + "{}"
        response = self.session.get(api_url_base.format(request_input['hash']))
        data = response.text
        return hive_pb2.Response(output=data, status_message='Success', status=True)
