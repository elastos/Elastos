package httpjsonrpc

import (
	"strconv"
	"net/http"

	"Elastos.ELA/common/log"
	. "Elastos.ELA/common/config"
	. "Elastos.ELA/net/servers"
	"io/ioutil"
	"encoding/json"
	"Elastos.ELA/errors"
)

//an instance of the multiplexer
var mainMux map[string]func(map[string]interface{}) map[string]interface{}

const (
	// JSON-RPC protocol error codes.
	ParseError     = -32700
	InvalidRequest = -32600
	MethodNotFound = -32601
	InvalidParams  = -32602
	InternalError  = -32603
	//-32000 to -32099	Server error, waiting for defining
)

func StartRPCServer() {
	mainMux = make(map[string]func(map[string]interface{}) map[string]interface{})

	http.HandleFunc("/", Handle)

	mainMux["setloglevel"] = SetLogLevel
	mainMux["getblock"] = GetBlockByHash
	mainMux["getcurrentheight"] = GetCurrentHeight
	mainMux["getblockhash"] = GetBlockHash
	mainMux["getconnectioncount"] = GetConnectionCount
	mainMux["getrawmempool"] = GetTransactionPool
	mainMux["getrawtransaction"] = GetRawTransaction
	mainMux["getneighbors"] = GetNeighbors
	mainMux["getnodestate"] = GetNodeState
	mainMux["sendrawtransaction"] = SendRawTransaction
	mainMux["submitblock"] = SubmitBlock
	mainMux["getarbitratorgroupbyheight"] = GetArbitratorGroupByHeight
	mainMux["getbestblockhash"] = GetBestBlockHash
	mainMux["getblockcount"] = GetBlockCount

	// mining interfaces
	mainMux["getinfo"] = GetInfo
	mainMux["help"] = AuxHelp
	mainMux["submitauxblock"] = SubmitAuxBlock
	mainMux["createauxblock"] = CreateAuxBlock
	mainMux["togglemining"] = ToggleMining
	mainMux["manualmining"] = ManualMining
	mainMux["gettransaction"] = GetTransactionByHash

	err := http.ListenAndServe("127.0.0.1:"+strconv.Itoa(Parameters.HttpJsonPort), nil)
	if err != nil {
		log.Fatal("ListenAndServe: ", err.Error())
	}
}

//this is the funciton that should be called in order to answer an rpc call
//should be registered like "http.AddMethod("/", httpjsonrpc.Handle)"
func Handle(w http.ResponseWriter, r *http.Request) {
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
		RPCError(w, http.StatusBadRequest, ParseError, "rpc json parse error:"+error.Error())
		return
	}
	//get the corresponding function
	requestMethod, ok := request["method"].(string)
	if !ok {
		RPCError(w, http.StatusBadRequest, InvalidRequest, "need a method!")
		return
	}
	method, ok := mainMux[requestMethod]
	if !ok {
		RPCError(w, http.StatusNotFound, MethodNotFound, "method "+requestMethod+" not found")
		return
	}

	params := request["params"]
	//Json rpc 1.0 support positional parameters while json rpc 2.0 support named parameters.
	// positional parameters: { "params":[1, 2, 3....] }
	// named parameters: { "params":{ "a":1, "b":2, "c":3 } }
	//Here we support both of them, because bitcion does so.
	formattedParams, ok := params.(map[string]interface{})

	if !ok {
		if _, ok := params.([]interface{}); ok {
			formattedParams = convertParams(w, request)
		} else if params == nil {
			formattedParams = map[string]interface{}{}
		} else {
			RPCError(w, http.StatusBadRequest, InvalidRequest, "params format error, must be an array or a map")
			return
		}
	}

	response := method(formattedParams)
	var data []byte
	if response["Error"] != errors.ErrCode(0) {
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
		})
	}
	w.Write(data)
}

func RPCError(w http.ResponseWriter, httpStatus int, code errors.ErrCode, message string) {
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

func convertParams(w http.ResponseWriter,request map[string]interface{}) map[string]interface{} {
	method := request["method"].(string)
	params := request["params"].([]interface{})

	switch method {
	case "createauxblock":
		return map[string]interface{}{"paytoaddress": params[0]}
	case "submitauxblock":
		return map[string]interface{}{"blockhash": params[0], "auxpow": params[1]}
	case "getblockhash":
		return map[string]interface{}{"index": params[0]}
	case "getblock":
		return map[string]interface{}{"hash": params[0]}
	case "setloglevel":
		return map[string]interface{}{"level": params[0]}
	case "getrawtransaction":
		return map[string]interface{}{"hash": params[0]}
	case "getarbitratorgroupbyheight":
		return map[string]interface{}{"height": params[0]}
	case "togglemining":
		return map[string]interface{}{"mine": params[0]}
	case "manualmining":
		return map[string]interface{}{"count": params[0]}
	case "gettransaction":
		return map[string]interface{}{"hash": params[0]}
	default:
		return map[string]interface{}{}
	}
}
