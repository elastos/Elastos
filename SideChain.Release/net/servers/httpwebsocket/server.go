package httpwebsocket

import (
	"net"
	"sync"
	"time"
	"bytes"
	"strconv"
	"context"
	"net/http"
	"crypto/tls"
	"encoding/json"

	"Elastos.ELA.SideChain/events"
	. "Elastos.ELA.SideChain/common"
	. "Elastos.ELA.SideChain/errors"
	"Elastos.ELA.SideChain/common/log"
	"Elastos.ELA.SideChain/core/ledger"
	. "Elastos.ELA.SideChain/net/servers"
	. "Elastos.ELA.SideChain/common/config"
	"Elastos.ELA.SideChain/core/transaction"

	"github.com/pborman/uuid"
	"github.com/gorilla/websocket"
)

var (
	PushBlockFlag    = true
	PushRawBlockFlag = false
	PushBlockTxsFlag = false
	PushNewTxsFlag   = true
)

type Handler func(map[string]interface{}) map[string]interface{}

type WebSocketServer struct {
	sync.RWMutex
	*http.Server
	net.Listener
	websocket.Upgrader

	SessionList *SessionList
	ActionMap   map[string]Handler
}

func StartServer() {
	server := &WebSocketServer{
		Upgrader:    websocket.Upgrader{},
		SessionList: &SessionList{OnlineList: make(map[string]*Session)},
	}

	ledger.DefaultLedger.Blockchain.BCEvents.Subscribe(
		events.EventBlockPersistCompleted, server.pushBlockToClients)
	ledger.DefaultLedger.Blockchain.BCEvents.Subscribe(
		events.EventNewTransactionPutInPool, server.pushNewTxToClients)

	go server.Start()
}

func (server *WebSocketServer) Start() {
	server.initializeMethods()
	server.Upgrader.CheckOrigin = func(r *http.Request) bool { return true }

	if Parameters.HttpWsPort%1000 == TlsPort {
		var err error
		server.Listener, err = server.initTlsListen()
		if err != nil {
			log.Error("Https Cert: ", err.Error())
		}
	} else {
		var err error
		server.Listener, err = net.Listen("tcp", ":"+strconv.Itoa(Parameters.HttpWsPort))
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
		"heartbeat":          server.hearBeat,
		"getsessioncount":    server.getSessionCount,
	}
}

func (server *WebSocketServer) hearBeat(cmd map[string]interface{}) map[string]interface{} {
	return ResponsePack(Success, cmd["UserId"])
}

func (server *WebSocketServer) getSessionCount(cmd map[string]interface{}) map[string]interface{} {
	return ResponsePack(Success, len(server.SessionList.OnlineList))
}

func (server *WebSocketServer) Stop() {
	server.Shutdown(context.Background())
	log.Info("Close websocket ")
}

func (server *WebSocketServer) checkSessionsTimeout(done chan bool) {
	ticker := time.NewTicker(time.Second * Parameters.Configuration.WsHeartbeatInterval)
	defer ticker.Stop()
	for {
		select {
		case <-ticker.C:
			var closeList []*Session
			server.SessionList.ForEachSession(func(v *Session) {
				if v.IsTimeOut() {
					resp := ResponsePack(SessionExpired, "")
					server.response(v.SessionId, resp)
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

	session := &Session{
		Connection: wsConn,
		LastActive: time.Now().Unix(),
		SessionId:  uuid.NewUUID().String(),
	}
	server.SessionList.OnlineList[session.SessionId] = session

	defer server.SessionList.CloseSession(session)

	for {
		_, bysMsg, err := wsConn.ReadMessage()
		if err == nil {
			if server.OnDataHandle(session, bysMsg, r) {
				session.LastActive = time.Now().Unix()
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
		server.response(currentSession.SessionId, resp)
		log.Error("websocket OnDataHandle:", err)
		return false
	}
	actionName := req["Action"].(string)

	action, ok := server.ActionMap[actionName]
	if !ok {
		resp := ResponsePack(InvalidMethod, "")
		server.response(currentSession.SessionId, resp)
		return false
	}
	if !server.IsValidMsg(req) {
		resp := ResponsePack(InvalidParams, "")
		server.response(currentSession.SessionId, resp)
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

	server.response(currentSession.SessionId, resp)

	return true
}

func (server *WebSocketServer) response(sessionId string, resp map[string]interface{}) {
	resp["Desc"] = ErrMap[resp["Error"].(ErrCode)]
	data, err := json.Marshal(resp)
	if err != nil {
		log.Error("Websocket response:", err)
		return
	}
	server.SessionList.OnlineList[sessionId].Send(data)
}

func (server *WebSocketServer) pushNewTxToClients(v interface{}) {
	if PushNewTxsFlag {
		if trx, ok := v.(*transaction.Transaction); ok {
			go server.broadcast(GetTransactionInfo(trx))
		}
	}
}

func (server *WebSocketServer) pushBlockToClients(v interface{}) {
	if block, ok := v.(*ledger.Block); ok {
		if PushBlockFlag {
			go server.broadcast(GetBlockInfo(block))
		}
		if PushRawBlockFlag {
			go func() {
				buf := new(bytes.Buffer)
				block.Serialize(buf)
				server.broadcast(BytesToHexString(buf.Bytes()))
			}()
		}
		if PushBlockTxsFlag {
			go server.broadcast(GetBlockTransactions(block))
		}
	}
}

func (server *WebSocketServer) broadcast(result interface{}) {
	resp := ResponsePack(Success, result)

	data, err := json.Marshal(resp)
	if err != nil {
		log.Error("Websocket PushResult:", err)
		return
	}

	server.SessionList.ForEachSession(func(session *Session) {
		session.Send(data)
	})
}

func (server *WebSocketServer) initTlsListen() (net.Listener, error) {

	CertPath := Parameters.RestCertPath
	KeyPath := Parameters.RestKeyPath

	// load cert
	cert, err := tls.LoadX509KeyPair(CertPath, KeyPath)
	if err != nil {
		log.Error("load keys fail", err)
		return nil, err
	}

	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
	}

	log.Info("TLS listen port is ", strconv.Itoa(Parameters.HttpWsPort))
	listener, err := tls.Listen("tcp", ":"+strconv.Itoa(Parameters.HttpWsPort), tlsConfig)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	return listener, nil
}
