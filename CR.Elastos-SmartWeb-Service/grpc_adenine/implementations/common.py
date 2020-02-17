import random
import string
import datetime

from decouple import config
from grpc_adenine.database import db_engine
from grpc_adenine.database.user import Users
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc
from sqlalchemy.orm import sessionmaker
from grpc_adenine import settings
from grpc_adenine.implementations.utils import check_rate_limit, get_info_from_mnemonics
from grpc_adenine.implementations.rate_limiter import RateLimiter


class Common(common_pb2_grpc.CommonServicer):

    def __init__(self):
        session_maker = sessionmaker(bind=db_engine)
        self.session = session_maker()
        self.rate_limiter = RateLimiter(self.session)

    def GenerateAPIRequestMnemonic(self, request, context):
        mnemonic = request.mnemonic

        private_key, did = get_info_from_mnemonics(mnemonic)
        if not (private_key and did):
            return common_pb2.Response(api_key='', status_message='Invalid Mnemonics Error', status=False)

        response = generate_api_key(self.session, self.rate_limiter, did)
        return response

    def GenerateAPIRequest(self, request, context):
        secret_key = config('SHARED_SECRET_ADENINE')
        did = request.did

        if secret_key != request.secret_key:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)

        response = generate_api_key(self.session, self.rate_limiter, did)
        return response

    def GetAPIKeyMnemonic(self, request, context):
        mnemonic = request.mnemonic

        private_key, did = get_info_from_mnemonics(mnemonic)
        if not (private_key and did):
            return common_pb2.Response(api_key='', status_message='Invalid Mnemonics Error', status=False)

        response = get_api_key(self.session, did)
        return response

    def GetAPIKey(self, request, context):
        secret_key = config('SHARED_SECRET_ADENINE')
        did = request.did

        if secret_key != request.secret_key:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)

        response = get_api_key(self.session, did)
        return response


def get_api_key(session, did):
    result = session.query(Users).filter_by(did=did).first()
    if result:
        api_present = session.query(UserApiRelations).filter_by(user_id=result.id).first()
        api_key = api_present.api_key
        return common_pb2.Response(api_key=api_key, status_message='Success', status=True)
    else:
        return common_pb2.Response(api_key='', status_message='No such API Key exists', status=False)


def generate_api_key(session, rate_limiter, did):
    string_length = 64
    date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")

    api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))
    api_present = session.query(UserApiRelations).filter_by(api_key=api_key).first()

    # Generate a new api key if its already present
    if api_present:
        while api_present.api_key == api_key:
            api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))

    result = session.query(Users).filter_by(did=did).first()

    # If did is already present replace the api key
    if result:
        insert = session.query(UserApiRelations).filter_by(user_id=result.id).first()
        # check rate limit
        response = check_rate_limit(rate_limiter, settings.GENERATE_API_LIMIT, insert.api_key,
                                    generate_api_key.__name__)
        if response:
            return common_pb2.Response(api_key='', status_message='Number of daily access limit exceeded',
                                       status=False)

        # replace the new API Key
        insert.api_key = api_key
        session.commit()
    # Else insert the did and api key into respective tables
    else:
        # insert to Users table
        user = Users(
            did=did,
            created_on=date_now,
            last_logged_on=date_now
        )
        session.add(user)
        # insert to UserApiRelations table
        insert = session.query(Users).filter_by(did=did).first()
        user_api = UserApiRelations(
            user_id=insert.id,
            api_key=api_key
        )
        session.add(user_api)
        session.commit()
        # insert into SERVICES LISTS table
        rate_limiter.add_new_access_entry(api_key, generate_api_key.__name__)
    return common_pb2.Response(api_key=api_key, status_message='Success', status=True)
