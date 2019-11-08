from adenine.common import Common
from adenine.did_sidechain import DidSidechain

def run():
    
    common = Common()
    common.generateAPIRequest('kHDP9V4JJbwd5GN', 'qhfiueq98dqwbd')
    
    did_sidechain =  DidSidechain()
    did_sidechain.sign('shguBwHIVOi3EkHDP9V4JJb5GN', 'shguBwHIVO2wdw3EkHDPJb5GN', 'hey')


if __name__ == '__main__':
    run()