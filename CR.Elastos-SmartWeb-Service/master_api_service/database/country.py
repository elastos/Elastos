from datetime import datetime

from master_api_service.database import db
from sqlalchemy import ForeignKey

class Country(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(20), nullable=False)

    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return '<Country %r>' % self.name