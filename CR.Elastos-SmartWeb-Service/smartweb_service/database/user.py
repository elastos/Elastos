# The examples in this file come from the Flask-SQLAlchemy documentation
# For more information take a look at:
# http://flask-sqlalchemy.pocoo.org/2.1/quickstart/#simple-relationships

from datetime import datetime

from master_api_service.database import db
from sqlalchemy import ForeignKey

"""
Users table is mapped to the elastos_console database. Users table has the user information which is stored in it during the registration.
"""

class Users(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    user_id = db.Column(db.String(20), unique=True, nullable=False)
    did = db.Column(db.String(50), unique=True, nullable=False)
    name = db.Column(db.String(80), nullable=False)
    email = db.Column(db.Text, nullable=False)
    organization = db.Column(db.String(50))
    phone_number = db.Column(db.Integer)
    city = db.Column(db.String(20), nullable=False)
    country = db.Column(db.Integer, db.ForeignKey('country.id'), nullable=False)
    user_type = db.Column(db.Integer, db.ForeignKey('user_type.id'), nullable=False)
    created_on = db.Column(db.DateTime, nullable=False)
    last_logged_on = db.Column(db.DateTime, nullable=False)

    def __init__(self, user_id, did, name, email,
                 organization, phone_number, city, country, user_type, created_on=None, last_logged_on=None):
        self.user_id = user_id
        self.did = did
        self.name = name
        self.email = email
        self.organization = organization
        self.phone_number = phone_number
        self.city = city
        self.country = country
        self.user_type = user_type
        if created_on is None:
            created_on = datetime.utcnow()
        self.created_on = created_on
        self.last_logged_on = last_logged_on

    def __repr__(self):
        return '<User %r>' % self.name


