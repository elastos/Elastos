package httpjsonrpc

import "github.com/elastos/Elastos.ELA.SideChain/service"

type Server interface {
	RegisterAction(name string, action service.Handler, params ...string)
	Start() error
	Stop() error
}
