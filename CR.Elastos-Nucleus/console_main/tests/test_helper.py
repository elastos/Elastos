import json
import secrets
from decouple import config
from datetime import time
import jwt


def get_elastos_fields():
    fields = ['appid', 'iss', 'iat', 'exp', 'callbackurl', 'claims']
    return fields


def get_elephant_url_fields():
    fields = ['CallbackUrl', 'Description', 'AppID', 'DID', 'RandomNumber', 'AppName', 'RequestInfo']
    return fields


def get_elastos_jwt_token(random_num):
    req = {
        "appid": str(random_num)
    }
    req = jwt.encode(req, config('SECRET_KEY'), algorithm='HS256').decode('utf-8')
    json_dict = {'did': 'did:elastos:callbacktestdid', 'presentation': {'type': 'VerifiablePresentation',
                                                                        'proof': {'type': 'ECDSAsecp256r1',
                                                                                  'verificationMethod': 'did:elastos:callbacktestdid#primary',
                                                                                  'realm': 'no-realm',
                                                                                  'nonce': 'no-nonce',
                                                                                  'signature': 'Bu8-FkGqECqBwZeYdl4ChjgBLzH1Ad6qrb0Dwz0aF-OCOerxAbuEXyY35GyCDKyyptTzKpnRYK6hKsr2eG127w'},
                                                                        'verifiableCredential': [{
                                                                            'id': 'did:elastos:callbacktestdid#email',
                                                                            'clazz': 5,
                                                                            'type': [
                                                                                'BasicProfileCredential',
                                                                                'SelfProclaimedCredential'],
                                                                            'issuer': 'did:elastos:callbacktestdid',
                                                                            'issuanceDate': '2020-04-29T11:39:20.000Z',
                                                                            'expirationDate': '2025-04-28T11:39:20.000Z',
                                                                            'credentialSubject': {
                                                                                'id': 'did:elastos:callbacktestdid',
                                                                                'email': 'callbacktest@nucleusconsole.com'},
                                                                            'proof': {
                                                                                'type': 'ECDSAsecp256r1',
                                                                                'verificationMethod': 'did:elastos:callbackdid#primary',
                                                                                'signature': 'xvOx2sO80Gm2svCzvjTNLqIabXt3Pb1yNl7dqVLEkOUvOe3tDeiYn_ltO9uLyMQcKOqPBxaOX1CwpqJQel433w'}},
                                                                            {
                                                                                'id': 'did:elastos:callbacktestdid#name',
                                                                                'clazz': 5,
                                                                                'type': [
                                                                                    'BasicProfileCredential',
                                                                                    'SelfProclaimedCredential'],
                                                                                'issuer': 'did:elastos:callbacktestdid',
                                                                                'issuanceDate': '2020-04-27T19:21:35.000Z',
                                                                                'expirationDate': '2025-04-26T19:21:34.000Z',
                                                                                'credentialSubject': {
                                                                                    'id': 'did:elastos:callbacktestdid',
                                                                                    'name': 'callback test user'},
                                                                                'proof': {
                                                                                    'type': 'ECDSAsecp256r1',
                                                                                    'verificationMethod': 'did:elastos:callbacktestdid#primary',
                                                                                    'signature': 'W_X5zulKI86qxE-DSzuky3t-dvJskCmEu6ov-xyCZ0VV0wHikPjtKxcf1zTFOTVc6TUKmoyvyArlIxbNsDSO5g'}}]},
                 'req': req, 'method': 'credaccess', 'iat': 1590243865,
                 'aud': 'did:elastos:iq5zjqhroDsnvnyMNrr3tZWNdHAxuyRedj'}

    jwt_token = jwt.encode(json_dict , config('SECRET_KEY'), algorithm='HS256')
    return jwt_token.decode()
