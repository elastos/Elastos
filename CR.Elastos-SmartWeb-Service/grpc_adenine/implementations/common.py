import random
import string
import datetime
from decouple import config
from sqlalchemy import create_engine
from grpc_adenine.database.user import Users
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.stubs import common_pb2
from grpc_adenine.stubs import common_pb2_grpc
from sqlalchemy.orm import sessionmaker

class Common(common_pb2_grpc.CommonServicer):

    def GenerateAPIRequest(self, request, context):
            stringLength = 64
            secret_key = config('SHARED_SECRET_ADENINE')
            check_did = request.did

            DATABASE_URI = config('SQLALCHEMY_DATABASE_URI')
            engine = create_engine(DATABASE_URI)
            Session = sessionmaker(bind=engine)
            date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")
            

            if secret_key == request.secret_key:
                api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(stringLength))
                s = Session()
                result = s.query(Users).filter_by(did = check_did).first()
                
                if result is None:
                    user = Users(
                    did = check_did,
                    name = 'Elastos User',
                    email = 'elastos_user@elastos.org',
                    created_on = date_now,
                    last_logged_on = date_now
                    )
                    s.add(user)
                    s.commit()
                    insert = s.query(Users).filter_by(did = check_did).first()
                    userapi = UserApiRelations(
                    user_id = insert.id,
                    api_key = api_key
                    )
                    s.add(userapi)
                    s.commit()
                    s.close()
                elif result.did == check_did:
                    user_api_insert = UserApiRelations(
                    user_id = result.id,
                    api_key = api_key
                    )
                    s.add(user_api_insert)
                    s.commit()
                    s.close()

                return common_pb2.Response(api_key=api_key, status_message='Success', status=True)
            else:
                return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)

    def GetAPIKey(self, request, context):

            secret_key = config('SHARED_SECRET_ADENINE')
            DATABASE_URI = config('SQLALCHEMY_DATABASE_URI')
            engine = create_engine(DATABASE_URI)
            Session = sessionmaker(bind=engine)

            if secret_key == request.secret_key:
                s = Session()
                show_did = request.did
                result = s.query(Users).filter_by(did = show_did).first()

                if result.did == show_did:
                    get_api_key = s.query(UserApiRelations).filter_by(user_id = result.id).first()
                    got_api_key = get_api_key.api_key
                    print("The API key is " + got_api_key)
                    s.close()
                    return common_pb2.Response(api_key = got_api_key, status_message='Success', status=True)
            else:
                return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)
