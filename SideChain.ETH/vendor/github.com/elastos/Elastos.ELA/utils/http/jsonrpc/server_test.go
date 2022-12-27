package jsonrpc

import (
	"bytes"
	"encoding/json"
	"fmt"
	"net"
	"net/http"
	"testing"
	"time"

	htp "github.com/elastos/Elastos.ELA/utils/http"
	"github.com/elastos/Elastos.ELA/utils/test"
)

//if bRunServer is true .run server for every testcase
var bRunServer bool = true

var (
	urlNotLoopBack string
	urlLoopBack    string
	urlLocalhost   string
	req_new        *bytes.Buffer
	clientAuthUser string
	clientAuthPass string
	pServer        *http.Server
)

type RpcConfiguration struct {
	User        string
	Pass        string
	WhiteIPList []string
}

func initUrl() {
	ipNotLoopBack := resolveHostIp()
	if ipNotLoopBack == "" {
		//t.Error("expecting not found get %", resp.Status)
		fmt.Printf("ipNotLoopBack  error should exit!!!!!!!!!!!!!!!")
		return
	}
	httpPrefix := "http://"
	httPostfix := ":20336"

	urlNotLoopBack = httpPrefix + ipNotLoopBack + httPostfix
	fmt.Printf("Before Test init url %v", urlNotLoopBack)

	urlLoopBack = "http://127.0.0.1:20336"
	urlLocalhost = "http://localhost:20336"
}
func isRunServer() bool {
	return bRunServer
}
func init() {

	initUrl()
}

func registerTestAction(s *Server, t *testing.T) {

	s.RegisterAction("/api/test", func(data htp.Params) (interface{}, error) {
		t.Logf("client_side Receive POST request from path %s, data %v", "/api/test", data)

		return nil, nil
	}, "level")
}

func initReqObject() {

	type ReqObj struct {
		method string
	}
	var reqObj ReqObj
	reqStr := `{
	"method":"/api/test",
	"params":{"name":"test data"}
	}`
	if err := json.Unmarshal([]byte(reqStr), &reqObj); err == nil {
		//fmt.Printf("reqObj %+v\n", reqObj)
	} else {
		fmt.Println(err)
		return
	}
	///////////////
	req_new = bytes.NewBuffer([]byte(reqStr))
}

func GetInternalIP() string {
	itf, _ := net.InterfaceByName("en0") //here your interface
	item, _ := itf.Addrs()
	var ip net.IP
	for _, addr := range item {
		switch v := addr.(type) {
		case *net.IPNet:
			if !v.IP.IsLoopback() {
				if v.IP.To4() != nil { //Verify if IP is IPV4
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
func resolveHostIp() string {

	netInterfaceAddresses, err := net.InterfaceAddrs()

	if err != nil {
		return ""
	}

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

func PostReq(url string, withAuthorization bool, expectStatus int, t *testing.T) {
	//t.Logf("PostReq req_new !!!!!!!!! %v", req_new)
	client := &http.Client{}
	req, err := http.NewRequest("POST", url, req_new)
	if err != nil {
	}
	req.Header.Set("Content-Type", "application/json")
	if withAuthorization {
		req.SetBasicAuth(clientAuthUser, clientAuthPass)
	}
	resp, err2 := client.Do(req)
	if resp != nil {
		if resp.StatusCode != expectStatus {
			t.Error("expecting not found get resp.Status %", resp.Status)
		} else {
			t.Logf(" PostReq resp.StatusCode == expectStatus %v\n", resp.Status)
		}
	} else {
		t.Logf(" PostReq resp.StatusCode == expectStatus err2 %v\n", err2)

	}
}
func Wait(s *Server) {

	select {
	case <-time.After(time.Second * 1):
		s.Stop()
	}
}

func TestServer_NotInitRpcConf(t *testing.T) {

	t.Logf("NotInitRpcConf1 request with no authorization and 127.0.0.1 begin")
	test.SkipShort(t)

	svrConf := RpcConfiguration{
		User:        "",
		Pass:        "",
		WhiteIPList: []string{""},
	}

	s := NewServer(&Config{
		ServePort: 20336,
		User:      svrConf.User,
		Pass:      svrConf.Pass,
		WhiteList: svrConf.WhiteIPList,
	})

	registerTestAction(s, t)

	if isRunServer() {
		go s.Start()
	}

	urlLoopBackNoAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)
		t.Logf("TestServer_NotInitRpcConf    urlLoopBackNoAuthTest end")

	}
	urlLoopBackNoAuthTest(urlLoopBack, false, http.StatusOK, t)

	urlLoopBackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)
		t.Logf("TestServer_NotInitRpcConf    urlLoopBackWithAuthTest end")

	}
	urlLoopBackWithAuthTest(urlLoopBack, true, http.StatusOK, t)

	urlLocalhostWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)
		t.Logf("TestServer_NotInitRpcConf    urlLocalhostWithAuthTest end")

	}
	urlLocalhostWithAuthTest(urlLocalhost, true, http.StatusOK, t)

	urlNotLoopBackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)
		t.Logf("TestServer_NotInitRpcConf    urlNotLoopBackWithAuthTest end")

	}
	urlNotLoopBackWithAuthTest(urlNotLoopBack, true, http.StatusForbidden, t)

	Wait(s)
}

func TestServer_WithUserPassNoIp(t *testing.T) {
	t.Logf("TestServer_WithUserPassNoIp    authorization(user,pass) ok and localhost begin")
	test.SkipShort(t)

	svrConf := RpcConfiguration{
		User:        "ElaUser",
		Pass:        "Ela123",
		WhiteIPList: []string{""},
	}

	s := NewServer(&Config{
		ServePort: 20336,
		User:      svrConf.User,
		Pass:      svrConf.Pass,
		WhiteList: svrConf.WhiteIPList,
	})

	registerTestAction(s, t)

	if isRunServer() {
		go s.Start()
	}

	/////////////////////////

	urlLocalhostWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLocalhostWithAuthTest(urlLocalhost, true, http.StatusOK, t)

	//////////////////////////
	urlLoopBackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopBackWithAuthTest(urlLoopBack, true, http.StatusOK, t)

	urlNotLoopBackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlNotLoopBackWithAuthTest(urlNotLoopBack, true, http.StatusForbidden, t)
	////////////////////////

	urlLocalhostWithAuthWrongUserPassTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = "1111"
		clientAuthPass = "1111"
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLocalhostWithAuthWrongUserPassTest(urlLocalhost, true, http.StatusUnauthorized, t)

	urlLocalhostWithAuthWrongUserTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = "1111"
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}

	urlLocalhostWithAuthWrongUserTest(urlLocalhost, true, http.StatusUnauthorized, t)

	urlLocalhostWithAuthWrongPassTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = "123"
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLocalhostWithAuthWrongPassTest(urlLocalhost, true, http.StatusUnauthorized, t)

	urlLocalhostNoAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLocalhostNoAuthTest(urlLocalhost, false, http.StatusUnauthorized, t)

	Wait(s)
}

func TestServer_NoUserPassWithIp(t *testing.T) {

	t.Logf("NoUserPassWithIp1  no  user and pass and whiteiplist is allowd")

	test.SkipShort(t)
	svrConf := RpcConfiguration{
		//User:        "ElaUser",
		//Pass:        "Ela123",
		WhiteIPList: []string{"127.0.0.1"},
	}
	s := NewServer(&Config{
		ServePort: 20336,
		User:      svrConf.User,
		Pass:      svrConf.Pass,
		WhiteList: svrConf.WhiteIPList,
	})

	registerTestAction(s, t)

	if isRunServer() {
		go s.Start()
	}
	urlLocalhostNoAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLocalhostNoAuthTest(urlLocalhost, false, http.StatusOK, t)

	urlLoopBackNoAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopBackNoAuthTest(urlLoopBack, false, http.StatusOK, t)

	urlNotLoopBacktNoAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlNotLoopBacktNoAuthTest(urlNotLoopBack, false, http.StatusForbidden, t)

	urlLoopBackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopBackWithAuthTest(urlLoopBack, true, http.StatusOK, t)

	Wait(s)
}

func TestServer_WithUserPassWithIp(t *testing.T) {

	t.Logf("WithUserPassWithIp1  with  user and pass and whiteiplist are all correct")
	initReqObject()

	test.SkipShort(t)
	svrConf := RpcConfiguration{
		User:        "ElaUser",
		Pass:        "Ela123",
		WhiteIPList: []string{"127.0.0.1"},
	}
	s := NewServer(&Config{
		ServePort: 20336,
		User:      svrConf.User,
		Pass:      svrConf.Pass,
		WhiteList: svrConf.WhiteIPList,
	})

	registerTestAction(s, t)

	if isRunServer() {
		go s.Start()
	}

	urlLoopbackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopbackWithAuthTest(urlLoopBack, true, http.StatusOK, t)

	urlNotLoopbackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlNotLoopbackWithAuthTest(urlNotLoopBack, true, http.StatusForbidden, t)

	urlLoopbackWithAuthWrongUserTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = "1111"
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopbackWithAuthWrongUserTest(urlLoopBack, true, http.StatusUnauthorized, t)

	urlLoopbackWithAuthWrongPassTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = "1111"
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopbackWithAuthWrongPassTest(urlLoopBack, true, http.StatusUnauthorized, t)

	Wait(s)
}

func TestServer_WithIp0000(t *testing.T) {

	t.Logf("WithIp0000  with  user and pass and ip 0.0.0.0. client user 192.168 expect ok")

	test.SkipShort(t)
	svrConf := RpcConfiguration{
		User:        "ElaUser",
		Pass:        "Ela123",
		WhiteIPList: []string{"0.0.0.0"},
		//WhiteIPList: []string{"192.168.2.194"},

	}

	s := NewServer(&Config{
		ServePort: 20336,
		User:      svrConf.User,
		Pass:      svrConf.Pass,
		WhiteList: svrConf.WhiteIPList,
	})

	registerTestAction(s, t)

	if isRunServer() {
		go s.Start()
	}

	urlNotLoopbackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlNotLoopbackWithAuthTest(urlNotLoopBack, true, http.StatusOK, t)

	urlLoopbackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopbackWithAuthTest(urlLoopBack, true, http.StatusOK, t)

	urlLocalhostWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLocalhostWithAuthTest(urlLocalhost, true, http.StatusOK, t)

	urlLoopbackWithAuthWrongUserTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = "111"
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopbackWithAuthWrongUserTest(urlLoopBack, true, http.StatusUnauthorized, t)

	Wait(s)
}

func TestServer_WithIpNotLoopBackIp(t *testing.T) {

	t.Logf("TestServer_WithIpNotLoopBackIp begin")
	ipNotLoopBack := resolveHostIp()

	test.SkipShort(t)
	svrConf := RpcConfiguration{
		User:        "ElaUser",
		Pass:        "Ela123",
		WhiteIPList: []string{ipNotLoopBack},
		//WhiteIPList: []string{"192.168.2.194"},

	}
	t.Logf("TestServer_WithIpNotLoopBackIp %v", svrConf)

	s := NewServer(&Config{
		ServePort: 20336,
		User:      svrConf.User,
		Pass:      svrConf.Pass,
		WhiteList: svrConf.WhiteIPList,
	})

	registerTestAction(s, t)

	if isRunServer() {
		go s.Start()
	}

	urlNotLoopbackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlNotLoopbackWithAuthTest(urlNotLoopBack, true, http.StatusOK, t)

	urlLoopbackWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopbackWithAuthTest(urlLoopBack, true, http.StatusOK, t)

	urlLocalhostWithAuthTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = svrConf.User
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLocalhostWithAuthTest(urlLocalhost, true, http.StatusOK, t)

	urlLoopbackWithAuthWrongUserTest := func(url string, withAuthorization bool, expectStatus int, t *testing.T) {
		clientAuthUser = "111"
		clientAuthPass = svrConf.Pass
		initReqObject()
		PostReq(url, withAuthorization, expectStatus, t)

	}
	urlLoopbackWithAuthWrongUserTest(urlLoopBack, true, http.StatusUnauthorized, t)

	Wait(s)
}