package httpjsonrpc

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"

	"github.com/elastos/Elastos.ELA.SideChain/servers"
)

const (
	// JSON-RPC protocol error codes.
	ParseError     = -32700
	InvalidRequest = -32600
	MethodNotFound = -32601
	InvalidParams  = -32602
	InternalError  = -32603
	//-32000 to -32099	Server error, waiting for defining
)

type rpcserver struct {
	port    uint16
	service *servers.HttpService
	mux     map[string]func(servers.Params) map[string]interface{}
}

func New(port uint16, service *servers.HttpService) *rpcserver {
	return &rpcserver{
		port:    port,
		service: service,
		mux:     make(map[string]func(servers.Params) map[string]interface{}),
	}
}

func (s *rpcserver) RegisterAction(name string, action func(servers.Params) map[string]interface{}) {
	s.mux[name] = action
}

func (s *rpcserver) Start() {
	s.RegisterAction("setloglevel", s.service.SetLogLevel)
	s.RegisterAction("getinfo", s.service.GetInfo)
	s.RegisterAction("getblock", s.service.GetBlockByHash)
	s.RegisterAction("getcurrentheight", s.service.GetBlockHeight)
	s.RegisterAction("getblockhash", s.service.GetBlockHash)
	s.RegisterAction("getconnectioncount", s.service.GetConnectionCount)
	s.RegisterAction("getrawmempool", s.service.GetTransactionPool)
	s.RegisterAction("getrawtransaction", s.service.GetRawTransaction)
	s.RegisterAction("getneighbors", s.service.GetNeighbors)
	s.RegisterAction("getnodestate", s.service.GetNodeState)
	s.RegisterAction("sendtransactioninfo", s.service.SendTransactionInfo)
	s.RegisterAction("sendrawtransaction", s.service.SendRawTransaction)
	s.RegisterAction("getbestblockhash", s.service.GetBestBlockHash)
	s.RegisterAction("getblockcount", s.service.GetBlockCount)
	s.RegisterAction("getblockbyheight", s.service.GetBlockByHeight)
	s.RegisterAction("getdestroyedtransactions", s.service.GetDestroyedTransactionsByHeight)
	s.RegisterAction("getexistdeposittransactions", s.service.GetExistDepositTransactions)
	s.RegisterAction("help", s.service.AuxHelp)
	s.RegisterAction("submitsideauxblock", s.service.SubmitSideAuxBlock)
	s.RegisterAction("createauxblock", s.service.CreateAuxBlock)
	s.RegisterAction("togglemining", s.service.ToggleMining)
	s.RegisterAction("discretemining", s.service.DiscreteMining)

	http.HandleFunc("/", s.handle)
	err := http.ListenAndServe(fmt.Sprint(":", s.port), nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err.Error())
	}
}

//this is the funciton that should be called in order to answer an rpc call
//should be registered like "http.AddMethod("/", httpjsonrpc.Handle)"
func (s *rpcserver) handle(w http.ResponseWriter, r *http.Request) {
	//JSON RPC commands should be POSTs
	if r.Method != "POST" {
		log.Warn("HTTP JSON RPC Handle - Method!=\"POST\"")
		http.Error(w, "JSON RPC procotol only allows POST method", http.StatusMethodNotAllowed)
		return
	}

	if r.Header["Content-Type"][0] != "application/json" {
		http.Error(w, "need content type to be application/json", http.StatusUnsupportedMediaType)
		return
	}

	//read the body of the request
	body, _ := ioutil.ReadAll(r.Body)
	request := make(map[string]interface{})
	error := json.Unmarshal(body, &request)
	if error != nil {
		log.Error("HTTP JSON RPC Handle - json.Unmarshal: ", error)
		responseError(w, http.StatusBadRequest, ParseError, "rpc json parse error:"+error.Error())
		return
	}
	//get the corresponding function
	requestMethod, ok := request["method"].(string)
	if !ok {
		responseError(w, http.StatusBadRequest, InvalidRequest, "need a method!")
		return
	}
	method, ok := s.mux[requestMethod]
	if !ok {
		responseError(w, http.StatusNotFound, MethodNotFound, "method "+requestMethod+" not found")
		return
	}

	requestParams := request["params"]
	//Json rpc 1.0 support positional parameters while json rpc 2.0 support named parameters.
	// positional parameters: { "requestParams":[1, 2, 3....] }
	// named parameters: { "requestParams":{ "a":1, "b":2, "c":3 } }
	//Here we support both of them, because bitcion does so.
	var params servers.Params
	switch requestParams := requestParams.(type) {
	case nil:
	case []interface{}:
		params = convertParams(requestMethod, requestParams)
	case map[string]interface{}:
		params = servers.Params(requestParams)
	default:
		responseError(w, http.StatusBadRequest, InvalidRequest, "params format error, must be an array or a map")
		return
	}

	response := method(params)
	var data []byte
	if response["Error"] != servers.ErrorCode(0) {
		data, _ = json.Marshal(map[string]interface{}{
			"jsonrpc": "2.0",
			"error": map[string]interface{}{
				"code":    response["Error"],
				"message": response["Result"],
				"id":      request["id"],
			},
		})

	} else {
		data, _ = json.Marshal(map[string]interface{}{
			"jsonrpc": "2.0",
			"result":  response["Result"],
			"id":      request["id"],
			"error":   nil,
		})
	}
	w.Write(data)
}

func responseError(w http.ResponseWriter, httpStatus int, code servers.ErrorCode, message string) {
	w.WriteHeader(httpStatus)
	data, _ := json.Marshal(map[string]interface{}{
		"jsonrpc": "2.0",
		"error": map[string]interface{}{
			"code":    code,
			"message": message,
			"id":      nil,
		},
	})
	w.Write(data)
}

func convertParams(method string, params []interface{}) servers.Params {
	switch method {
	case "createauxblock":
		return servers.FromArray(params, "paytoaddress")
	case "submitsideauxblock":
		return servers.FromArray(params, "blockhash", "auxpow")
	case "getblockhash":
		return servers.FromArray(params, "index")
	case "getblock":
		return servers.FromArray(params, "hash", "format")
	case "setloglevel":
		return servers.FromArray(params, "level")
	case "getrawtransaction":
		return servers.FromArray(params, "hash", "decoded")
	case "getarbitratorgroupbyheight":
		return servers.FromArray(params, "height")
	case "togglemining":
		return servers.FromArray(params, "mine")
	case "discretemining":
		return servers.FromArray(params, "count")
	default:
		return servers.Params{}
	}
}
