from elastos_adenine.common import Common

shared_secret_adenine = "7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW"
did = 'iHdasfhasdflkHdasfasdfD'
mnemonic = 'obtain pill nest sample caution stone candy habit silk husband give net'

def generate_api_key():
    try:
        common = Common()
        # Generate API Key
        print("--> Generate API Key - SHARED_SECRET_ADENINE")
        response = common.generate_api_request(shared_secret_adenine, did)
        if response.status:
            print("Api Key: " + response.api_key)
        else:
            print("Error Message: " + response.status_message)
        print("--> Generate API Key - MNEMONICS")
        response = common.generate_api_request_mnemonic(mnemonic)
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
        # Get API Key
        print("--> Get API Key - SHARED_SECRET_ADENINE")
        response = common.get_api_key_request(shared_secret_adenine, did)
        if response.status:
            print("Api Key: " + response.api_key)
        else:
            print("Error Message: " + response.status_message)
        print("--> Get API Key - MNEMONICS")
        response = common.get_api_request_mnemonic(mnemonic)
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
