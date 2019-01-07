package service

type RegisterIdentificationValueInfo struct {
	DataHash string `json:"datahash"`
	Proof    string `json:"proof"`
	Info     string `json:"info"`
}

type RegisterIdentificationContentInfo struct {
	Path   string                            `json:"path"`
	Values []RegisterIdentificationValueInfo `json:"values"`
}

type RegisterIdentificationInfo struct {
	Id       string                              `json:"id"`
	Sign     string                              `json:"sign"`
	Contents []RegisterIdentificationContentInfo `json:"contents"`
}
