from datetime import datetime

from grpc_adenine.database import (connection as db)

"""
ServicesList table is mapped to the elastos_console database. It maps the User with its api key.
"""


class ServicesLists(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_api_id = db.Column(db.Integer, db.ForeignKey('user_api_relations.id'), nullable=False)
    service_name = db.Column(db.String(20), unique=True, nullable=False)
    last_access = db.Column(db.DateTime, nullable=False)
    access_count = db.Column(db.Integer, default=0)

    def __init__(self, user_api_id, service_name, last_access, access_count):
        self.user_api_id = user_api_id
        self.service_name = service_name
        if last_access is None:
            last_access = datetime.utcnow()
        self.last_access = last_access
        self.access_count = access_count

    def __repr__(self):
        return "(user_api_id:{}, service_name:{}, last_access:{}, access_count:{})" \
            .format(self.user_api_id, self.service_name, self.last_access, self.access_count)
