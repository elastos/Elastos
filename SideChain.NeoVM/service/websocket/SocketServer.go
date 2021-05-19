package websocket

import (
	"encoding/json"

	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/service/websocket"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/utils/http"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/store"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm/datatype"
	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/event"
)

type SocketServer struct {
	Server *websocket.Server
}

func NewSocketServer(orgCfg *websocket.Config) *SocketServer {
	s := websocket.NewServer(orgCfg)
	server := &SocketServer{
		s,
	}

	return server
}

func (s *SocketServer) OnEvent(et *events.Event) {
	var resp = &store.ResponseExt{
		Action:   "",
		Result:   false,
		Desc:     "",
		TxID:     "",
		CodeHash: "",
	}
	var err error = nil
	if et.Type == event.ETRunTimeNotify {
		resp.Action = store.RunTime_Notify
		resp.Desc = common.BytesToHexString(et.Data.(datatype.StackItem).GetByteArray())
	} else if et.Type == event.ETRunTimeLog {
		resp.Action = store.RunTime_Log
		resp.Desc = string(et.Data.(datatype.StackItem).GetByteArray())
	} else if et.Type == event.ETDeployTransaction || et.Type == event.ETInvokeTransaction {
		resp = et.Data.(*store.ResponseExt)
	}

	if resp.Action != "" {
		s.response(resp, err)
	}
}

func (s *SocketServer) response(resp *store.ResponseExt, err error) {
	if err != nil {
		switch e := err.(type) {
		case *http.Error:
			resp.Error = e.Code
		}
	}

	data, err := json.Marshal(resp)
	if err != nil {
		log.Fatalf("HTTP Handle - json.Marshal: %v", err)
		return
	}

	sessionList := s.Server.GetSessionList()
	sessionList.ForEach(func(session *websocket.Session) {
		session.Send(data)
	})
}
