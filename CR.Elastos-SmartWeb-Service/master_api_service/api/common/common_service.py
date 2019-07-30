from master_api_service.database import db

def validate_api_key(api_key):
    print("---> inside service method ",api_key)
    return False