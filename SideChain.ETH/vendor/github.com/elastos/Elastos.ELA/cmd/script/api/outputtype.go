package api

import (
	"encoding/hex"
	"fmt"
	"os"

	"github.com/elastos/Elastos.ELA/common/log"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/yuin/gopher-lua"
)

const (
	luaOutputTypeName        = "output"
	luaVoteOutputTypeName    = "voteoutput"
	luaDefaultOutputTypeName = "defaultoutput"
	luaVoteContentTypeName   = "votecontent"
)

func RegisterOutputType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaOutputTypeName)
	L.SetGlobal("output", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newTxOutput))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), outputMethods))
}

// Constructor
func newTxOutput(L *lua.LState) int {
	assetIDStr := L.ToString(1)
	value := L.ToInt64(2)
	address := L.ToString(3)
	outputType := L.ToInt(4)
	outputPayloadData := L.CheckUserData(5)

	assetIDSlice, _ := hex.DecodeString(assetIDStr)
	assetIDSlice = common.BytesReverse(assetIDSlice)
	var assetID common.Uint256
	copy(assetID[:], assetIDSlice[0:32])

	programHash, err := common.Uint168FromAddress(address)
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	var outputPayload types.OutputPayload
	switch outputPayloadData.Value.(type) {
	case *outputpayload.DefaultOutput:
		payload, ok := outputPayloadData.Value.(*outputpayload.DefaultOutput)
		if !ok {
			log.Debug("error default output payload")
		}
		outputPayload = payload
	case *outputpayload.VoteOutput:
		payload, ok := outputPayloadData.Value.(*outputpayload.VoteOutput)
		if !ok {
			log.Debug("error vote output payload")
		}
		outputPayload = payload
	}

	output := &types.Output{
		AssetID:     assetID,
		Value:       common.Fixed64(value),
		OutputLock:  0,
		ProgramHash: *programHash,
		Type:        types.OutputType(outputType),
		Payload:     outputPayload,
	}

	ud := L.NewUserData()
	ud.Value = output
	L.SetMetatable(ud, L.GetTypeMetatable(luaOutputTypeName))
	L.Push(ud)

	return 1
}

// Checks whether the first lua argument is a *LUserData with *Output and returns this *Output.
func checkTxOutput(L *lua.LState, idx int) *types.Output {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*types.Output); ok {
		return v
	}
	L.ArgError(1, "Output expected")
	return nil
}

var outputMethods = map[string]lua.LGFunction{
	"get": outputGet,
}

// Getter and setter for the Person#Name
func outputGet(L *lua.LState) int {
	p := checkTxOutput(L, 1)
	fmt.Println(p)

	return 0
}

// Default Output Payload
func RegisterDefaultOutputType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaDefaultOutputTypeName)
	L.SetGlobal("defaultoutput", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newDefaultOutput))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), newDefaultOutputMethods))
}

func newDefaultOutput(L *lua.LState) int {
	defaultOutput := &outputpayload.DefaultOutput{}
	ud := L.NewUserData()
	ud.Value = defaultOutput
	L.SetMetatable(ud, L.GetTypeMetatable(luaDefaultOutputTypeName))
	L.Push(ud)

	return 1
}

func checkDefaultOutput(L *lua.LState, idx int) *outputpayload.DefaultOutput {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*outputpayload.DefaultOutput); ok {
		return v
	}
	L.ArgError(1, "OTNone expected")
	return nil
}

var newDefaultOutputMethods = map[string]lua.LGFunction{
	"get": defaultOutputGet,
}

func defaultOutputGet(L *lua.LState) int {
	p := checkDefaultOutput(L, 1)
	fmt.Println(p)

	return 0
}

// OTVote Output Payload
func RegisterVoteOutputType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaVoteOutputTypeName)
	L.SetGlobal("voteoutput", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newVoteOutput))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), newVoteOutputMethods))
}

func newVoteOutput(L *lua.LState) int {
	version := L.ToInt(1)
	contentsTable := L.ToTable(2)

	contents := make([]outputpayload.VoteContent, 0)
	contentsTable.ForEach(func(i, v lua.LValue) {
		lv, ok := v.(*lua.LUserData)
		if !ok {
			println("error vote content user data")
		}
		content, ok := lv.Value.(*outputpayload.VoteContent)
		if !ok {
			fmt.Println("error vote content")
		}
		contents = append(contents, *content)
	})

	voteOutput := &outputpayload.VoteOutput{
		Version:  byte(version),
		Contents: contents,
	}
	ud := L.NewUserData()
	ud.Value = voteOutput
	L.SetMetatable(ud, L.GetTypeMetatable(luaVoteOutputTypeName))
	L.Push(ud)

	return 1
}

func checkVoteOutput(L *lua.LState, idx int) *outputpayload.VoteOutput {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*outputpayload.VoteOutput); ok {
		return v
	}
	L.ArgError(1, "OTVote expected")
	return nil
}

var newVoteOutputMethods = map[string]lua.LGFunction{
	"get": voteOutputGet,
}

func voteOutputGet(L *lua.LState) int {
	p := checkVoteOutput(L, 1)
	fmt.Println(p)

	return 0
}

// OTVote Content
func RegisterVoteContentType(L *lua.LState) {
	mt := L.NewTypeMetatable(luaVoteContentTypeName)
	L.SetGlobal("votecontent", mt)
	// static attributes
	L.SetField(mt, "new", L.NewFunction(newVoteContent))
	// methods
	L.SetField(mt, "__index", L.SetFuncs(L.NewTable(), newVoteContentMethods))
}

func newVoteContent(L *lua.LState) int {
	voteType := L.ToInt(1)
	candidatesTable := L.ToTable(2)

	candidates := make([][]byte, 0)
	candidatesTable.ForEach(func(i, value lua.LValue) {
		//fmt.Println(lua.LVAsString(value))
		publicKey := lua.LVAsString(value)
		pk, err := common.HexStringToBytes(publicKey)
		if err != nil {
			fmt.Println("invalid public key")
			os.Exit(1)
		}
		candidates = append(candidates, pk)
	})

	voteContent := &outputpayload.VoteContent{
		VoteType:   outputpayload.VoteType(voteType),
		Candidates: candidates,
	}

	ud := L.NewUserData()
	ud.Value = voteContent
	L.SetMetatable(ud, L.GetTypeMetatable(luaVoteContentTypeName))
	L.Push(ud)

	return 1
}

func checkVoteContent(L *lua.LState, idx int) *outputpayload.VoteContent {
	ud := L.CheckUserData(idx)
	if v, ok := ud.Value.(*outputpayload.VoteContent); ok {
		return v
	}
	L.ArgError(1, "VoteContent expected")
	return nil
}

var newVoteContentMethods = map[string]lua.LGFunction{
	"get": voteContentGet,
}

func voteContentGet(L *lua.LState) int {
	p := checkVoteContent(L, 1)
	fmt.Println(p)

	return 0
}
