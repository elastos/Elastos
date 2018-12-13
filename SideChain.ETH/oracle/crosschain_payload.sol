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

    mapping (bytes32 => bool) public txProcessed;
    mapping (bytes32 => address[]) public addrs;
    mapping (bytes32 => uint256[]) public indexes;
    mapping (bytes32 => uint256[]) public amounts;
    mapping (bytes32 => uint256) public num;

    event PayloadSent(bytes32 indexed _txhash, address indexed _addr, uint256 _amount, address indexed _arbiter);
    event TxProcessed(bytes32 indexed _txhash, address indexed _arbiter);
    event PayloadReceived(string _addr, uint256 _amount, address indexed _sender);
    event EtherDeposited(address indexed _sender, uint256 _amount);

    function sendPayload(bytes32 _txhash) public {
        require(!txProcessed[_txhash]);
        deserialize(getPayload(_txhash), _txhash);
        for (uint256 i = 0; i < num[_txhash]; i++) {
            addrs[_txhash][i].transfer(amounts[_txhash][i]);
            emit PayloadSent(_txhash, addrs[_txhash][i], amounts[_txhash][i], msg.sender);
        }
        txProcessed[_txhash] = true;
        emit TxProcessed(_txhash, msg.sender);
    }

    function getPayload(bytes32 _txhash) private view returns (bytes memory _payload) {
        uint256 size;
        // 823b -> 82dd
        assembly {
            size := spvpayloadsize(_txhash)
        }
        _payload = new bytes(size);
        // 853c -> 85de
        assembly {
            spvpayloadcopy(_txhash, add(_payload, 0x20), 0, size)
        }
    }

    function deserialize(bytes _input, bytes32 _txhash) private {
        uint256 start = 0;
        uint256 end;
        (num[_txhash], end) = getNum(_input, start);
        start = end;
        for (uint256 i = 0; i< num[_txhash]; i++) {
            address addr;
            (addr, end) = getAddress(_input, start);
            addrs[_txhash].push(addr);
            start = end;
            uint256 index;
            (index, end) = getNum(_input, start);
            indexes[_txhash].push(index);
            start = end;
            uint256 amount;
            (amount, end) = get64bitNum(_input, start);
            amount = amount.mul(uint256(10) ** uint256(10));
            amounts[_txhash].push(amount);
            start = end;
        }
    }

    function getNum(bytes _input, uint256 _start) private pure returns (uint256 _num, uint256 _end) {
        require(_input.length > _start);
        if (_input[_start] == hex"fd") {
            _num = getLittleEndian(_input, _start + 1, _start + 3);
            _end = _start + 3;
        } else if (_input[_start] == hex"fe") {
            _num = getLittleEndian(_input, _start + 1, _start + 5);
            _end = _start + 5;
        } else if (_input[_start] == hex"ff") {
            _num = getLittleEndian(_input, _start + 1, _start + 9);
            _end = _start + 9;
        } else {
            _num = getLittleEndian(_input, _start, _start + 1);
            _end = _start + 1;
        }
    }

    function getLittleEndian(bytes _input, uint256 _start, uint256 _end) private pure returns (uint256 _num) {
        require(_input.length > _start && _input.length >= _end);
        _num = 0;
        uint256 base = 1;
        for (uint256 i = _start; i < _end; i++) {
            _num = _num.add(uint256(_input[i]).mul(base));
            base = base.mul(256);
        }
    }

    function getAddress(bytes _input, uint256 _start) private pure returns (address _addr, uint256 _end) {
        require(_input.length > _start);
        uint256 slen;
        uint256 start;
        (slen, start) = getNum(_input, _start);
        require(slen == 42 || slen == 40);
        _end = start.add(slen);
        if (slen == 42) start = start.add(2);
        uint256 iaddr = 0;
        require(_input.length >= _end);
        for (uint256 i = start; i < _end; i++) {
            iaddr = iaddr.mul(uint256(16)).add(fromHexChar(uint256(_input[i])));
        }
        _addr = address(iaddr);
    }

    function fromHexChar(uint256 c) private pure returns (uint256) {
        if (byte(c) >= byte('0') && byte(c) <= byte('9')) {
            return c - uint256(byte('0'));
        }
        if (byte(c) >= byte('a') && byte(c) <= byte('f')) {
            return 10 + c - uint256(byte('a'));
        }
        if (byte(c) >= byte('A') && byte(c) <= byte('F')) {
            return 10 + c - uint256(byte('A'));
        }
    }

    function get64bitNum(bytes _input, uint256 _start) private pure returns (uint256 _num, uint256 _end) {
        require(_input.length > _start);
        _num = getLittleEndian(_input, _start, _start + 8);
        _end = _start + 8;
    }

    function receivePayload(string[] _addrs, uint256[] _amounts) public payable {
        uint256 total = 0;
        uint256 i = 0;
        while (i < _amounts.length) {
            total = total.add(_amounts[i]);
            emit PayloadReceived(_addrs[i], _amounts[i], msg.sender);
            i++;
        }

        require(total == msg.value && _addrs.length == _amounts.length);

        emit EtherDeposited(msg.sender, msg.value);
    }

    function () public payable {
        revert();
    }
}
