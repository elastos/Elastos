package httprestful

import "github.com/elastos/Elastos.ELA.SideChain/service"

type Server interface {
	RegisterAction(method, path string, handler service.Handler, params ...string)
	Start() error
	Stop() error
}
