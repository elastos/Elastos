from datetime import datetime

from smartweb_service.database import db


"""
UserType table is mapped to the elastos_console database. It contains the different types of users that the console has eg., Administrator, developer.
"""

class UserType(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    type = db.Column(db.String(20), nullable=False)

    def __init__(self, type):
        self.type = type

    def __repr__(self):
        return '<User_Type %r>' % self.type