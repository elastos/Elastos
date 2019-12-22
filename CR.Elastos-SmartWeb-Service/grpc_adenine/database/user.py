# The examples in this file come from the Flask-SQLAlchemy documentation
# For more information take a look at:
# http://flask-sqlalchemy.pocoo.org/2.1/quickstart/#simple-relationships

from datetime import datetime

from grpc_adenine.database import (connection as db)
from sqlalchemy import ForeignKey

"""
Users table is mapped to the elastos_console database. Users table has the user information which is stored in it during the registration.
"""


class Users(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    did = db.Column(db.String(50), unique=True, nullable=False)
    name = db.Column(db.String(80), nullable=False)
    email = db.Column(db.Text, nullable=False)
    created_on = db.Column(db.DateTime, nullable=False)
    last_logged_on = db.Column(db.DateTime, nullable=False)

    def __init__(self, did, name, email, created_on=None, last_logged_on=None):
        self.did = did
        self.name = name
        self.email = email
        if created_on is None:
            created_on = datetime.utcnow()
        self.created_on = created_on
        self.last_logged_on = last_logged_on

    def _repr_(self):
        return "(did={})"\
            .format(self.name)


