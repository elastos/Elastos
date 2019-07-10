from master_api_service.database import db
from master_api_service.database.models import User


def create_user(data):
    name = data.get('name')
    email = data.get('email')
    user = User(name, email)
    db.session.add(user)
    db.session.commit()

