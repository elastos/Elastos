from adenine.common import Common
from adenine.did_sidechain import DidSidechain

def run():
    
    common = Common()
    common.generateAPIRequest('eewfew', 'eewfedqww')
    
    did_sidechain =  DidSidechain()
    did_sidechain.sign('shguBwHIVO2ziKMQi3EkHDP9V4JJb5GN', 'shguBwHIVO2ziKMQi3EkHDP9V4JJb5GN', 'hey')


if __name__ == '__main__':
    run()