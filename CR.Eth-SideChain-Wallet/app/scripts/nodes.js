'use strict';
var nodes = function() {}
nodes.customNode = require('./nodeHelpers/customNode');
nodes.infuraNode = require('./nodeHelpers/infura');
nodes.metamaskNode = require('./nodeHelpers/metamask');
nodes.nodeTypes = {
    ETH: "ETH",
    ETC: "ETC",
    MUS: "MUSIC",
    Ropsten: "ROPSTEN ETH",
    Kovan: "KOVAN ETH",
    Rinkeby: "RINKEBY ETH",
    RSK: "RSK",
    EXP: "EXP",
    UBQ: "UBQ",
    POA: "POA",
    TOMO: "TOMO",
    ELLA: "ELLA",
    ETSC: "ETSC",
    Custom: "CUSTOM ETH"
};
nodes.ensNodeTypes = [nodes.nodeTypes.ETH, nodes.nodeTypes.Ropsten];
nodes.domainsaleNodeTypes = [nodes.nodeTypes.ETH, nodes.nodeTypes.Ropsten];
nodes.customNodeObj = {
    'name': 'CUS',
    'blockExplorerTX': '',
    'blockExplorerAddr': '',
    'type': nodes.nodeTypes.Custom,
    'eip155': false,
    'chainId': '',
    'tokenList': [],
    'abiList': [],
    'service': 'Custom',
    'lib': null
};
nodes.nodeList = {
    'eth_mycrypto': {
        'name': 'ETH',
        'blockExplorerTX': 'https://ethchain.elastos.org//tx/[[txHash]]',
        'blockExplorerAddr': 'https://ethchain.elastos.org//address/[[address]]',
        'type': nodes.nodeTypes.ETH,
        'eip155': true,
        'chainId': 1,
        'tokenList': require('./tokens/ethTokens.json'),
        'abiList': require('./abiDefinitions/ethAbi.json'),
        'service': 'mainnet',
        'lib': new nodes.customNode('https://mainrpc.elaeth.io', '')
    },
    'eth_etherscan': {
        'name': 'ETH',
        'blockExplorerTX': 'https://explorer.elaeth.io/tx/[[txHash]]',
        'blockExplorerAddr': 'https://explorer.elaeth.io/address/[[address]]',
        'type': nodes.nodeTypes.ETH,
        'eip155': true,
        'chainId': 3,
        'tokenList': require('./tokens/ethTokens.json'),
        'abiList': require('./abiDefinitions/etcAbi.json'),
        'service': 'testnet',
        'lib': new nodes.customNode('https://rpc.elaeth.io', '')
    },
};

// add a 'key' attribute to each node that is its object key
Object.keys(nodes.nodeList).forEach(function(key) {
    nodes.nodeList[key].key = key;
});


nodes.ethPrice = require('./nodeHelpers/ethPrice');
module.exports = nodes;
