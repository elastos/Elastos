// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package httpwebsocket

import (
	"errors"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

type session struct {
	mtx        sync.Mutex
	id         int64
	conn       *websocket.Conn
	lastActive time.Time
}

func (s *session) Send(data []byte) error {
	if s.conn == nil {
		return errors.New("WebSocket is null")
	}

	s.mtx.Lock()
	err := s.conn.WriteMessage(websocket.TextMessage, data)
	s.mtx.Unlock()
	return err
}

type sessions struct {
	sync.Map
}

func (ss *sessions) Load(id int64) (*session, bool) {
	v, ok := ss.Map.Load(id)
	return v.(*session), ok
}

func (ss *sessions) Delete(s *session) {
	s.conn.Close()
	ss.Map.Delete(s.id)
}

func (ss *sessions) Count() int {
	count := 0
	ss.Map.Range(func(k, v interface{}) bool {
		count++
		return true
	})
	return count
}

func (ss *sessions) Foreach(f func(*session)) {
	ss.Map.Range(func(k, v interface{}) bool {
		f(v.(*session))
		return true
	})
}
