from elastos_adenine.common import Common

SHARED_SECRET_ADENINE = "7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW"
DID = 'iHdasfhasdflkHdasfasdfD'

def generate_api_key():
    try:
        common = Common()
        # Get API Key
        print("--> Get API Key")
        response = common.get_api_key_request(SHARED_SECRET_ADENINE, DID)
        if response.status:
            print("Api Key: " + response.api_key)
        else:
            print("Error Message: " + response.status_message)
    except Exception as e:
        print(e)
    finally:
        common.close()

def get_api_key():
    try:
        common = Common()
        # Generate API Key
        print("--> Generate API Key")
        response = common.generate_api_request(SHARED_SECRET_ADENINE, DID)
        if response.status:
            print("Api Key: " + response.api_key)
        else:
            print("Error Message: " + response.status_message)
    except Exception as e:
        print(e)
    finally:
        common.close()

def main():
    generate_api_key()
    get_api_key()

if __name__ == '__main__':
    main()
