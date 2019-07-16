# The examples in this file come from the Flask-SQLAlchemy documentation
# For more information take a look at:
# http://flask-sqlalchemy.pocoo.org/2.1/quickstart/#simple-relationships

from datetime import datetime

from master_api_service.database import db


class User(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    name = db.Column(db.String(80))
    email = db.Column(db.Text)
    #organization = db.Column(db.Text)
    #phone_number = db.Column(db.Text)
    #password = db.Column(db.Text)
    #city = db.Column(db.Text)

    created = db.Column(db.DateTime)

    def __init__(self, name, email, created_on=None):
        self.name = name
        self.email = email
        if created is None:
            created = datetime.utcnow()
        self.created = created

    def __repr__(self):
        return '<User %r>' % self.name


