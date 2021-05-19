// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package utils

import (
	"fmt"
	"io"
	"net"
	"net/http"
	_ "net/http/pprof"
	"os"
	"strconv"

	"github.com/elastos/Elastos.ELA/common"

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

// CopyStringSet copy the src map's key, and return the dst map.
func CopyStringSet(src map[string]struct{}) (dst map[string]struct{}) {
	dst = map[string]struct{}{}
	for k := range src {
		dst[k] = struct{}{}
	}
	return
}

// CopyStringMap copy the src map's key and value, and return the dst map.
func CopyStringMap(src map[string]string) (dst map[string]string) {
	dst = map[string]string{}
	for k, v := range src {
		p := v
		dst[k] = p
	}
	return
}

func SerializeStringMap(w io.Writer, smap map[string]string) (err error) {
	if err = common.WriteVarUint(w, uint64(len(smap))); err != nil {
		return
	}
	for k, v := range smap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}

		if err = common.WriteVarString(w, v); err != nil {
			return
		}
	}
	return
}

func DeserializeStringMap(r io.Reader) (smap map[string]string, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	smap = make(map[string]string)
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		var v string
		if v, err = common.ReadVarString(r); err != nil {
			return
		}
		smap[k] = v
	}
	return
}

func SerializeStringSet(w io.Writer, vmap map[string]struct{}) (err error) {
	if err = common.WriteVarUint(w, uint64(len(vmap))); err != nil {
		return
	}
	for k := range vmap {
		if err = common.WriteVarString(w, k); err != nil {
			return
		}
	}
	return
}

func DeserializeStringSet(r io.Reader) (vmap map[string]struct{}, err error) {
	var count uint64
	if count, err = common.ReadVarUint(r, 0); err != nil {
		return
	}
	vmap = make(map[string]struct{})
	for i := uint64(0); i < count; i++ {
		var k string
		if k, err = common.ReadVarString(r); err != nil {
			return
		}
		vmap[k] = struct{}{}
	}
	return
}
