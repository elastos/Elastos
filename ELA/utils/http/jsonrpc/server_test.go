package jsonrpc

import (
	"bytes"
	"github.com/elastos/Elastos.ELA/common/log"
	htp "github.com/elastos/Elastos.ELA/utils/http"
	"github.com/elastos/Elastos.ELA/utils/test"
	"net/http"
	"testing"
	"time"
	//"encoding/json"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
	"net"
)

var (
	logger = log.NewDefault(1, 10000, 10000)
	urlNotLoopBack string
)

func init() {
	ipNotLoopBack := resolveHostIp()
	if ipNotLoopBack == "" {
		//t.Error("expecting not found get %", resp.Status)
		fmt.Printf("ipNotLoopBack  error should exit!!!!!!!!!!!!!!!")
		return
	}
	httpPrefix := "http://"
	httPostfix := ":20336/api/test"

	urlNotLoopBack = httpPrefix + ipNotLoopBack+ httPostfix
	fmt.Printf("Before Test init url %v", urlNotLoopBack)
}

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


testcase5 : whiteiplist 0.0.0.0  allow all ip


*/

/*
func TestServer_NotInitRpcConf1(t *testing.T) {

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
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test",data)
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
		fmt.Printf("reqObj %+v\n",reqObj)
		//fmt.Printf("someOne %v",someOne)

	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))

	//request with no authorization and 127.0.0.1
	go func() {
		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		t.Logf("request with no authorization 11111111111\n")

		resp, err2 := client.Do(req)

		t.Logf("request with no authorization   err2 %v resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with no authorization   status %v",  resp.Status)

			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %", resp.Status)
			}
		}

	}()

	//// request with  authorization
	//go func() {
	//
	//	client := &http.Client{}
	//	req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
	//	if err != nil {
	//		// handle error
	//	}
	//	req.Header.Set("Content-Type", "application/json")
	//
	//	req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass);
	//	t.Logf("request with  authorization 22222222222\n")
	//
	//	resp, err2 := client.Do(req)
	//	t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
	//	//if resp != nil {
	//	//	t.Logf("request with  authorization   status %v",  resp.Status)
	//	//	//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
	//	//	assert.NoError(t, err)
	//	//	if resp.StatusCode != http.StatusOK {
	//	//		t.Error("expecting not found get %v", resp.Status)
	//	//	}
	//	//}
	//
	//
	//}()

	//request with no authorization and localhost
	//go func() {
	//	return
	//	client := &http.Client{}
	//	req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
	//	if err != nil {
	//		// handle error
	//	}
	//	req.Header.Set("Content-Type", "application/json")
	//	t.Logf("request with no authorization 33333333333\n")
	//
	//	resp, err2 := client.Do(req)
	//
	//	t.Logf("request with no authorization and localhost err2 %v %v\n", err2, resp)
	//	//if  resp != nil {
	//	//	t.Logf("no authorization request   status %v", resp.Status)
	//	//
	//	//	if resp.StatusCode != http.StatusOK {
	//	//		t.Error("expecting not found get %", resp.Status)
	//	//	}
	//	//}
	//
	//}()
	////////////////
	select {
	case <-time.After(time.Second*1):
		s.Stop()
	}
}
*/

func GetInternalIP() string {
	itf, _ := net.InterfaceByName("en0") //here your interface
	item, _ := itf.Addrs()
	var ip net.IP
	for _, addr := range item {
		switch v := addr.(type) {
		case *net.IPNet:
			if !v.IP.IsLoopback() {
				if v.IP.To4() != nil {//Verify if IP is IPV4
					ip = v.IP
				}
			}
		}
	}
	if ip != nil {
		return ip.String()
	} else {
		return ""
	}
}
func resolveHostIp() (string) {

	netInterfaceAddresses, err := net.InterfaceAddrs()

	if err != nil { return "" }

	for _, netInterfaceAddress := range netInterfaceAddresses {

		networkIp, ok := netInterfaceAddress.(*net.IPNet)

		if ok && !networkIp.IP.IsLoopback() && networkIp.IP.To4() != nil {

			ip := networkIp.IP.String()

			fmt.Println("Resolved Host IP: " + ip)

			return ip
		}
	}
	return ""
}
//func getLocalIp()string {
//	IpAddr := "127.0.0.1"
//	addrSlice, err := net.InterfaceAddrs()
//	fmt.Printf("getLocalIp %v \n", addrSlice);
//	if nil != err {
//		log.Error("Get local IP addr failed!!!")
//		return IpAddr
//
//	}
//	for _, addr := range addrSlice {
//		if ipnet, ok := addr.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
//			if nil != ipnet.IP.To4() {
//				IpAddr = ipnet.IP.String()
//
//			}
//		}
//	}
//	return IpAddr
//}

func TestServer_NotInitRpcConf1(t *testing.T) {

	t.Logf("NotInitRpcConf1 request with no authorization and 127.0.0.1 begin resolveHostIp %s", urlNotLoopBack)

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)

		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	//var someOne ReqObj
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))

	//request with no authorization and 127.0.0.1
	go func() {
		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		resp, err2 := client.Do(req)

		t.Logf("request with no authorization   err2 %v resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with no authorization   status %v", resp.Status)

			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %", resp.Status)
			}
		}

	}()
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}
func TestServer_NotInitRpcConf2(t *testing.T) {

	t.Logf(" NotInitRpcConf2 request with  authorization  and 127.0.0.1 begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")
	//assert.NoError(t, err)

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	//var someOne ReqObj
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))

	// request with  authorization  and 127.0.0.1
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")

		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)
		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			assert.NoError(t, err)
			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %v", resp.Status)
			}
		}

	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_NotInitRpcConf3(t *testing.T) {

	t.Logf("NotInitRpcConf3 WithAuthAndLocalhost begin")

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
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
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

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
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
	var reqObj ReqObj
	//data ,errw := json.Marshal(reqObj)
	//t.Logf("json.Marshal-------   %s errw %v", data, errw)

	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	//var someOne ReqObj
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
		//fmt.Printf("someOne %v",someOne)

	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request with  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")

		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)
		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			assert.NoError(t, err)
			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

/*
server config with User and Pass but no whitiplist
client req with no authorization
expect: resp.StatusCode StatusUnauthorized
*/
func TestServer_WithUserPassNoIp1(t *testing.T) {

	t.Logf("WithUserPassNoIp1    authorization(user,pass) ok and localhost begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}
func TestServer_WithUserPassNoIp2(t *testing.T) {

	t.Logf("WithUserPassNoIp2    header.authorization fail. user and pass all wrong and localhost begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth("111", "222")

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusUnauthorized {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

/*
server config with User and Pass but no whitiplist
client req with no authorization
expect: resp.StatusCode StatusUnauthorized
*/
func TestServer_WithUserPassNoIp3(t *testing.T) {

	t.Logf("WithUserPassNoIp3  header.authorization fail. user  is wrong and localhost begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth("111", s.cfg.RpcConfiguration.Pass)

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusUnauthorized {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

/*
server config with User and Pass but no whitiplist
client req with no authorization
expect: resp.StatusCode StatusUnauthorized
*/
func TestServer_WithUserPassNoIp4(t *testing.T) {

	t.Logf("WithUserPassNoIp3  header.authorization fail. pass  is wrong and localhost begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth(s.cfg.RpcConfiguration.User, "111")

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusUnauthorized {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

/*
server config with User and Pass but no whitiplist
client req with no authorization
expect: resp.StatusCode StatusUnauthorized
*/
func TestServer_WithUserPassNoIp5(t *testing.T) {

	t.Logf("WithUserPassNoIp1  no  authorization and localhost begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://localhost:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusUnauthorized {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_NoUserPassWithIp1(t *testing.T) {

	t.Logf("NoUserPassWithIp1  no  user and pass and whiteiplist is allowd")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		RpcConfiguration: RpcConfiguration{
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_NoUserPassWithIp2(t *testing.T) {

	t.Logf("NoUserPassWithIp1  no  user and pass and whiteiplist is forbid")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!

		RpcConfiguration: RpcConfiguration{
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", urlNotLoopBack, req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusForbidden {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_WithUserPassWithIp1(t *testing.T) {

	t.Logf("WithUserPassWithIp1  with  user and pass and whiteiplist are all correct")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!
		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}
func TestServer_WithUserPassWithIp2(t *testing.T) {

	t.Logf("WithUserPassWithIp1  with  user and pass and ip is forbid")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!
		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", urlNotLoopBack, req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusForbidden {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_WithUserPassWithIp3(t *testing.T) {

	t.Logf("WithUserPassWithIp1  with  user and pass and ip ,but user is wrong")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!
		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth("ElaUser111", s.cfg.RpcConfiguration.Pass)

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusUnauthorized {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}
func TestServer_WithUserPassWithIp4(t *testing.T) {

	t.Logf("WithUserPassWithIp4  with  user and pass and pass is wrong")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!
		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth(s.cfg.RpcConfiguration.User, "wrong pass")

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusUnauthorized {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_WithIp0000_1(t *testing.T) {

	t.Logf("WithIp0000  with  user and pass and ip 0.0.0.0. client user 192.168 expect ok")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!
		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
			WhiteIPList: []string{
				"0.0.0.0",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", urlNotLoopBack, req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_WithIp0000_2(t *testing.T) {

	t.Logf("WithIp0000  with  user and pass and ip 0.0.0.0. client user 127.0.0.1 expect ok")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		//mention that  no init RpcConfiguration
		//!!!!!!!!!!!!!!!!!!!!!!
		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
			WhiteIPList: []string{
				"0.0.0.0",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
		return nil, nil
	}, "level")

	go s.Start()

	type Student struct {
		name string
		age  int
	}

	type ReqObj struct {
		method string
		params Student
	}

	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"junneyang", "age": 88}
	}`
	///////////////
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
	}
	///////////////
	req_new := bytes.NewBuffer([]byte(reqStr))
	// request no  authorization and localhost
	go func() {

		client := &http.Client{}
		req, err := http.NewRequest("POST", "http://127.0.0.1:20336/api/test", req_new)
		if err != nil {
			// handle error
		}
		req.Header.Set("Content-Type", "application/json")
		req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)

		resp, err2 := client.Do(req)
		t.Logf("request with  authorization   err2 %v  resp%v", err2, resp)
		if resp != nil {
			t.Logf("request with  authorization   status %v", resp.Status)
			//t.Logf("request with  authorization err  %v StatusCode %v", err2, resp.StatusCode)
			if resp.StatusCode != http.StatusOK {
				t.Error("expecting not found get %v", resp.Status)
			}
		}
	}()
	////////////////
	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_WithAuth(t *testing.T) {
	return
	t.Logf("client_side TestServer_WithAuth begin")

	test.SkipShort(t)

	s := NewServer(&Config{
		ServePort: 20336,
		RpcConfiguration: RpcConfiguration{
			User: "ElaUser",
			Pass: "Ela123",
			WhiteIPList: []string{
				"127.0.0.1",
			},
		},
	})

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)
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
	req.SetBasicAuth(s.cfg.RpcConfiguration.User, s.cfg.RpcConfiguration.Pass)
	//t.Logf(" client_side req.Header!!!!!!!!  %v", req.Header)
	res, err5 := client.Do(req)
	assert.NoError(t, err)
	t.Logf("client_side client.Do err  %v res %v", err5, res.Status)

	//////////////// d
	select {
	case <-time.After(time.Second * 1):
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
