from datetime import datetime

from master_api_service.database import db


class UserType(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    type = db.Column(db.String(20), nullable=False)

    def __init__(self, type):
        self.type = type

    def __repr__(self):
        return '<User_Type %r>' % self.type