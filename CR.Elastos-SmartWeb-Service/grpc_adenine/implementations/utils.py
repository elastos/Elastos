import datetime
import json
import logging

import pytz
from requests import Session
from sqlalchemy.orm import sessionmaker

from grpc_adenine import settings
from grpc_adenine.database import connection, db_engine
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.database.user import Users
from sqlalchemy.sql import exists
from decouple import config
import base64
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC

from grpc_adenine.settings import REQUEST_TIMEOUT

headers = {
    'Accepts': 'application/json',
    'Content-Type': 'application/json'
}
session = Session()
session.headers.update(headers)
session_maker = sessionmaker(bind=db_engine)


def validate_api_key(api_key):
    result = connection.query(exists().where(UserApiRelations.api_key == api_key)).scalar()
    return result


def get_time():
    return datetime.datetime.now(pytz.timezone('America/New_York')).strftime("%Y-%m-%d %H:%M:%S %z")


def get_did_from_api(api_key):
    did = None
    db_session = session_maker()
    api_key_data = db_session.query(UserApiRelations).filter_by(api_key=api_key).first()
    if api_key_data is not None:
        result = db_session.query(Users).filter_by(id=api_key_data.user_id).first()
        did = result.did
    db_session.close()
    return did


def get_api_from_did(did):
    api_key = None
    db_session = session_maker()
    user_data = db_session.query(Users).filter_by(did=did).first()
    if user_data is not None:
        result = db_session.query(UserApiRelations).filter_by(user_id=user_data.id).first()
        api_key = result.api_key
    db_session.close()
    return api_key


def get_info_from_mnemonics(mnemonic):
    private_key, did = None, None
    retrieve_wallet_url = config(
        'PRIVATE_NET_DID_SERVICE_URL') + settings.DID_SERVICE_RETRIEVE_WALLET_FROM_MNEMONIC
    req_data = {
        "mnemonic": mnemonic,
        "index": 1
    }
    try:
        response = session.post(retrieve_wallet_url, data=json.dumps(req_data), timeout=REQUEST_TIMEOUT)
        data = json.loads(response.text)['result']
        private_key = data['privateKey']
        did = data['did']
    except Exception as e:
        logging.debug(f'Error while retrieving private key from mnemonics: {e}')
    return private_key, did


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
