pragma solidity ^0.4.25;
pragma experimental ABIEncoderV2;

contract CrossChainMainContract {

    address lib = address(0);

    function upgrade() public {
        address addr;
        assembly {
            addr := getsubcontractaddress()
        }
        require(addr != lib);
        lib = addr;
    }

    function () public payable {
        require(lib != address(0));
        if (msg.data.length > 0) {
            lib.delegatecall(msg.data);
        }
    }

    function watchlibaddr() view returns (address _lib) {
        return lib;
    }
}
