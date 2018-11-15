package msg

import (
	"bytes"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

const (
	CmdVersion = "version"
	CmdVerAck  = "verack"
	CmdPing    = "ping"
	CmdPong    = "pong"

	ReceivedProposal  = "proposal"
	AcceptVote        = "acc_vote"
	RejectVote        = "rej_vote"
	GetBlocks         = "get_blc"
	ResponseBlocks    = "res_blc"
	RequestConsensus  = "req_con"
	ResponseConsensus = "res_con"
)

func GetMessageHash(msg p2p.Message) common.Uint256 {
	buf := new(bytes.Buffer)
	msg.Serialize(buf)
	msgHash := common.Sha256D(buf.Bytes())
	return msgHash
}
