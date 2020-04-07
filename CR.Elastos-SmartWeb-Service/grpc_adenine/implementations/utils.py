import datetime
import pytz
from requests import Session
from sqlalchemy.orm import sessionmaker

from grpc_adenine.database import connection, db_engine
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.database.user import Users
from sqlalchemy.sql import exists

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


