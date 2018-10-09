package httpjsonrpc

import "github.com/elastos/Elastos.ELA.SideChain/servers"

type Server interface {
	RegisterAction(name string, action servers.Handler, params... string)
	Start() error
	Stop() error
}
