import json

from elastos_adenine.wallet import Wallet

def main():
    api_key = "9A5Fy8jDxsJQSDdU4thLZs9fwDmtVzBU"
    address = "EQeMkfRk3JzePY7zpUSg5ZSvNsWedzqWXN"
    address_eth = "0x282c2795B9722d638778f5a1A0045c60b330F1A0"
    try:
        wallet = Wallet()
        print("\n--> Request ELA")
        # Mainchain
        response = wallet.request_ela(api_key, 'mainchain', address)
        json_output = json.loads(response.output)
        if response.status:
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: ", response.status_message)

        # DID sidechain
        response = wallet.request_ela(api_key, 'did', address)
        json_output = json.loads(response.output)
        if response.status:
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: ", response.status_message)

        # Token sidechain
        response = wallet.request_ela(api_key, 'token', address)
        json_output = json.loads(response.output)
        if response.status:
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: ", response.status_message)

        # Eth sidechain
        response = wallet.request_ela(api_key, 'eth', address_eth)
        json_output = json.loads(response.output)
        if response.status:
            for i in json_output['result']:
                print(i, ':', json_output['result'][i])
        else:
            print("Error Message: ", response.status_message)
    except Exception as e:
        print(e)
    finally:
        wallet.close()

if __name__ == '__main__':
    main()
