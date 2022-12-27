import json
import grpc
import jwt
import datetime

from .stubs import hive_pb2, hive_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT, TOKEN_EXPIRATION


class Hive:

    def __init__(self, host, port, production):
        if not production:
            self._channel = grpc.insecure_channel('{}:{}'.format(host, port))
        else:
            credentials = grpc.ssl_channel_credentials()
            self._channel = grpc.secure_channel('{}:{}'.format(host, port), credentials)

        self.stub = hive_pb2_grpc.HiveStub(self._channel)

    def close(self):
        self._channel.close()

    def upload_and_sign(self, api_key, did, network, private_key, filename):
        with open(filename, 'rb') as myfile:
            file_content = myfile.read()

        # generate JWT token
        jwt_info = {
            'network': network,
            'privateKey': private_key
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.UploadAndSign(hive_pb2.Request(input=jwt_token, file_content=file_content),
                                           timeout=REQUEST_TIMEOUT, metadata=[('did', did)])

        if response.status:
            output = jwt.decode(response.output, key=api_key, algorithms=['HS256']).get('jwt_info')
            result = {
                'output': json.dumps(output),
                'status_message': response.status_message,
                'status': response.status
            }

        else:
            result = {
                'output': '',
                'status_message': response.status_message,
                'status': response.status
            }
        return result

    def verify_and_show(self, api_key, did, network, request_input):
        jwt_info = {
            'network': network,
            'msg': request_input['msg'],
            'pub': request_input['pub'],
            'sig': request_input['sig'],
            'hash': request_input['hash'],
            'privateKey': request_input['privateKey']
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.VerifyAndShow(hive_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT,
                                           metadata=[('did', did)])

        if response.status:
            result = {
                'file_content': response.file_content,
                'status_message': response.status_message,
                'status': response.status
            }
        else:
            result = {
                'output': '',
                'status_message': response.status_message,
                'status': response.status
            }
        return result



