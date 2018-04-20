package httpwebsocket

import (
	"errors"
	"sync"
	"time"

	"github.com/gorilla/websocket"
)

type Session struct {
	sync.Mutex
	Connection *websocket.Conn
	LastActive int64
	SessionId  string
}

type SessionList struct {
	sync.RWMutex
	OnlineList map[string]*Session //key is SessionId
}

const SessionTimeOut int64 = 120

func (s *Session) Send(data []byte) error {
	if s.Connection == nil {
		return errors.New("WebSocket is null")
	}
	//https://godoc.org/github.com/gorilla/websocket
	s.Lock()
	defer s.Unlock()
	return s.Connection.WriteMessage(websocket.TextMessage, data)
}

func (s *Session) SessionTimeoverCheck() bool {
	nCurTime := time.Now().Unix()
	if nCurTime-s.LastActive > SessionTimeOut { //sec
		return true
	} else {
		return false
	}
}

func (sl *SessionList) CloseSession(session *Session) {
	delete(sl.OnlineList, session.SessionId)
	session.Connection.Close()
	session.SessionId = ""
}

func (sl *SessionList) ForEachSession(visit func(*Session)) {
	sl.RLock()
	defer sl.RUnlock()
	for _, v := range sl.OnlineList {
		visit(v)
	}
}