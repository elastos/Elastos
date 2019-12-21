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

def get_time():
    return datetime.datetime.now(pytz.timezone('America/New_York')).strftime("%Y-%m-%d %H:%M:%S %z")

def get_encryption_salt():
	return os.urandom(16)

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

