package httpjsonrpc

import (
	"github.com/elastos/Elastos.ELA.SideChain.ID/servers"

	"github.com/elastos/Elastos.ELA.SideChain/servers/httpjsonrpc"
)

func InitRpcServer() {
	httpjsonrpc.InitRpcServer()
	httpjsonrpc.MainMux["getidentificationtxbyidandpath"] = servers.GetIdentificationTxByIdAndPath
}
