import json

from elastos_adenine.wallet import Wallet

def main():
    host = "localhost"
    port = 8001
    production = False

    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    did = "iHdasfhasdflkHdasfasdfD"
    network = "gmunet"
    address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
    address_eth = "0x282c2795B9722d638778f5a1A0045c60b330F1A0"

    try:
        wallet = Wallet(host, port, production)
        print("\n--> View Wallet")
        # Mainchain
        response = wallet.view_wallet(api_key, did, network, 'mainchain', address)
        if response['status']:
            json_output = json.loads(response['output'])
            print("Status Message :", response['status_message'])
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: " + response['status_message'])

        # DID sidechain
        response = wallet.view_wallet(api_key, did, network, 'did', address)
        if response['status']:
            json_output = json.loads(response['output'])
            print("Status Message :", response['status_message'])
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: " + response['status_message'])

        # Token sidechain
        response = wallet.view_wallet(api_key, did, network, 'token', address)
        if response['status']:
            json_output = json.loads(response['output'])
            print("Status Message :", response['status_message'])
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: " + response['status_message'])

        # Eth sidechain
        response = wallet.view_wallet(api_key, did, network, 'eth', address_eth)
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
