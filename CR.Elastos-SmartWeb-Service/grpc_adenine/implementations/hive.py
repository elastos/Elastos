import logging
import sys
import jwt
import datetime
import json
import base64

import requests
from cryptography.fernet import Fernet
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC

from decouple import config

from grpc_adenine import settings
from grpc_adenine.implementations.utils import get_api_from_did
from grpc_adenine.settings import REQUEST_TIMEOUT
from grpc_adenine.stubs.python import hive_pb2, hive_pb2_grpc
from grpc_adenine.implementations.rate_limiter import RateLimiter


class Hive(hive_pb2_grpc.HiveServicer):

    def __init__(self):
        headers_general = {
            'Accepts': 'application/json',
            'Content-Type': 'application/json'
        }
        headers_hive = {'Content-Disposition': 'multipart/form-data;boundary'
                                               '=--------------------------608819652137318562927303'}
        self.headers = {
            "general": headers_general,
            "hive": headers_hive
        }
        self.rate_limiter = RateLimiter()

    def UploadAndSign(self, request, context):

        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        api_key = get_api_from_did(did)

        try:
            jwt_info = jwt.decode(request.input, key=api_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"UploadAndSign : {did} : {api_key} : {status_message} : {e}")
            return hive_pb2.Response(output='', status_message=status_message, status=False)

        # reading input data
        if type(jwt_info) == str:
            # request from go package
            jwt_info = json.loads(jwt_info)

        network = jwt_info['network']
        private_key = jwt_info['privateKey']
        file_content = request.file_content

        # Check whether the user is able to use this API by checking their rate limiter
        response = self.rate_limiter.check_rate_limit(settings.UPLOAD_AND_SIGN_LIMIT, api_key,
                                                      self.UploadAndSign.__name__)
        if response:
            status_message = f'Number of daily access limit exceeded {response["result"]["daily_limit"]}'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return hive_pb2.Response(output=json.dumps(response),
                                     status_message=status_message,
                                     status=False)

        # checking file size
        if sys.getsizeof(file_content) > settings.FILE_UPLOAD_SIZE_LIMIT:
            status_message = "File size limit exceeded"
            logging.debug(f"{did} : {api_key} : {status_message}")
            return hive_pb2.Response(output="", status_message=status_message, status=False)

        if network == "mainnet":
            did_api_url = config('MAIN_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
            hive_api_url = config('MAIN_NET_HIVE_PORT') + settings.HIVE_API_ADD_FILE
        else:
            did_api_url = config('PRIVATE_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
            hive_api_url = config('PRIVATE_NET_HIVE_PORT') + settings.HIVE_API_ADD_FILE

        # encoding and encrypting
        key = get_encrypt_key(private_key)
        fernet = Fernet(key)
        file_content_encrypted = fernet.encrypt(file_content)

        # upload file to hive
        response = requests.get(hive_api_url, files={'file': file_content_encrypted}, headers=self.headers['hive'],
                                    timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)
        file_hash = data['Hash']

        if not data:
            status_message = 'Error: File could not be uploaded'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return hive_pb2.Response(output="", status_message=status_message, status=False)

        # signing the hash key
        req_data = {
            "privateKey": private_key,
            "msg": file_hash
        }
        response = requests.post(did_api_url, data=json.dumps(req_data), headers=self.headers['general'],
                                     timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)
        data['result']['hash'] = file_hash

        if data['status'] == 200:
            status_message = 'Successfully uploaded file to Elastos Hive'
            status = True
        else:
            status_message = 'Error'
            status = False

        del data['status']

        # generate jwt token
        jwt_info = {
            'result': data['result']
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        return hive_pb2.Response(output=jwt_token, status_message=status_message, status=status)

    def VerifyAndShow(self, request, context):

        metadata = dict(context.invocation_metadata())
        did = metadata["did"]
        api_key = get_api_from_did(did)

        try:
            jwt_info = jwt.decode(request.input, key=api_key, algorithms=['HS256']).get('jwt_info')
        except Exception as e:
            status_message = 'Authentication Error'
            logging.debug(f"VerifyAndShow : {did} : {api_key} : {status_message} : {e}")
            return hive_pb2.Response(output='', status_message=status_message, status=False)

        if type(jwt_info) == str:
            jwt_info = json.loads(jwt_info)

        network = jwt_info['network']
        signed_message = jwt_info['msg']
        public_key = jwt_info['pub']
        message_signature = jwt_info['sig']
        message_hash = jwt_info['hash']
        private_key = jwt_info['privateKey']

        # Check whether the user is able to use this API by checking their rate limiter
        response = self.rate_limiter.check_rate_limit(settings.VERIFY_AND_SHOW_LIMIT, api_key,
                                                      self.VerifyAndShow.__name__)
        if response:
            status_message = f'Number of daily access limit exceeded {response["result"]["daily_limit"]}'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return hive_pb2.Response(output=json.dumps(response),
                                     status_message=status_message,
                                     status=False)

        if network == "mainnet":
            did_api_sign_url = config('MAIN_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
            did_api_verify_url = config('MAIN_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_VERIFY
            hive_api_url = config('MAIN_NET_HIVE_PORT') + settings.HIVE_API_RETRIEVE_FILE + "{}"
        else:
            did_api_sign_url = config('PRIVATE_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_SIGN
            did_api_verify_url = config('PRIVATE_NET_DID_SERVICE_URL') + settings.DID_SERVICE_API_VERIFY
            hive_api_url = config('PRIVATE_NET_HIVE_PORT') + settings.HIVE_API_RETRIEVE_FILE + "{}"

        # verify the hash key
        json_data = {
            "msg": signed_message,
            "pub": public_key,
            "sig": message_signature
        }
        response = requests.post(did_api_verify_url, data=json.dumps(json_data), headers=self.headers['general'],
                                     timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)
        if not data['result']:
            return hive_pb2.Response(output="", status_message='Hash key could not be verified', status=False)

        # verify the given input message using private key
        req_data = {
            "privateKey": private_key,
            "msg": message_hash
        }
        response = requests.post(did_api_sign_url, data=json.dumps(req_data), headers=self.headers['general'],
                                     timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)
        if data['status'] != 200:
            status_message = 'Hash Key and message could not be verified'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return hive_pb2.Response(output="", status_message=status_message,
                                     status=False)

        if data['result']['msg'] != signed_message:
            status_message = 'Hash Key and message could not be verified'
            logging.debug(f"{did} : {api_key} : {status_message}")
            return hive_pb2.Response(output="", status_message=status_message,
                                     status=False)

        # show content
        response = requests.get(hive_api_url.format(jwt_info['hash']), headers=self.headers['general'], timeout=REQUEST_TIMEOUT)

        # decrypt message
        key = get_encrypt_key(private_key)
        fernet = Fernet(key)
        decrypted_message = fernet.decrypt(response.text.encode())

        # generate jwt token
        jwt_info = {}

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=settings.TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        return hive_pb2.Response(output=jwt_token, file_content=decrypted_message,
                                 status_message='Successfully retrieved file from Elastos Hive', status=True)


def get_encrypt_key(key):
    encoded = key.encode()
    salt = config('SHARED_SECRET_ADENINE').encode()
    kdf = PBKDF2HMAC(
        algorithm=hashes.SHA256(),
        length=32,
        salt=salt,
        iterations=100000,
        backend=default_backend()
    )
    key = base64.urlsafe_b64encode(kdf.derive(encoded))
    return key
