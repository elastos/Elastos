package interfaces

import (

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain/auxpow"
)

type Header interface {
	common.Serializable

	SetVersion(uint32)
	GetVersion() uint32
	Hash() common.Uint256
	SetHeight(uint32)
	GetHeight() uint32
	SetBits(uint32)
	GetBits() uint32
	GetAuxPow() *auxpow.SideAuxPow
	SetAuxPow(*auxpow.SideAuxPow)
	SetPrevious(common.Uint256)
	GetPrevious() common.Uint256
	SetMerkleRoot(common.Uint256)
	GetMerkleRoot() common.Uint256
	SetTimeStamp(uint32)
	GetTimeStamp() uint32
	SetNonce(uint32)
	GetNonce() uint32
}
