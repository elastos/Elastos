from master_api_service.database import db
from master_api_service.database.user_api_relation import UserApiRelation

def validate_api_key(api_key):
	user_api_rel = UserApiRelation.query.get(1)
	print("---> inside service method ",user_api_rel.id)
	print("---> inside service method ",user_api_rel.user_id)
	print("---> inside service method ",user_api_rel.api_key)
	return False