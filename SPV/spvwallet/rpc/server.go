package rpc

import (
	"encoding/json"
	"io/ioutil"
	"net/http"
	"os"

	. "github.com/elastos/Elastos.ELA/core"
	"github.com/elastos/Elastos.ELA.SPV/log"
)

type RequestHandler interface {
	NotifyNewAddress(hash []byte)
	SendTransaction(Transaction)
}

func InitServer(handler RequestHandler) *Server {
	server := new(Server)
	server.Server = http.Server{Addr: ":" + RPCPort}
	server.methods = map[string]func(Req) Resp{
		"notifynewaddress": server.NotifyNewAddress,
		"sendtransaction":  server.SendTransaction,
	}
	server.handler = handler
	http.HandleFunc("/spvwallet/", server.handle)
	return server
}

type Server struct {
	http.Server
	methods map[string]func(Req) Resp
	handler RequestHandler
}

func (server *Server) Start() {
	go func() {
		err := server.ListenAndServe()
		if err != nil {
			log.Error("RPC service start failed:", err)
			os.Exit(800)
		}
	}()
	log.Debug("RPC server started...")
}

func (server *Server) handle(w http.ResponseWriter, r *http.Request) {
	resp := server.getResp(r)
	data, err := json.Marshal(resp)
	if err != nil {
		log.Error("Marshal response error: ", err)
	}
	w.Write(data)
}

func (server *Server) getResp(r *http.Request) Resp {
	if r.Method != "POST" {
		return NonPostRequest
	}

	if r.Body == nil {
		return EmptyRequestBody
	}

	//read the body of the request
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		return ReadRequestError
	}

	log.Debug("Receive request:", string(body))

	var req Req
	err = json.Unmarshal(body, &req)
	if err != nil {
		return UnmarshalRequestError
	}

	function, ok := server.methods[req.Method]
	if !ok {
		return InvalidMethod
	}

	return function(req)
}
