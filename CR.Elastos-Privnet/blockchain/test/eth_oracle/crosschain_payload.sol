pragma solidity ^0.4.25;

library SafeMath {
    function add(uint256 _a, uint256 _b) internal pure returns (uint256) {
        uint256 c = _a + _b;
        assert(c >= _a);
        return c;
    }

    function sub(uint256 _a, uint256 _b) internal pure returns (uint256) {
        assert(_a >= _b);
        return _a - _b;
    }

    function mul(uint256 _a, uint256 _b) internal pure returns (uint256) {
        uint256 c = _a * _b;
        assert(_a == 0 || c / _a == _b);
        return c;
    }
}

contract CrossChainPayload {
    using SafeMath for uint256;
    event PayloadReceived(string _addr, uint256 _amount, uint256 _crosschainamount, address indexed _sender);
    event EtherDeposited(address indexed _sender, uint256 _amount, address indexed _black);

    function receivePayload(string _addr, uint256 _amount, uint256 _fee) public payable {
        require(msg.value == _amount);
        require(_fee >= 100000000000000 && _fee % 10000000000 == 0);
        require(_amount % 10000000000 == 0 && _amount.sub(_fee) >= _fee);
        emit PayloadReceived(_addr, _amount, _amount.sub(_fee), msg.sender);
        emit EtherDeposited(msg.sender, msg.value, address(0));
    }

    function() public payable {
        revert();
    }
}