// Copyright 2015 The Elastos.ELA.SideChain.ETH Authors
// This file is part of the Elastos.ELA.SideChain.ETH library.
//
// The Elastos.ELA.SideChain.ETH library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Elastos.ELA.SideChain.ETH library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Elastos.ELA.SideChain.ETH library. If not, see <http://www.gnu.org/licenses/>.

package params

// MainnetBootnodes are the enode URLs of the P2P bootstrap nodes running on
// the main Ethereum network.
var MainnetBootnodes = []string{
	"enode://dee112e94b17b3b49366e5dec78d7e8a1ee342ff363b490819d40a55482046d333b2bd51b3d1ce250078c5315bf302758d13f63ac94fd8e43f6e54be8412c316@52.74.28.202:20630",
	"enode://152fae4134f4db49d24905762ade694fc86e0a24124c0927c9c1cbc816bb9929e790d4fba236c7a55c9d9817df72c1d23353c2dccc3796bd397d72320a722ef1@52.62.113.83:20630",
	"enode://dbfdb62b5cf4cb5a12ee1df68bfb4c0626ad5335ec5ee0c594c315b08a61e7f0bc8ce5b264136eec0db17db1e55f1bb0f1de67f9bb9c57bea77feef74f2baa2c@35.156.51.127:20630",
	"enode://a1a37849c8a0d5247870fc2d70da053fdae503b99498daf63905728bc801a57818577a88b02895763a5af8037ab5378b3ea12eb01ec2546712cf5ebaab3e94c6@35.177.89.244:20630",
	"enode://9b43d046414b722ec3e6237083ab413280555cf4ee5765c9a4c55c9fc86c979c7389840a35377846cf0c6923afa7fa3cd115f6d8152e1f8df24273466cb15cf1@52.53.134.102:20630",
}

// TestnetBootnodes are the enode URLs of the P2P bootstrap nodes running on the
// Ropsten test network.
var TestnetBootnodes = []string{
	"enode://5e1d6f9f74e33b2d1e2fda87efaf60a788b338c08eefd3a435e9c7de98645bc041421c27d9ed3927c7b5195febd691aff30de881842749f3030089df0e135232@3.208.184.54:20630",
	"enode://30dc2b7986e2ec5902498ec26fad6fcecece617aa1652f227f684ede6a0939bb7a205ada1c91420d30b427c86bbdcc31fdfd6d955dd8f5854370f583025a0708@3.209.35.13:20630",
	"enode://b0357d45e9070c1660f63f077e0e3b0054a18d93785589d498586b6e0b7ec7c5b39ef608e82e7280ca95019db7c36455275d98a3e8684916ba8f3a7aab4ad38b@3.210.227.193:20630",
}

// RinkebyBootnodes are the enode URLs of the P2P bootstrap nodes running on the
// Rinkeby test network.
var RinkebyBootnodes = []string{
	"enode://fe44bc423f210805daad60cc5d308f449e9282c28a9aba91040d7c727cf5751d1ae9e85d32a430f4a6fe15c8eb52833a1747e8b28e6ed5ae291fdae32e6b9181@3.209.120.83:20630",
	"enode://777e2a86687d675c05344acc6e24cefbd3e233759e8b89d7b3d101aeffc89e6292f66a115c5bfc30f250c120e6a2354a7a6ea304439cfded706de1c9ade61abf@3.212.134.14:20630",
	"enode://deb84117dada6c2c8f9c5d9d44f749b6fbbefdc987a1611b683ead6e4e2ce8e0d05a196591a713376eee5d9c165d3888d2e175e8eb842e5a381f273c0268edca@3.212.156.65:20630",
}

// DiscoveryV5Bootnodes are the enode URLs of the P2P bootstrap nodes for the
// experimental RLPx v5 topic-discovery network.
var DiscoveryV5Bootnodes = []string{
	"enode://da476658b470ccfd35e7886cd8c971ef77fa0ae6557e963686af7ef3f09cf484ee3063301db2e33969b31dbbff480373911fa2e478cc583deb80ffa005c513c0@54.223.196.249:20000",
}
