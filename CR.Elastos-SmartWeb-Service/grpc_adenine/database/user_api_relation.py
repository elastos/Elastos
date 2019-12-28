from grpc_adenine.database import (connection as db)


"""
UserApiRelation table is mapped to the elastos_console database. It maps the User with its api key.
"""


class UserApiRelations(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.Integer, db.ForeignKey('users.id'), nullable=False)
    api_key = db.Column(db.String(20), unique=True, nullable=False)

    def __init__(self, user_id, api_key):
        self.user_id = user_id
        self.api_key = api_key

    def __repr__(self):
        return "(id:{}, user_id:{}, api_key:{})"\
                .format(self.id, self.user_id, self.api_key)