from datetime import datetime

from master_api_service.database import db
from sqlalchemy import ForeignKey

class UserApiRelation(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.String(20), db.ForeignKey('users.id'), nullable=False)
    api_key = db.Column(db.String(20), unique=True, nullable=False)

    def __init__(self, user_id, api_key):
        self.user_id = user_id
        self.api_key = api_key

    def __repr__(self):
        return '<UserApiRelation %r>' % self.user_id