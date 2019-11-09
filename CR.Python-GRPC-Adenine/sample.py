from adenine.common import Common
from adenine.did_sidechain import DidSidechain

def run():
    
    common = Common()
    print("--> Generate Api Request")
    common.generateAPIRequest('kHDP9V4JJbwd5GN', 'qhfiueq98dqwbd')
    
    did_sidechain =  DidSidechain()
    print("\n--> Sign a message")
    did_sidechain.sign('shguBwHIVOi3EkHDP9V4JJb5GN', 'shguBwHIVO2wdw3EkHDPJb5GN', 'hey')


if __name__ == '__main__':
    run()