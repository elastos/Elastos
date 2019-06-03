package utils

import (
	"fmt"
	"net"
	"net/http"
	_ "net/http/pprof"
	"os"
	"strconv"

	"github.com/howeyc/gopass"
)

// GetPassword gets password from user input
func GetPassword() ([]byte, error) {
	fmt.Printf("Password:")
	return gopass.GetPasswd()
}

// GetConfirmedPassword gets double confirmed password from user input
func GetConfirmedPassword() ([]byte, error) {
	fmt.Printf("Password:")
	first, err := gopass.GetPasswd()
	if err != nil {
		return nil, err
	}
	fmt.Printf("Re-enter Password:")
	second, err := gopass.GetPasswd()
	if err != nil {
		return nil, err
	}
	if len(first) != len(second) {
		fmt.Println("Unmatched Password")
		os.Exit(1)
	}
	for i, v := range first {
		if v != second[i] {
			fmt.Println("Unmatched Password")
			os.Exit(1)
		}
	}
	return first, nil
}

func FileExisted(filename string) bool {
	_, err := os.Stat(filename)
	return err == nil || os.IsExist(err)
}

//open ela pprof ,must run with goroutine
func StartPProf(port uint32) {
	listenAddr := net.JoinHostPort("", strconv.FormatUint(uint64(port), 10))
	fmt.Printf("Profile server listening on %s\n", listenAddr)
	profileRedirect := http.RedirectHandler("/debug/pprof", http.StatusSeeOther)
	http.Handle("/", profileRedirect)
	ret := http.ListenAndServe(listenAddr, nil)
	fmt.Printf("Profile server ListenAndServe return %v", ret)
}
