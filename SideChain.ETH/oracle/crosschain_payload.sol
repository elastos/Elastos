pragma solidity ^0.4.25;
pragma experimental ABIEncoderV2;

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

    function receivePayload(string[] _addrs, uint256[] _amounts, uint256 _fee) public payable {
        uint256 total = 0;
        uint256 i = 0;
        require(_fee >= 100000000000000 && _fee % 10000000000 == 0);
        require(_addrs.length == _amounts.length);
        while (i < _amounts.length) {
            require(_amounts[i] % 10000000000 == 0 && _amounts[i].sub(_fee) >= _fee);
            total = total.add(_amounts[i]);
            emit PayloadReceived(_addrs[i], _amounts[i], _amounts[i].sub(_fee), msg.sender);
            i++;
        }
        require(total == msg.value);
        address(0).transfer(msg.value);
        emit EtherDeposited(msg.sender, msg.value, address(0));
    }

    function() public payable {
        revert();
    }
}