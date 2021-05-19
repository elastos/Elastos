import json

from elastos_adenine.common import Common

shared_secret_adenine = "7XDnFBdHafpPyIC4nrtuJ5EUYVqdEKjW"

def main():
    host = "localhost"
    port = 8001
    production = False

    did = "iHdasfhasdflkHdasfasdfD"

    generate_api_key(host, port, production, did)
    get_api_key(host, port, production, did)

def generate_api_key(host, port, production, did_to_use):
    try:
        common = Common(host, port, production)
        # Generate API Key
        print("--> Generate API Key - SHARED_SECRET_ADENINE")
        response = common.generate_api_request(shared_secret_adenine, did_to_use)
        if response['status']:
            json_output = json.loads(response['output'])
            print("Status Message :", response['status_message'])
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: " + response['status_message'])
    except Exception as e:
        print(e)
    finally:
        common.close()

def get_api_key(host, port, production, did_to_use):
    try:
        common = Common(host, port, production)
        # Get API Key
        print("--> Get API Key - SHARED_SECRET_ADENINE")
        response = common.get_api_key_request(shared_secret_adenine, did_to_use)
        if response['status']:
            json_output = json.loads(response['output'])
            print("Status Message :", response['status_message'])
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: " + response['status_message'])
    except Exception as e:
        print(e)
    finally:
        common.close()

if __name__ == '__main__':
    main()
