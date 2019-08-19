// Copyright (c) 2017-2019 Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package wallet

import (
	"github.com/elastos/Elastos.ELA/blockchain"
)

const (
	key             = "utxo"
	dataExtension   = ".ucp"
	savePeriod      = uint32(720)
	effectivePeriod = uint32(720)

	WalletVersion = "0.0.1"
)

var (
	Store blockchain.IChainStore
	FFLDB blockchain.IFFLDBChainStore
	Chain  *blockchain.BlockChain
)
