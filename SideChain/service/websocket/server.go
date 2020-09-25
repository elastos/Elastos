package websocket

import (
	"bytes"
	"context"
	"encoding/hex"
	"encoding/json"
	"errors"
	"fmt"
	"net"
	"net/http"
	"time"

	"github.com/elastos/Elastos.ELA.SideChain/events"
	"github.com/elastos/Elastos.ELA.SideChain/service"
	"github.com/elastos/Elastos.ELA.SideChain/types"

	htp "github.com/elastos/Elastos.ELA/utils/http"
	"github.com/gorilla/websocket"
)

const (
	// defaultHeartbeatInterval is the default time interval to send a heartbeat
	// message.
	defaultHeartbeatInterval = time.Minute

	// PFBlock indicates push block info (JSON format) message to online
	// list when a block added to chain.
	PFBlock uint32 = 1 << iota

	// PFRawBlock indicates push RAW block message to online list when a
	// block added to chain.
	PFRawBlock

	// PFBlockTxs indicates push block transactions info (JSON format) message
	// to online list when a block added to chain.
	PFBlockTxs

	// PFBlockTxs indicates push new transaction info (JSON format) message to
	// online list when a transaction added to mempool.
	PFNewTx
)

var (
	ErrSessionExpired    = errors.New("session expired")
	ErrIllegalDataFormat = errors.New("illegal data format")
	ErrInvalidMethod     = errors.New("invalid method")
)

// Handler is the registered method to handle a http request.
type Handler func(htp.Params) (interface{}, error)

// Response represent the response data structure.
type Response struct {
	Action string
	Result interface{}
	Error  int
	Desc   string
}

type Config struct {
	ServiceCfg        *service.Config
	ServePort         uint16
	Flags             uint32
	HeartbeatInterval time.Duration
	Service           *service.HttpService
}

type Server struct {
	server *http.Server
	websocket.Upgrader
	handlers map[string]Handler

	cfg      Config
	sessions *SessionList
}

func (s *Server) Start() error {
	listener, err := net.Listen("tcp", fmt.Sprint(":", s.cfg.ServePort))
	if err != nil {
		return err
	}
	var done = make(chan struct{})
	go s.checkSessionsTimeout(done)

	s.server = &http.Server{Handler: s}

	events.Subscribe(s.onEvent)
	err = s.server.Serve(listener)
	if err != nil {
		return err
	}

	done <- struct{}{}

	return nil
}

func (s *Server) Stop() error {
	if s.server != nil {
		return s.server.Shutdown(context.Background())
	}

	return fmt.Errorf("server not started")
}

func (s *Server) GetSessionList() *SessionList {
	return s.sessions
}

func (s *Server) hearBeat(params htp.Params) (interface{}, error) {
	return params["Userid"], nil
}

func (s *Server) getSessionCount(htp.Params) (interface{}, error) {
	return len(s.sessions.list), nil
}

func (s *Server) checkSessionsTimeout(done chan struct{}) {
	ticker := time.NewTicker(s.cfg.HeartbeatInterval)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			var closeList []*Session
			s.sessions.ForEach(func(se *Session) {
				if se.IsTimeout() {
					s.response(se.id, "", nil, ErrSessionExpired)
					closeList = append(closeList, se)
				}
			})
			for _, se := range closeList {
				s.sessions.CloseSession(se)
			}
		case <-done:
			return
		}
	}

}

func (s *Server) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	conn, err := s.Upgrade(w, r, nil)
	if err != nil {
		log.Error("websocket Upgrader: ", err)
		return
	}
	defer conn.Close()

	session := s.sessions.NewSession(conn)

	defer s.sessions.CloseSession(session)

	for {
		_, bysMsg, err := conn.ReadMessage()
		if err == nil {
			if s.handle(session, bysMsg, r) {
				session.lastActive = time.Now().Unix()
			}
			continue
		}
		e, ok := err.(net.Error)
		if !ok || !e.Timeout() {
			log.Error("websocket conn:", err)
			return
		}
	}
}

func (s *Server) handle(session *Session, data []byte, r *http.Request) bool {

	var req = make(map[string]interface{})

	if err := json.Unmarshal(data, &req); err != nil {
		s.response(session.id, "", nil, ErrIllegalDataFormat)
		log.Error("websocket OnDataHandle:", err)
		return false
	}
	action := req["Action"].(string)

	handler, ok := s.handlers[action]
	if !ok {
		s.response(session.id, "", nil, ErrInvalidMethod)
		return false
	}

	result, err := handler(req)

	s.response(session.id, action, result, err)

	return true
}

func (s *Server) response(id, action string, result interface{}, err error) {
	resp := Response{
		Action: action,
		Result: result,
		Error:  0,
		Desc:   "Success",
	}

	if err != nil {
		switch e := err.(type) {
		case *htp.Error:
			resp.Error = e.Code
			resp.Desc = e.Message

		default:
			resp.Error = http.StatusInternalServerError
			resp.Desc = err.Error()
		}
	}

	data, err := json.Marshal(resp)
	if err != nil {
		log.Fatalf("HTTP Handle - json.Marshal: %v", err)
		return
	}
	s.sessions.list[id].Send(data)
}

func (s *Server) onEvent(event *events.Event) {
	switch event.Type {
	case events.ETBlockConnected:
		go s.broadcast(event.Data)

	case events.ETTransactionAccepted:
		go s.broadcast(event.Data)

	}
}

func (s *Server) broadcast(v interface{}) {
	var action string
	var result interface{}

	if block, ok := v.(*types.Block); ok {
		if s.cfg.Flags&PFBlock == PFBlock {
			action = "sendblock"
			result = service.GetBlockInfo(s.cfg.ServiceCfg, block, true)

		} else if s.cfg.Flags&PFRawBlock == PFRawBlock {
			action = "sendrawblock"
			w := bytes.NewBuffer(nil)
			block.Serialize(w)
			result = hex.EncodeToString(w.Bytes())
		}

		if s.cfg.Flags&PFBlockTxs == PFBlockTxs {
			action = "sendblocktransactions"
			txs := make([]*service.TransactionInfo, 0, len(block.Transactions))
			for _, tx := range block.Transactions {
				txs = append(txs, service.GetTransactionInfo(s.cfg.ServiceCfg,
					block.Header, tx))
			}
			result = txs
		}

	} else if tx, ok := v.(*types.Transaction); ok {
		if s.cfg.Flags&PFNewTx == PFNewTx {
			action = "sendnewtransaction"
			result = service.GetTransactionInfo(s.cfg.ServiceCfg, nil, tx)
		}
	}

	s.sessions.ForEach(func(v *Session) {
		s.response(v.id, action, result, nil)
	})
}

func NewServer(orgCfg *Config) *Server {
	cfg := *orgCfg
	if cfg.HeartbeatInterval <= 0 {
		cfg.HeartbeatInterval = defaultHeartbeatInterval
	}

	s := Server{
		cfg:      cfg,
		sessions: newSessionList(),
	}

	s.Upgrader.CheckOrigin = func(r *http.Request) bool {
		return true
	}

	s.handlers = map[string]Handler{
		"getconnectioncount": cfg.Service.GetConnectionCount,
		"getblockbyheight":   cfg.Service.GetBlockByHeight,
		"getblockbyhash":     cfg.Service.GetBlockByHash,
		"getblockheight":     cfg.Service.GetBlockHeight,
		"gettransaction":     cfg.Service.GetTransactionByHash,
		"getasset":           cfg.Service.GetAssetByHash,
		"getunspendoutput":   cfg.Service.GetUnspendsByAddr,
		"sendrawtransaction": cfg.Service.SendRawTransaction,
		"heartbeat":          s.hearBeat,
		"getsessioncount":    s.getSessionCount,
	}

	return &s
}
