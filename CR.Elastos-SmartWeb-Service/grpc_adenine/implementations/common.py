import random
import string
import datetime
import jwt
from decouple import config
from grpc_adenine.database import db_engine
from grpc_adenine.database.user import Users
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.stubs.python import common_pb2, common_pb2_grpc
from sqlalchemy.orm import sessionmaker
from grpc_adenine import settings
from grpc_adenine.implementations.utils import get_info_from_mnemonics
from grpc_adenine.implementations.rate_limiter import RateLimiter


class Common(common_pb2_grpc.CommonServicer):

    def __init__(self):
        session_maker = sessionmaker(bind=db_engine)
        self.session = session_maker()
        self.rate_limiter = RateLimiter(self.session)

    def GenerateAPIRequestMnemonic(self, request, context):
        metadata = dict(context.invocation_metadata())
        input_did = metadata["did"]
        secret_key = config('SHARED_SECRET_ADENINE')
        try:
            jwt_info = jwt.decode(request.input, key=secret_key, algorithms=['HS256']).get('jwt_info')
        except:
            return common_pb2.Response(output='', status_message='Authentication Error', status=False)
        
        mnemonic = jwt_info['mnemonic']

        private_key, did = get_info_from_mnemonics(mnemonic)
        if not (private_key and did):
            return common_pb2.Response(output='', status_message='Invalid Mnemonics Error', status=False)

        #if (input_did != did):
        #    return common_pb2.Response(output='', status_message='Invalid DID', status=False)

        response = generate_api_key(self.session, self.rate_limiter, did)
        return response

    def GenerateAPIRequest(self, request, context):
        metadata = dict(context.invocation_metadata())
        input_did = metadata["did"]
        secret_key = config('SHARED_SECRET_ADENINE')
        try:
            jwt_info = jwt.decode(request.input, key=secret_key, algorithms=['HS256']).get('jwt_info')
        except:
            return common_pb2.Response(output='', status_message='Authentication Error', status=False)

        response = generate_api_key(self.session, self.rate_limiter, input_did)
        return response

    def GetAPIKeyMnemonic(self, request, context):
        metadata = dict(context.invocation_metadata())
        input_did = metadata["did"]
        secret_key = config('SHARED_SECRET_ADENINE')
        try:
            jwt_info = jwt.decode(request.input, key=secret_key, algorithms=['HS256']).get('jwt_info')
        except:
            return common_pb2.Response(output='', status_message='Authentication Error', status=False)

        mnemonic = jwt_info['mnemonic']

        private_key, did = get_info_from_mnemonics(mnemonic)
        if not (private_key and did):
            return common_pb2.Response(output='', status_message='Invalid Mnemonics Error', status=False)

        #if (input_did != did):
        #    return common_pb2.Response(output='', status_message='Invalid DID', status=False)

        response = get_api_key(self.session, did)
        return response

    def GetAPIKey(self, request, context):
        metadata = dict(context.invocation_metadata())
        input_did = metadata["did"]
        secret_key = config('SHARED_SECRET_ADENINE')
        try:
            jwt_info = jwt.decode(request.input, key=secret_key, algorithms=['HS256']).get('jwt_info')
        except:
            return common_pb2.Response(output='', status_message='Authentication Error', status=False)

        response = get_api_key(self.session, input_did)
        return response

TOKEN_EXPIRATION = 24 * 30

def get_api_key(session, did):
    secret_key = config('SHARED_SECRET_ADENINE')
    global TOKEN_EXPIRATION
    result = session.query(Users).filter_by(did=did).first()
    if result:
        api_present = session.query(UserApiRelations).filter_by(user_id=result.id).first()
        api_key = api_present.api_key

        #generate jwt token
        jwt_info = {
            'api_key': api_key
        }

        jwt_token = jwt.encode({
            'jwt_info': jwt_info,
            'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
        }, secret_key, algorithm='HS256')

        return common_pb2.Response(output=jwt_token, status_message='Success', status=True)
    else:
        return common_pb2.Response(output='', status_message='No such API Key exists', status=False)

def generate_api_key(session, rate_limiter, did):
    string_length = 64
    date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")
    secret_key = config('SHARED_SECRET_ADENINE')
    global TOKEN_EXPIRATION

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
        response = rate_limiter.check_rate_limit(settings.GENERATE_API_LIMIT, insert.api_key,
                                    generate_api_key.__name__)
        if response:
            return common_pb2.Response(output='', status_message='Number of daily access limit exceeded',
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

    #generate jwt token
    jwt_info = {
        'api_key': api_key
    }

    jwt_token = jwt.encode({
        'jwt_info': jwt_info,
        'exp': datetime.datetime.utcnow() + datetime.timedelta(hours=TOKEN_EXPIRATION)
    }, secret_key, algorithm='HS256')

    return common_pb2.Response(output=jwt_token, status_message='Success', status=True)
