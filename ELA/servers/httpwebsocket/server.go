package httpwebsocket

import (
	"context"
	"crypto/tls"
	"encoding/json"
	"net"
	"net/http"
	"strconv"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/common/config"
	"github.com/elastos/Elastos.ELA/common/log"
	. "github.com/elastos/Elastos.ELA/core/types"
	. "github.com/elastos/Elastos.ELA/errors"
	"github.com/elastos/Elastos.ELA/events"
	. "github.com/elastos/Elastos.ELA/servers"

	"github.com/gorilla/websocket"
	"github.com/pborman/uuid"
)

const (
	// sessionTimeout is the duration of inactivity before we time out a session.
	sessionTimeout = time.Minute
)

var instance *WebSocketServer

var (
	PushBlockFlag    = true
	PushRawBlockFlag = true
	PushBlockTxsFlag = true
	PushNewTxsFlag   = true
)

type Handler func(Params) map[string]interface{}

type WebSocketServer struct {
	sync.RWMutex
	*http.Server
	net.Listener
	websocket.Upgrader

	SessionList *SessionList
	ActionMap   map[string]Handler
}

func StartServer() {
	events.Subscribe(func(e *events.Event) {
		switch e.Type {
		case events.ETBlockConnected:
			SendBlock2WSclient(e.Data)

		case events.ETTransactionAccepted:
			SendTransaction2WSclient(e.Data)

		}
	})

	instance = &WebSocketServer{
		Upgrader:    websocket.Upgrader{},
		SessionList: &SessionList{OnlineList: make(map[string]*Session)},
	}
	instance.Start()

}

func (server *WebSocketServer) Start() {
	server.initializeMethods()
	server.Upgrader.CheckOrigin = func(r *http.Request) bool { return true }

	if config.Parameters.HttpWsPort%1000 == TlsPort {
		var err error
		server.Listener, err = server.initTlsListen()
		if err != nil {
			log.Error("Https Cert: ", err.Error())
		}
	} else {
		var err error
		server.Listener, err = net.Listen("tcp", ":"+strconv.Itoa(config.Parameters.HttpWsPort))
		if err != nil {
			log.Fatal("net.Listen: ", err.Error())
		}
	}
	var done = make(chan bool)
	go server.checkSessionsTimeout(done)

	server.Server = &http.Server{Handler: http.HandlerFunc(server.webSocketHandler)}
	err := server.Serve(server.Listener)

	done <- true
	if err != nil {
		log.Fatal("ListenAndServe: ", err.Error())
	}
}

func (server *WebSocketServer) initializeMethods() {
	server.ActionMap = map[string]Handler{
		"getconnectioncount": GetConnectionCount,
		"getblockbyheight":   GetBlockByHeight,
		"getblockbyhash":     GetBlockByHash,
		"getblockheight":     GetBlockHeight,
		"gettransaction":     GetTransactionByHash,
		"getasset":           GetAssetByHash,
		"getunspendoutput":   GetUnspendOutput,
		"sendrawtransaction": SendRawTransaction,
		"heartbeat":          server.heartBeat,
		"getsessioncount":    server.getSessionCount,
	}
}

func (server *WebSocketServer) heartBeat(cmd Params) map[string]interface{} {
	return ResponsePack(Success, "123")
}

func (server *WebSocketServer) getSessionCount(cmd Params) map[string]interface{} {
	return ResponsePack(Success, len(server.SessionList.OnlineList))
}

func (server *WebSocketServer) Stop() {
	server.Shutdown(context.Background())
	log.Info("Close websocket ")
}

func (server *WebSocketServer) checkSessionsTimeout(done chan bool) {
	ticker := time.NewTicker(sessionTimeout)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			var closeList []*Session
			server.SessionList.ForEachSession(func(v *Session) {
				if v.SessionTimeoverCheck() {
					resp := ResponsePack(SessionExpired, "")
					server.response(v.SessionID, resp)
					closeList = append(closeList, v)
				}
			})
			for _, s := range closeList {
				server.SessionList.CloseSession(s)
			}
		case <-done:
			return
		}
	}

}

//webSocketHandler
func (server *WebSocketServer) webSocketHandler(w http.ResponseWriter, r *http.Request) {
	wsConn, err := server.Upgrader.Upgrade(w, r, nil)

	if err != nil {
		log.Error("websocket Upgrader: ", err)
		return
	}
	defer wsConn.Close()

	newSession := &Session{
		Connection: wsConn,
		LastActive: time.Now().Unix(),
		SessionID:  uuid.NewUUID().String(),
	}
	server.SessionList.OnlineList[newSession.SessionID] = newSession

	defer func() {
		server.SessionList.CloseSession(newSession)
	}()

	for {
		_, bysMsg, err := wsConn.ReadMessage()
		if err == nil {
			if server.OnDataHandle(newSession, bysMsg, r) {
				newSession.LastActive = time.Now().Unix()
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

func (server *WebSocketServer) IsValidMsg(reqMsg map[string]interface{}) bool {
	if _, ok := reqMsg["Hash"].(string); !ok && reqMsg["Hash"] != nil {
		return false
	}
	if _, ok := reqMsg["Addr"].(string); !ok && reqMsg["Addr"] != nil {
		return false
	}
	if _, ok := reqMsg["AssetId"].(string); !ok && reqMsg["AssetId"] != nil {
		return false
	}
	return true
}

func (server *WebSocketServer) OnDataHandle(currentSession *Session, bysMsg []byte, r *http.Request) bool {

	var req = make(map[string]interface{})

	if err := json.Unmarshal(bysMsg, &req); err != nil {
		resp := ResponsePack(IllegalDataFormat, "")
		server.response(currentSession.SessionID, resp)
		log.Error("websocket OnDataHandle:", err)
		return false
	}
	actionName := req["Action"].(string)

	action, ok := server.ActionMap[actionName]
	if !ok {
		resp := ResponsePack(InvalidMethod, "")
		server.response(currentSession.SessionID, resp)
		return false
	}
	if !server.IsValidMsg(req) {
		resp := ResponsePack(InvalidParams, "")
		server.response(currentSession.SessionID, resp)
		return true
	}
	if height, ok := req["Height"].(float64); ok {
		req["Height"] = strconv.FormatInt(int64(height), 10)
	}
	if raw, ok := req["Raw"].(float64); ok {
		req["Raw"] = strconv.FormatInt(int64(raw), 10)
	}

	resp := action(req)
	resp["Action"] = actionName

	server.response(currentSession.SessionID, resp)

	return true
}

func (server *WebSocketServer) response(sessionID string, resp map[string]interface{}) {
	resp["Desc"] = ErrMap[resp["Error"].(ErrCode)]
	data, err := json.Marshal(resp)
	if err != nil {
		log.Error("Websocket response:", err)
		return
	}
	server.SessionList.OnlineList[sessionID].Send(data)
}

func SendTransaction2WSclient(v interface{}) {
	if PushNewTxsFlag {
		go func() {
			instance.PushResult("sendnewtransaction", v)
		}()
	}
}

func SendBlock2WSclient(v interface{}) {
	//if PushBlockFlag {
	//	go func() {
	//		instance.PushResult("sendblock", v)
	//	}()
	//}
	if PushRawBlockFlag {
		go func() {
			instance.PushResult("sendrawblock", v)
		}()
	}
	if PushBlockTxsFlag {
		go func() {
			instance.PushResult("sendblocktransactions", v)
		}()
	}
}

func (server *WebSocketServer) PushResult(action string, v interface{}) {
	var result interface{}
	switch action {
	case "sendblock", "sendrawblock":
		if block, ok := v.(*Block); ok {
			result = GetBlockInfo(block, true)
		}
		//case "sendrawblock":
		//	if block, ok := v.(*Block); ok {
		//		w := bytes.NewBuffer(nil)
		//		block.Serialize(w)
		//		result = BytesToHexString(w.Bytes())
		//	}
	case "sendblocktransactions":
		if block, ok := v.(*Block); ok {
			result = GetBlockTransactions(block)
		}
	case "sendnewtransaction":
		if tx, ok := v.(*Transaction); ok {
			result = GetTransactionInfo(nil, tx)
		}
	default:
		log.Error("httpwebsocket/server.go in pushresult function: unknown action")
	}

	resp := ResponsePack(Success, result)
	resp["Action"] = action

	data, err := json.Marshal(resp)
	if err != nil {
		log.Error("Websocket PushResult:", err)
		return
	}
	server.broadcast(data)
}

func (server *WebSocketServer) broadcast(data []byte) error {
	server.SessionList.ForEachSession(func(v *Session) {
		v.Send(data)
	})
	return nil
}

func (server *WebSocketServer) initTlsListen() (net.Listener, error) {

	CertPath := config.Parameters.RestCertPath
	KeyPath := config.Parameters.RestKeyPath

	// load cert
	cert, err := tls.LoadX509KeyPair(CertPath, KeyPath)
	if err != nil {
		log.Error("load keys fail", err)
		return nil, err
	}

	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
	}

	log.Info("TLS listen port is ", strconv.Itoa(config.Parameters.HttpWsPort))
	listener, err := tls.Listen("tcp", ":"+strconv.Itoa(config.Parameters.HttpWsPort), tlsConfig)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	return listener, nil
}
