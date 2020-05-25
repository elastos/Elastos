// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package wallet

import (
	"sync"

	"github.com/elastos/Elastos.ELA/blockchain"
	"github.com/elastos/Elastos.ELA/common/config"
)

var (
	Store      blockchain.IChainStore
	Chain      *blockchain.BlockChain
	ChainParam *config.Params

	addressBook = make(map[string]*AddressInfo, 0)
	abMutex     sync.RWMutex
)

// GetWalletAccount retrieval an address information in wallet by address
func GetWalletAccount(address string) (*AddressInfo, bool) {
	abMutex.RLock()
	defer abMutex.RUnlock()

	addressInfo, exist := addressBook[address]
	return addressInfo, exist
}

// SetWalletAccount set an address information to wallet
func SetWalletAccount(addressInfo *AddressInfo) {
	abMutex.Lock()
	defer abMutex.Unlock()

	addressBook[addressInfo.address] = addressInfo
}
