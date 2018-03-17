package db

import (
	"sync"

	"SPVWallet/core"
	"SPVWallet/core/transaction"
)

type ScriptFilter struct {
	sync.Mutex
	scripts map[core.Uint168][]byte
}

func NewScriptFilter(scripts [][]byte) *ScriptFilter {
	filter := new(ScriptFilter)
	filter.LoadScripts(scripts)
	return filter
}

func (filter *ScriptFilter) LoadScripts(scripts [][]byte) {
	filter.Lock()
	defer filter.Unlock()

	filter.scripts = make(map[core.Uint168][]byte)
	for _, script := range scripts {
		hash, _ := transaction.ToProgramHash(script)
		filter.scripts[*hash] = script
	}
}

func (filter *ScriptFilter) AddScript(script []byte) {
	filter.Lock()
	defer filter.Unlock()

	hash, _ := transaction.ToProgramHash(script)
	filter.scripts[*hash] = script
}

func (filter *ScriptFilter) DeleteScript(hash core.Uint168) {
	filter.Lock()
	defer filter.Unlock()

	delete(filter.scripts, hash)
}

func (filter *ScriptFilter) GetScriptHashes() []core.Uint168 {
	var scriptHashes = make([]core.Uint168, 0)
	for scriptHash := range filter.scripts {
		scriptHashes = append(scriptHashes, scriptHash)
	}

	return scriptHashes
}

func (filter *ScriptFilter) ContainAddress(hash core.Uint168) bool {
	filter.Lock()
	defer filter.Unlock()

	_, ok := filter.scripts[hash]
	return ok
}
