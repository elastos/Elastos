package avm

type TriggerType byte

const (
	Verification  TriggerType = iota
	VerificationR             = 1
	Application               = 16
	ApplicationR              = 17
)
