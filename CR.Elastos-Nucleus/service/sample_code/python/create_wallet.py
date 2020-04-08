import json

from elastos_adenine.wallet import Wallet

def main():
    host = "localhost"
    port = 8001
    production = False

    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    did = "iHdasfhasdflkHdasfasdfD"
    network = "gmunet"

    try:
        wallet = Wallet(host, port, production)
        print("\n--> Create Wallet")
        response = wallet.create_wallet(api_key, did, network)
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
        wallet.close()

if __name__ == '__main__':
    main()
