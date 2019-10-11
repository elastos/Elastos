from datetime import datetime

from smartweb_service.database import db
from sqlalchemy import ForeignKey


"""
UserApiRelation table is mapped to the elastos_console database. It maps the User with its api key.
"""

class UserApiRelation(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.String(20), db.ForeignKey('users.id'), nullable=False)
    api_key = db.Column(db.String(20), unique=True, nullable=False)

    def __init__(self, user_id, api_key):
        self.user_id = user_id
        self.api_key = api_key

    def __repr__(self):
        return '<UserApiRelation %r>' % self.user_id