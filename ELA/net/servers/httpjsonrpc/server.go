package httpjsonrpc

import (
	"strconv"
	"net/http"
	"io/ioutil"
	"encoding/json"

	"Elastos.ELA/common/log"
	. "Elastos.ELA/common/config"
	. "github.com/elastos/Elastos.ELA/net/servers"
	"github.com/elastos/Elastos.ELA/errors"
)

//an instance of the multiplexer
var mainMux map[string]func(map[string]interface{}) map[string]interface{}

func StartRPCServer() {
	mainMux = make(map[string]func(map[string]interface{}) map[string]interface{})

	http.HandleFunc("/", Handle)

	mainMux["setloglevel"] = SetLogLevel
	mainMux["getblockbyhash"] = GetBlockByHash
	mainMux["getblockbyheight"] = GetBlockByHeight
	mainMux["getcurrentheight"] = GetCurrentHeight
	mainMux["getblockhash"] = GetBlockHash
	mainMux["getconnectioncount"] = GetConnectionCount
	mainMux["gettransactionpool"] = GetTransactionPool
	mainMux["getrawtransaction"] = GetRawTransaction
	mainMux["getneighbors"] = GetNeighbors
	mainMux["getnodestate"] = GetNodeState
	mainMux["sendrawtransaction"] = SendRawTransaction
	mainMux["submitblock"] = SubmitBlock
	mainMux["getarbitratorgroupbyheight"] = GetArbitratorGroupByHeight

	// mining interfaces
	mainMux["getinfo"] = GetInfo
	mainMux["help"] = AuxHelp
	mainMux["submitauxblock"] = SubmitAuxBlock
	mainMux["createauxblock"] = CreateAuxBlock
	mainMux["togglemining"] = ToggleMining
	mainMux["manualmining"] = ManualMining

	// TODO: only listen to localhost
	err := http.ListenAndServe(":"+strconv.Itoa(Parameters.HttpJsonPort), nil)
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
		return
	}

	//check if there is Request Body to read
	if r.Body == nil {
		log.Warn("HTTP JSON RPC Handle - Request body is nil")
		return
	}

	//read the body of the request
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		log.Error("HTTP JSON RPC Handle - ioutil.ReadAll: ", err)
		return
	}
	request := make(map[string]interface{})
	err = json.Unmarshal(body, &request)
	if err != nil {
		log.Error("HTTP JSON RPC Handle - json.Unmarshal: ", err)
		return
	}
	//get the corresponding function
	function, method, ok := checkMethod(request)
	if !ok {
		Error(w, errors.InvalidMethod, method)
		return
	}

	var params map[string]interface{}

	if request["method"] == "createauxblock" || request["method"] == "submitauxblock" {
		params = fixAuxInterfaces(request)
	} else {
		params, ok = checkParams(request)
		if !ok {
			Error(w, errors.InvalidParams, method)
			return
		}

	}


	response := function(params)
	data, err := json.Marshal(map[string]interface{}{
		"jsonpc": "2.0",
		"code":   response["Error"],
		"result": response["Result"],
	})
	if err != nil {
		log.Error("HTTP JSON RPC Handle - json.Marshal: ", err)
		return
	}
	w.Write(data)
}

func checkMethod(request map[string]interface{}) (func(map[string]interface{}) map[string]interface{}, interface{}, bool) {
	method := request["method"]
	if method == nil {
		return nil, method, false
	}
	switch method.(type) {
	case string:
	default:
		return nil, method, false
	}
	function, ok := mainMux[request["method"].(string)]
	if !ok {
		return nil, method, false
	}
	return function, nil, true
}

func checkParams(request map[string]interface{}) (map[string]interface{}, bool) {
	params := request["params"]
	if params == nil {
		return map[string]interface{}{}, true
	}
	switch params.(type) {
	case map[string]interface{}:
		return params.(map[string]interface{}), true
	default:
		return nil, false
	}
	return nil, false
}

func Error(w http.ResponseWriter, code errors.ErrCode, method interface{}) {
	//if the function does not exist
	log.Warn("HTTP JSON RPC Handle - No function to call for ", method)
	data, _ := json.Marshal(map[string]interface{}{
		"jsonpc": "2.0",
		"code":   code,
		"result": code.Message(),
	})
	w.Write(data)
}

func fixAuxInterfaces(request map[string]interface{}) map[string]interface{}{
	method := request["method"].(string)
	params := request["params"].([]interface{})
	if method == "createauxblock" {
		return map[string]interface{}{"paytoaddress": params[0].(string)}
	}

	if method == "submitauxblock" {
		return map[string]interface{}{"blockhash": params[0], "auxpow": params[1]}
	}

	return map[string]interface{}{"no method": "false"}
}