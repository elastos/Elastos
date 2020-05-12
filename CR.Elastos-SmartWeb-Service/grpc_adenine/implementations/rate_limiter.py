import datetime
import logging

from sqlalchemy.orm import sessionmaker, scoped_session

from grpc_adenine.database import db_engine
from grpc_adenine.database.user_api_relation import UserApiRelations
from grpc_adenine.database.services_list import ServicesLists


class RateLimiter:
    def __init__(self, session=None):
        self.date_format = '%Y-%m-%d %H:%M:%S.%f'
        session_factory = sessionmaker(bind=db_engine)
        self.session = scoped_session(session_factory)

    def check_rate_limit(self, limit, api_key, service_name):
        session_local = self.session()
        response = {}
        try:
            result = self.get_last_access_count(api_key, service_name)
            if result:
                if result["diff"] < 86400:
                    if limit > result["access_count"]:
                        self.add_access_count(result["user_api_id"], service_name, 'increment')
                    else:
                        response = {
                            'result': {
                                'API': service_name,
                                'daily_limit': limit,
                                'num_requests': result['access_count']
                            }
                        }
                        session_local.commit()
                        return response
                else:
                    session_local.add_access_count(result["user_api_id"], service_name, 'reset')
            else:
                session_local.add_new_access_entry(api_key, service_name)
            session_local.commit()
        except Exception as e:
            session_local.rollback()
            logging.debug(f"method: 'check_rate_limit' : {api_key} : {service_name}: {e}")
        finally:
            session_local.close()
        return response

    def get_last_access_count(self, api_key, service_name):
        session_local = self.session()
        try:
            api_key_data = session_local.query(UserApiRelations).filter_by(api_key=api_key).first()
            result = session_local.query(ServicesLists).filter_by(service_name=service_name,
                                                                 user_api_id=api_key_data.id).first()

            if result is not None:
                date1 = datetime.datetime.now().strftime(self.date_format)
                date2 = result.last_access.strftime(self.date_format)
                diff = datetime.datetime.strptime(date1, self.date_format) - datetime.datetime.strptime(date2,
                                                                                                        self.date_format)
                diff_seconds = diff.days * 24 * 3600 + diff.seconds

                lists = {
                    'user_api_id': result.user_api_id,
                    'service_name': result.service_name,
                    'last_access': result.last_access,
                    'access_count': result.access_count,
                    'diff': diff_seconds
                }
                result = lists

            session_local.commit()
        except Exception as e:
            session_local.rollback()
            logging.debug(f"method: 'get_last_access_count' : {api_key} : {service_name}: {e}")
        finally:
            session_local.close()
        return result

    def add_access_count(self, user_api_id, service_name, flag):
        session_local = self.session()
        try:
            service_list_data = session_local.query(ServicesLists).filter_by(service_name=service_name,
                                                                            user_api_id=user_api_id).first()

            date_now = datetime.datetime.now().strftime(self.date_format)

            if flag == 'increment':
                service_list_data.access_count += 1
            elif flag == 'reset':
                service_list_data.access_count = 1
                service_list_data.last_access = date_now

            session_local.add(service_list_data)
            session_local.commit()
        except Exception as e:
            session_local.rollback()
            logging.debug(f"method: 'add_access_count' : {service_name}: {e}")
        finally:
            session_local.close()
        return True

    def add_new_access_entry(self, api_key, service_name):
        session_local = self.session()
        try:
            date_now = datetime.datetime.now().strftime(self.date_format)
            api_key_data = session_local.query(UserApiRelations).filter_by(api_key=api_key).first()

            services_lists = ServicesLists(
                user_api_id=api_key_data.id,
                service_name=service_name,
                last_access=date_now,
                access_count=1
            )
            session_local.add(services_lists)
            session_local.commit()
        except Exception as e:
            session_local.rollback()
            logging.debug(f"method: 'add_new_access_entry' : {api_key}: {service_name}: {e}")
        finally:
            session_local.close()
        return True
