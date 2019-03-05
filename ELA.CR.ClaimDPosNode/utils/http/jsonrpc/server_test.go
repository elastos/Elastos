package jsonrpc

import (
	"testing"
	"github.com/elastos/Elastos.ELA/utils/test"
	"time"
	htp "github.com/elastos/Elastos.ELA/utils/http"
	"bytes"
	"net/http"
	"github.com/elastos/Elastos.ELA/common/log"
	//"encoding/json"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
)

var (
	logger = log.NewDefault(1, 10000, 10000)
)
/*
hope： if no init RpcConfiguration. ip only accept IsLoopback  localhost
testcase1 : RpcConfiguration no init client no authorization
testcase1 : RpcConfiguration no init client have authorization

testcase2 : RpcConfiguration have User and Pass no ip.  User and Pass are correct
testcase2 : RpcConfiguration have User and Pass no ip.  User and Pass are wrong
testcase2 : RpcConfiguration have User and Pass no ip.  User is wrong
testcase2 : RpcConfiguration have User and Pass no ip.  Pass is  wrong
testcase2 : RpcConfiguration have User and Pass no ip.  client no authorization


testcase3 : RpcConfiguration have ip no User and Pass. Ip is allowed
testcase3 : RpcConfiguration have ip no User and Pass. Ip is forbid    *

testcase4 : RpcConfiguration have ip  User and Pass. All  is correct
testcase4 : RpcConfiguration have ip  User and Pass. ip  is wrong      *
testcase4 : RpcConfiguration have ip  User and Pass. User  is wrong
testcase4 : RpcConfiguration have ip  User and Pass. Pass  is wrong


testcase5 : user input localhost instead ip


*/
func TestServer_NotInitRpcConf(t *testing.T) {

	//t.Logf("TestServer_NotInitRpcConf begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		//RpcConfiguration:  RpcConfiguration{
		//	User:        "ElaUser",
		//	Pass:        "Ela123",
		//	WhiteIPList: []string{
		//		"127.0.0.1",
		//	},
		//},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		//t.Logf("client_side Receive POST request from path %s, data %v", "/api/test",data)
		////t.Logf("client_side Receive POST request len(data) %d",len(data))
		//for key,value := range data{
		//	t.Logf("client_side Receive POST key %v value %v",key, value)
		//
		//}
		//assert.NoError(t, err)
		return nil, nil
	}, "level")
	//assert.NoError(t, err)

	go s.Start()

	//////////

	type Message struct {
		Name string
		Body string
		Time int64
	}
	m := Message{"Alice", "Hello", 1294706395881547000}
	b, err22 := json.Marshal(m)
	t.Logf("json.Marshal-------   %s errw %v", b, err22)

	/////////

	type Student struct {
		name  string
		age     int
	}

	type ReqObj struct{
		method string
		params Student
	}

	//reqObj := ReqObj{
	//	method: "/api/test",
	//	params: Student{
	//		name: "junneyang",
	//		age:  88,
	//	},
	//}
	var reqObj ReqObj;
	//data ,errw := json.Marshal(reqObj)
	//t.Logf("json.Marshal-------   %s errw %v", data, errw)

	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	//var someOne ReqObj
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v",reqObj)
		//fmt.Printf("someOne %v",someOne)

	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))

	/* request with no authorization */
	go func() {
		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")

		_, err5 := client.Do(req)
		t.Logf("no authorization request   %v", err5)

	}()

	/* request with  authorization */
	go func() {
		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")

		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass);
		resp, err2 := client.Do(req)
		t.Logf("authorization request  err  %v StatusCode %v", err2, resp.StatusCode)
		assert.NoError(t, err)
		if resp.StatusCode != http.StatusOK {
			t.Error("expecting not found get %", resp.Status)
		}

	}()

	////////////////
	select {
	case <-time.After(time.Second*1):
		s.Stop()
	}
}


func TestServer_WithAuth(t *testing.T) {
	return ;
	t.Logf("client_side TestServer_WithAuth begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		RpcConfiguration:  RpcConfiguration{
			User:        "ElaUser",
			Pass:        "Ela123",
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test",data)
		//fmt.Fprintf(w,)
		//t.Logf("client_side data  %s", data)

		//if !bytes.Equal(data, []byte("data")) {
		//	t.Errorf("expected data = data get %s", data)
		//}
		ok := "/api/test  return ok!!!!!! "
		return ok, nil
	}, "level")
	//assert.NoError(t, err)

	go s.Start()

	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	req_new := bytes.NewBuffer([]byte(reqStr))

	/*_, err := http.Post("http://localhost:20336/api/test", "application/json",
		req_new)//bytes.NewReader([]byte("data"))
	if !assert.NoError(t, err) {
		t.FailNow()
	}*/


	/////////////////
	client := &http.Client{}
	req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
	if err != nil {
		// handle error
	}
	req.Header.Set("Content-Type", "application/json")

	//login := s.cfg.RpcConfiguration.User  + ":" + s.cfg.RpcConfiguration.Pass
	//t.Logf("  login-------  %v", login)

	//auth := "Basic " + base64.StdEncoding.EncodeToString([]byte(login))
	//requestHeader := make(http.Header)
	//req.Header.Add("Authorization", auth)
	//req.Header.Set("Authorization", auth)
	req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass);
	//t.Logf(" client_side req.Header!!!!!!!!  %v", req.Header)
	res, err5 := client.Do(req)
	assert.NoError(t, err)
	t.Logf("client_side client.Do err  %v res %v", err5, res.Status)

	//////////////// d
	select {
	case <-time.After(time.Second*1):
		s.Stop()
	}
}


//func TestServer_ServeHTTP(t *testing.T) {
//
//	t.Logf("TestServer_ServeHTTP")
//
//	test.SkipShort(t)
//
//	s := NewServer(&Config{
//		ServePort: 20336,
//		RpcConfiguration:  RpcConfiguration{
//			User:        "ElaUser",
//			Pass:        "Ela123",
//			WhiteIPList: []string{
//						 "127.0.0.1",
//		},
//	},
//	})
//
//	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
//		t.Logf("Receive POST request from path %s, data ", "/api/test")
//		t.Logf("  data  %s", data)
//
//		//if !bytes.Equal(data, []byte("data")) {
//		//	t.Errorf("expected data = data get %s", data)
//		//}
//		return nil, nil
//	}, "level")
//	//assert.NoError(t, err)
//
//	go s.Start()
//
//	req := `{
//	"method":"/api/test",
//	"params":{"name":"junneyang", "age": 88}
//	}`
//	req_new := bytes.NewBuffer([]byte(req))
//	_, err := http.Post("http://localhost:20336/api/test", "application/json",
//		req_new)//bytes.NewReader([]byte("data"))
//	if !assert.NoError(t, err) {
//		t.FailNow()
//	}
//
//	select {
//	case <-time.After(time.Second*1):
//		s.Stop()
//	}
//}
