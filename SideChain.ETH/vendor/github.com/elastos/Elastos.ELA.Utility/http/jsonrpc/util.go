package jsonrpc

import (
	"bytes"
	"encoding/json"
	"io/ioutil"
	"net/http"

	"github.com/elastos/Elastos.ELA.Utility/http/util"
)

// Call is a util method to send a JSON-RPC request to server.
func Call(url string, req Request) (interface{}, error) {
	data, err := json.Marshal(req)
	if err != nil {
		return nil, err
	}

	resp, err := http.Post(url, "application/json", bytes.NewReader(data))
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()

	body, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	ret := Response{}
	if err := json.Unmarshal(body, &ret); err != nil {
		return nil, err
	}

	if ret.Error != nil {
		return nil, ret.Error
	}

	return ret.Result, nil
}

// CallParams is a util method to send a JSON-RPC request to server.
func CallParams(url, method string, params util.Params) (interface{}, error) {
	req := Request{
		Method: method,
		Params: params,
	}
	return Call(url, req)
}

// CallArray is a util method to send a JSON-RPC request to server.
func CallArray(url, method string, params... interface{}) (interface{}, error) {
	req := Request{
		Method: method,
		Params: params,
	}
	return Call(url, req)
}
