import datetime
import pytz
from grpc_adenine.database import (connection as db)
from grpc_adenine.database.user_api_relation import UserApiRelations
from sqlalchemy.sql import exists
from cryptography.fernet import Fernet


def validate_api_key(api_key):
    result = db.query(exists().where(UserApiRelations.api_key == api_key)).scalar()
    return result

def get_time():
    return datetime.datetime.now(pytz.timezone('America/New_York')).strftime("%Y-%m-%d %H:%M:%S %z")

def get_encrypt_key():
    return Fernet.generate_key()

