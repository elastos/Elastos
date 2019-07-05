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
	"enode://0e96be781e9e2bc7a960e60bf597847f6aec476c2dcf4b14d7cc5f1722c5e70184b71c955c2fb4b263f3c02b7ebb1395346b828e3c0a25fcafc18e47a505f033@192.168.205.4:30301",
}

// TestnetBootnodes are the enode URLs of the P2P bootstrap nodes running on the
// Ropsten test network.
var TestnetBootnodes = []string{
	"enode://5e1d6f9f74e33b2d1e2fda87efaf60a788b338c08eefd3a435e9c7de98645bc041421c27d9ed3927c7b5195febd691aff30de881842749f3030089df0e135232@3.208.184.54:30301",
	"enode://30dc2b7986e2ec5902498ec26fad6fcecece617aa1652f227f684ede6a0939bb7a205ada1c91420d30b427c86bbdcc31fdfd6d955dd8f5854370f583025a0708@3.209.35.13:30301",
	"enode://b0357d45e9070c1660f63f077e0e3b0054a18d93785589d498586b6e0b7ec7c5b39ef608e82e7280ca95019db7c36455275d98a3e8684916ba8f3a7aab4ad38b@3.210.227.193:30301",
}

// RinkebyBootnodes are the enode URLs of the P2P bootstrap nodes running on the
// Rinkeby test network.
var RinkebyBootnodes = []string{
	"enode://ec8a4e686c19702ae3dae0b362bb8d6ad390f40c7eb1bb8bc0a22d506fb09c34efbe7baa492e8ba6ea6d5be238d7a478aede8f8b4f968686eeca562d9ac63fdb@52.81.82.85:30301",
}

// DiscoveryV5Bootnodes are the enode URLs of the P2P bootstrap nodes for the
// experimental RLPx v5 topic-discovery network.
var DiscoveryV5Bootnodes = []string{
	"enode://da476658b470ccfd35e7886cd8c971ef77fa0ae6557e963686af7ef3f09cf484ee3063301db2e33969b31dbbff480373911fa2e478cc583deb80ffa005c513c0@54.223.196.249:20000",
}
