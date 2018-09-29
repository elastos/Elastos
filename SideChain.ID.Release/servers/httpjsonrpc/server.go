package httpjsonrpc

import (
	sv "github.com/elastos/Elastos.ELA.SideChain.ID/servers"

	"github.com/elastos/Elastos.ELA.SideChain/servers/httpjsonrpc"
)

type RpcserverExtend struct {
	*httpjsonrpc.Rpcserver
	httpServers *sv.HttpServiceExtend
}

func New(port uint16, service *sv.HttpServiceExtend) *RpcserverExtend {

	return &RpcserverExtend{
		Rpcserver :  httpjsonrpc.New(port, service.HttpService),
		httpServers: service,
	}
}

func (s *RpcserverExtend) Start() {
	s.Rpcserver.Start()
	s.RegisterAction("getidentificationtxbyidandpath", s.httpServers.GetIdentificationTxByIdAndPath)
}