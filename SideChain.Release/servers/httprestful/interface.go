package httprestful

import "github.com/elastos/Elastos.ELA.SideChain/servers"

type Server interface {
	RegisterAction(method, path string, handler servers.Handler, params ...string)
	Start() error
	Stop() error
}
