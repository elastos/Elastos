import json
import grpc
import jwt
import datetime
from decouple import config
from .stubs import hive_pb2, hive_pb2_grpc
from elastos_adenine.settings import REQUEST_TIMEOUT, TOKEN_EXPIRATION
import base64
from cryptography.fernet import Fernet
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC


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
            file_contents = myfile.read()

        # encoding and encrypting
        key = get_encrypt_key(private_key)
        fernet = Fernet(key)
        encrypted_message = fernet.encrypt(file_contents)

        # generate JWT token
        jwt_info = {
            'network': network,
            'privateKey': private_key,
            'file_content': encrypted_message.decode("utf-8") 
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')


        response = self.stub.UploadAndSign(hive_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT, metadata=[('did', did)])
        
        if response.status:
            output = jwt.decode(response.output, key=api_key, algorithms=['HS256']).get('jwt_info')
            result = {
                'output': json.dumps(output),
                'status_message': response.status_message,
                'status': response.status
            }
            return result
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
            'request_input': request_input
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, api_key, algorithm='HS256')

        response = self.stub.VerifyAndShow(hive_pb2.Request(input=jwt_token), timeout=REQUEST_TIMEOUT, metadata=[('did', did)])

        if response.status:
            jwt_info = jwt.decode(response.output, key=api_key, algorithms=['HS256']).get('jwt_info')
            file_content = jwt_info['file_content']

            #decrypt file content
            key = get_encrypt_key(request_input['privateKey'])
            fernet = Fernet(key)
            decrypted_message = fernet.decrypt(file_content.encode()) 
            
            result = {
                'file_content': decrypted_message,
                'status_message': response.status_message,
                'status': response.status
            }
            return result
        else:
            result = {
                'output': '',
                'status_message': response.status_message,
                'status': response.status
            }
            return result

def get_encrypt_key(key):
    encoded = key.encode()
    salt = config('ENCRYPTION_SALT').encode()
    kdf = PBKDF2HMAC(
        algorithm=hashes.SHA256(),
        length=32,
        salt=salt,
        iterations=100000,
        backend=default_backend()
    )
    key = base64.urlsafe_b64encode(kdf.derive(encoded))
    return key
