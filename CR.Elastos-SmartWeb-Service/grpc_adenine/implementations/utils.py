from sqlalchemy.orm import sessionmaker

from grpc_adenine.database import db_engine
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.database.user import Users


session_maker = sessionmaker(bind=db_engine)


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


