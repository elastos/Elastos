import json

from elastos_adenine.hive import Hive

def main():
    host = "localhost"
    port = 8001
    production = False

    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    did = "iHdasfhasdflkHdasfasdfD"
    network = "gmunet"
    private_key = "1F54BCD5592709B695E85F83EBDA515971723AFF56B32E175F14A158D5AC0D99"
    file_to_upload = "test/sample.txt"

    try:
        hive = Hive(host, port, production)
        # Upload and Sign
        print("\n--> Upload and Sign")
        response = hive.upload_and_sign(api_key, did, network, private_key, file_to_upload)
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
        hive.close()

if __name__ == '__main__':
    main()
