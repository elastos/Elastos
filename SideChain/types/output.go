package types

import (
	"fmt"
	"math/big"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type Output struct {
	AssetID     Uint256
	Value       Fixed64
	TokenValue  big.Int
	OutputLock  uint32
	ProgramHash Uint168
}

func (o Output) String() string {
	return "Output: {\n\t\t" +
		"AssetID: " + o.AssetID.String() + "\n\t\t" +
		"Value: " + o.Value.String() + "\n\t\t" +
		"OutputLock: " + fmt.Sprint(o.OutputLock) + "\n\t\t" +
		"ProgramHash: " + o.ProgramHash.String() + "\n\t\t" +
		"}"
}
