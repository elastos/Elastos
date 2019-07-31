from datetime import datetime

from master_api_service.database import db
from sqlalchemy import ForeignKey

class User_Api_Relation(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.String(20), db.ForeignKey('user_type.id'), nullable=False)
    api_key = db.Column(db.String(20), unique=True, nullable=False)

    def __init__(self, user_id, api_key):
        self.user_id = user_id
        self.api_key = api_key

    def __repr__(self):
        return '<User_Api_Relation %r>' % self.user_id