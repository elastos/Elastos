from master_api_service.database import db
from master_api_service.database.user_api_relation import UserApiRelation

def validate_api_key(api_key):
	user_api_rel = UserApiRelation.query.filter(UserApiRelation.api_key == api_key).first()
	if user_api_rel is None:
		return False
	return True