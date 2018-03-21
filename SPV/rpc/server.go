package rpc

import (
	"net/http"
	"io/ioutil"
	"encoding/json"

	tx "SPVWallet/core/transaction"
	"SPVWallet/log"
	"os"
)

type Listeners struct {
	AddToFilter     func(hash []byte) error
	SendTransaction func(tx.Transaction) error
}

var listeners *Listeners

func InitServer(ls *Listeners) *Server {
	listeners = ls
	server := &Server{
		Server: http.Server{Addr: ":" + RPCPort},
		methods: map[string]func(Req) Resp{
			"addtofilter":     AddToFilter,
			"sendtransaction": SendTransaction,
		},
	}
	http.HandleFunc("/", server.handle)
	return server
}

type Server struct {
	http.Server
	methods map[string]func(Req) Resp
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
