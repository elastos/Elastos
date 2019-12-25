import datetime
from grpc_adenine.database import db_engine
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.database.services_list import ServicesLists
from sqlalchemy.orm import sessionmaker


class RateLimiter:
    def __init__(self):
        self.date_format = '%Y-%m-%d %H:%M:%S.%f'
        Session = sessionmaker(bind=db_engine)
        self.session = Session()

    def get_last_access_count(self, api_key, service_name):
        api_key_data = self.session.query(UserApiRelations).filter_by(api_key=api_key).first()
        result = self.session.query(ServicesLists).filter_by(service_name=service_name,
                                                             user_api_id=api_key_data.id).first()

        if result is not None:
            date1 = datetime.datetime.now().strftime(self.date_format)
            date2 = result.last_access.strftime(self.date_format)
            diff = datetime.datetime.strptime(date1, self.date_format) - datetime.datetime.strptime(date2,
                                                                                                    self.date_format)

            lists = {
                'user_api_id': result.user_api_id,
                'service_name': result.service_name,
                'last_access': result.last_access,
                'access_count': result.access_count,
                'diff': diff.seconds
            }

            self.session.close()
            return lists
        else:
            return False

    def add_access_count(self, user_api_id, service_name, flag):
        service_list_data = self.session.query(ServicesLists).filter_by(service_name=service_name,
                                                                        user_api_id=user_api_id).first()

        date_now = datetime.datetime.now().strftime(self.date_format)

        if flag == 'increment':
            service_list_data.access_count += 1
        elif flag == 'reset':
            service_list_data.access_count = 1

        self.session.add(service_list_data)

        self.session.commit()
        self.session.close()
        return True

    def add_new_access_entry(self, api_key, service_name):
        date_now = datetime.datetime.now().strftime(self.date_format)
        api_key_data = self.session.query(UserApiRelations).filter_by(api_key=api_key).first()

        print(api_key_data)
        print(api_key_data.id)

        services_lists = ServicesLists(
            user_api_id=api_key_data.id,
            service_name=service_name,
            last_access=date_now,
            access_count=1
        )
        self.session.add(services_lists)
        self.session.commit()

        self.session.close()
        return True
