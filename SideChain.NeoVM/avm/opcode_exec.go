package avm

import "errors"

type OpExec struct {
	Opcode    OpCode
	Name      string
	Exec      func(*ExecutionEngine) (VMState, error)
	Validator func(*ExecutionEngine) error
}

var (
	OpExecList = [256]OpExec{
		// control flow
		PUSH0:       {PUSH0, "PUSH0", opPushData, nil},
		PUSHBYTES1:  {PUSHBYTES1, "PUSHBYTES1", opPushData, nil},
		PUSHBYTES75: {PUSHBYTES75, "PUSHBYTES75", opPushData, nil},
		PUSHDATA1:   {PUSHDATA1, "PUSHDATA1", opPushData, nil},
		PUSHDATA2:   {PUSHDATA2, "PUSHDATA2", opPushData, nil},
		PUSHDATA4:   {PUSHDATA4, "PUSHDATA4", opPushData, validatorPushData4},
		PUSHM1:      {PUSHM1, "PUSHM1", opPushData, nil},
		PUSH1:       {PUSH1, "PUSH1", opPushData, nil},
		PUSH2:       {PUSH2, "PUSH2", opPushData, nil},
		PUSH3:       {PUSH3, "PUSH3", opPushData, nil},
		PUSH4:       {PUSH4, "PUSH4", opPushData, nil},
		PUSH5:       {PUSH5, "PUSH5", opPushData, nil},
		PUSH6:       {PUSH6, "PUSH6", opPushData, nil},
		PUSH7:       {PUSH7, "PUSH7", opPushData, nil},
		PUSH8:       {PUSH8, "PUSH8", opPushData, nil},
		PUSH9:       {PUSH9, "PUSH9", opPushData, nil},
		PUSH10:      {PUSH10, "PUSH10", opPushData, nil},
		PUSH11:      {PUSH11, "PUSH11", opPushData, nil},
		PUSH12:      {PUSH12, "PUSH12", opPushData, nil},
		PUSH13:      {PUSH13, "PUSH13", opPushData, nil},
		PUSH14:      {PUSH14, "PUSH14", opPushData, nil},
		PUSH15:      {PUSH15, "PUSH15", opPushData, nil},
		PUSH16:      {PUSH16, "PUSH16", opPushData, nil},

		//Control
		NOP:      {NOP, "NOP", opNop, nil},
		JMP:      {JMP, "JMP", opJmp, nil},
		JMPIF:    {JMPIF, "JMPIF", opJmp, nil},
		JMPIFNOT: {JMPIFNOT, "JMPIFNOT", opJmp, nil},
		CALL:     {CALL, "CALL", opCall, validateCall},
		RET:      {RET, "RET", opRet, nil},
		APPCALL:  {APPCALL, "APPCALL", opAppCall, validateAppCall},
		TAILCALL: {TAILCALL, "TAILCALL", opAppCall, validateAppCall},
		SYSCALL:  {SYSCALL, "SYSCALL", opSysCall, validateSysCall},

		//Stack ops
		DUPFROMALTSTACK: {DUPFROMALTSTACK, "DUPFROMALTSTACK", opDupFromAltStack, nil},
		TOALTSTACK:      {TOALTSTACK, "TOALTSTACK", opToAltStack, nil},
		FROMALTSTACK:    {FROMALTSTACK, "FROMALTSTACK", opFromAltStack, nil},
		XDROP:           {XDROP, "XDROP", opXDrop, validateXDrop},
		XSWAP:           {XSWAP, "XSWAPP", opXSwap, validateXSwap},
		XTUCK:           {XTUCK, "XTUCK", opXTuck, validateXTuck},
		DEPTH:           {DEPTH, "DEPTH", opDepth, nil},
		DROP:            {DROP, "DROP", opDrop, nil},
		DUP:             {DUP, "DUP", opDup, nil},
		NIP:             {NIP, "NIP", opNip, nil},
		OVER:            {OVER, "OVER", opOver, nil},
		PICK:            {PICK, "PICK", opPick, validatePick},
		ROLL:            {ROLL, "ROLL", opRoll, validateRoll},
		ROT:             {ROT, "ROT", opRot, nil},
		SWAP:            {SWAP, "SWAP", opSwap, nil},
		TUCK:            {TUCK, "TUCK", opTuck, nil},

		//Splice
		CAT:    {CAT, "CAT", opCat, validateCat},
		SUBSTR: {SUBSTR, "SUBSTR", opSubStr, validateSubStr},
		LEFT:   {LEFT, "LEFT", opLeft, validateLeft},
		RIGHT:  {RIGHT, "RIGHT", opRight, validateRight},
		SIZE:   {SIZE, "SIZE", opSize, nil},

		//Bitwiase logic
		INVERT: {INVERT, "INVERT", opInvert, nil},
		AND:    {AND, "AND", opBigIntZip, nil},
		OR:     {OR, "OR", opBigIntZip, nil},
		XOR:    {XOR, "XOR", opBigIntZip, nil},
		EQUAL:  {EQUAL, "EQUAL", opEqual, nil},

		//Arithmetic
		INC:         {INC, "INC", opBigInt, nil},
		DEC:         {DEC, "DEC", opBigInt, nil},
		SAL:         {SAL, "SAL", opBigInt, nil},
		SAR:         {SAR, "SAR", opBigInt, nil},
		NEGATE:      {NEGATE, "NEGATE", opBigInt, nil},
		ABS:         {ABS, "ABS", opBigInt, nil},
		NOT:         {NOT, "NOT", opNot, nil},
		NZ:          {NZ, "NZ", opNz, nil},
		ADD:         {ADD, "ADD", opBigIntZip, nil},
		SUB:         {SUB, "SUB", opBigIntZip, nil},
		MUL:         {MUL, "MUL", opBigIntZip, nil},
		DIV:         {DIV, "DIV", opBigIntZip, nil},
		MOD:         {MOD, "MOD", opBigIntZip, nil},
		SHL:         {SHL, "SHL", opBigIntZip, validateSHL},
		SHR:         {SHR, "SHR", opBigIntZip, validateSHR},
		BOOLAND:     {BOOLAND, "BOOLAND", opBoolZip, nil},
		BOOLOR:      {BOOLOR, "BOOLOR", opBoolZip, nil},
		NUMEQUAL:    {NUMEQUAL, "NUMEQUAL", opBigIntComp, validatorBigIntComp},
		NUMNOTEQUAL: {NUMNOTEQUAL, "NUMNOTEQUAL", opBigIntComp, validatorBigIntComp},
		LT:          {LT, "LT", opBigIntComp, nil},
		GT:          {GT, "GT", opBigIntComp, nil},
		LTE:         {LTE, "LTE", opBigIntComp, nil},
		GTE:         {GTE, "GTE", opBigIntComp, nil},
		MIN:         {MIN, "MIN", opBigIntZip, nil},
		MAX:         {MAX, "MAX", opBigIntZip, nil},
		WITHIN:      {WITHIN, "WITHIN", opWithIn, nil},

		//Crypto
		SHA1:          {SHA1, "SHA1", opHash, nil},
		SHA256:        {SHA256, "SHA256", opHash, nil},
		HASH160:       {HASH160, "HASH160", opHash, nil},
		HASH256:       {HASH256, "HASH256", opHash, nil},
		CHECKSIG:      {CHECKSIG, "CHECKSIG", opCheckSig, nil},
		CHECKREGID:    {CHECKREGID, "CHECKREGID", opCheckSig, nil},
		CHECKMULTISIG: {CHECKMULTISIG, "CHECKMULTISIG", opCheckMultiSig, nil},

		//Array
		ARRAYSIZE: {ARRAYSIZE, "ARRAYSIZE", opArraySize, nil},
		PACK:      {PACK, "PACK", opPack, validatePack},
		UNPACK:    {UNPACK, "UNPACK", opUnpack, nil},
		PICKITEM:  {PICKITEM, "PICKITEM", opPickItem, validatePickItem},
		SETITEM:   {SETITEM, "SETITEM", opSetItem, validatorSetItem},
		NEWARRAY:  {NEWARRAY, "NEWARRAY", opNewArray, validateNewArray},
		REVERSE:   {REVERSE, "REVERSE", opReverse, nil},
		REMOVE:    {REMOVE, "REMOVE", opRemove, nil},
		HASKEY:    {HASKEY, "HASKEY", opHasKey, nil},
		KEYS:      {KEYS, "KEYS", opKeys, nil},
		VALUES:    {VALUES, "VALUES", opValues, nil},

		//Stack isolation
		CALL_I:   {CALL_I, "CALL_I", opCallI, validateInvocationStack},
		CALL_E:   {CALL_E, "CALL_E", opCallE, validateInvocationStack},
		CALL_ED:  {CALL_ED, "CALL_E", opCallE, validateInvocationStack},
		CALL_ET:  {CALL_ET, "CALL_E", opCallE, validateInvocationStack},
		CALL_EDT: {CALL_EDT, "CALL_E", opCallE, validateInvocationStack},

		NEWSTRUCT: {NEWSTRUCT, "NEWSTRUCT", opNewArray, validateNewArray},
		//Map
		NEWMAP: {NEWMAP, "NEWMAP", opNewMap, nil},
		APPEND: {APPEND, "APPEND", opAppend, validateAppend},

		THROW: {THROW, "THROW", OpThrow, nil},
		THROWIFNOT: {THROWIFNOT, "THROWIFNOT", OpThowIfNot, nil},
	}
)

func GetOPCodeByName(name string) (OpCode, error) {
	for _, opexec := range OpExecList {
		if opexec.Name == name {
			return opexec.Opcode, nil
		}
	}
	return NOP, errors.New("error opcode name")
}