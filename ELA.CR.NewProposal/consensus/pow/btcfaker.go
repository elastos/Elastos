package pow

import (
	"bytes"
	"encoding/binary"
	"time"

	. "github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/auxpow"
)

func getBtcCoinbase(msgBlockHash Uint256) *auxpow.BtcTx {
	var magic [4]byte        // 4 byte
	var auxBlockHash Uint256 // 32 byte
	var merkleSize int32     // 4 byte
	var merkleNonce int32    // 4 byte

	magic = [4]byte{0xfa, 0xbe, 'm', 'm'}
	auxBlockHash = msgBlockHash
	merkleSize = 1
	merkleNonce = 0

	scriptSig := make([]byte, 0, 44) // 44 byte
	scriptSigBuf := bytes.NewBuffer(scriptSig)
	binary.Write(scriptSigBuf, binary.LittleEndian, magic)
	binary.Write(scriptSigBuf, binary.LittleEndian, auxBlockHash)
	binary.Write(scriptSigBuf, binary.LittleEndian, merkleSize)
	binary.Write(scriptSigBuf, binary.LittleEndian, merkleNonce)

	coinBaseTxin := auxpow.BtcTxIn{
		PreviousOutPoint: auxpow.BtcOutPoint{
			Hash:  Uint256{},
			Index: uint32(0),
		},
		SignatureScript: scriptSigBuf.Bytes(),
		Sequence:        uint32(0),
	}

	btcTxin := make([]*auxpow.BtcTxIn, 0)
	btcTxin = append(btcTxin, &coinBaseTxin)
	btcTxout := make([]*auxpow.BtcTxOut, 0)

	coinbase := auxpow.NewBtcTx(btcTxin, btcTxout)

	return coinbase
}

func generateAuxPow(msgBlockHash Uint256) *auxpow.AuxPow {
	auxMerkleBranch := make([]Uint256, 0)
	auxMerkleIndex := 0
	parCoinbaseTx := getBtcCoinbase(msgBlockHash)
	parCoinBaseMerkle := make([]Uint256, 0)
	parMerkleIndex := 0
	parBlockHeader := auxpow.BtcBlockHeader{
		Version:    0x7fffffff,
		PrevBlock:  Uint256{},
		MerkleRoot: parCoinbaseTx.Hash(),
		Timestamp:  uint32(time.Now().Unix()),
		Bits:       0, // do not care about parent block diff
		Nonce:      0, // to be solved
	}
	auxPow := auxpow.NewAuxPow(
		auxMerkleBranch,
		auxMerkleIndex,
		*parCoinbaseTx,
		parCoinBaseMerkle,
		parMerkleIndex,
		parBlockHeader,
	)

	return auxPow
}
