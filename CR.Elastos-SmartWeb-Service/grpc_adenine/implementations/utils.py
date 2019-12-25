import datetime
import pytz
from grpc_adenine.database import (connection as db)
from grpc_adenine.database.user_api_relation import UserApiRelations
from sqlalchemy.sql import exists
from decouple import config
import base64
import os
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC


def validate_api_key(api_key):
    result = db.query(exists().where(UserApiRelations.api_key == api_key)).scalar()
    return result


def check_rate_limit(rate_limiter, limit, api_key, service_name):
    response = {}
    result = rate_limiter.get_last_access_count(api_key, service_name)
    if result:
        if result["diff"] < 86400:
            if limit > result["access_count"]:
                rate_limiter.add_access_count(result["user_api_id"], service_name, 'increment')
            else:
                response = {
                    'result': {
                        'API': service_name,
                        'daily_limit': limit,
                        'num_requests': result['access_count']
                    }
                }
                return response
        else:
            rate_limiter.add_access_count(result["user_api_id"], service_name, 'reset')
    else:
        rate_limiter.add_new_access_entry(api_key, service_name)
    return response


def get_time():
    return datetime.datetime.now(pytz.timezone('America/New_York')).strftime("%Y-%m-%d %H:%M:%S %z")


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
