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
from sqlalchemy import update


class Common(common_pb2_grpc.CommonServicer):

    def GenerateAPIRequest(self, request, context):
        string_length = 64
        secret_key = config('SHARED_SECRET_ADENINE')
        check_did = request.did

        DATABASE_URI = config('SQLALCHEMY_DATABASE_URI')
        engine = create_engine(DATABASE_URI)
        Session = sessionmaker(bind=engine)
        date_now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S %z")

        if secret_key == request.secret_key:
            api_key = ''.join(random.choice(string.ascii_letters + string.digits) for i in range(string_length))
            s = Session()
            api_present = s.query(UserApiRelations).filter_by(api_key = api_key).first()
            
            if api_present is None:
                result = s.query(Users).filter_by(did=check_did).first()

                if result is None:
                    user = Users(
                    did = check_did,
                    created_on = date_now,
                    last_logged_on = date_now
                    )
                    s.add(user)
                    s.commit()
                    insert = s.query(Users).filter_by(did = check_did).first()
                    user_api = UserApiRelations(
                    user_id = insert.id,
                    api_key = api_key
                    )
                    s.add(user_api)
                    s.commit()
                    s.close()

                elif result.did == check_did:
                    insert = s.query(UserApiRelations).filter_by(user_id = result.id).first()
                    # print(insert.api_key)
                    insert.api_key = api_key
                    # print(insert.api_key)
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
            result = s.query(Users).filter_by(did=show_did).first()

            if result.did == show_did:
                get_api_key = s.query(UserApiRelations).filter_by(user_id=result.id).first()
                got_api_key = get_api_key.api_key
                s.close()
                return common_pb2.Response(api_key=got_api_key, status_message='Success', status=True)
        else:
            return common_pb2.Response(api_key='', status_message='Authentication Error', status=False)
