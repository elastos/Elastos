from elastos_adenine.common import Common

def main():
    SHARED_SECRET_ADENINE = "7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW"
    did = 'iHdasfhasdflkHdasfasdfD'
    try:
        common = Common()
        # Generate API Key
        print("--> Generate Api Request")
        response = common.generate_api_request(SHARED_SECRET_ADENINE, did)
        if response.status:
            print("Api Key: " + response.api_key)
        else:
            print("Error Message: " + response.status_message)
    except Exception as e:
        print(e)
    finally:
        common.close()

if __name__ == '__main__':
    main()
