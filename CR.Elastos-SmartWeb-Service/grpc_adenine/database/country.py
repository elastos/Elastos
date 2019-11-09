from datetime import datetime

from grpc_adenine.database import (connection as db)
from sqlalchemy import ForeignKey


"""
Country table maps to the elastos_console database. This table maps the country name with its id.
"""

class Country(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(20), nullable=False)

    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return '<Country %r>' % self.name