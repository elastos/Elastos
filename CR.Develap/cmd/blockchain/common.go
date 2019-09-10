/*
Copyright © 2019 Cyber Republic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
package blockchain

type DockerPort struct {
	ContainerRESTPort string
	ContainerRPCPort  string
	HostRESTPort      string
	HostRPCPort       string
}

var (
	Env                string
	Nodes              string
	NodeDockerImageMap = map[string]string{
		"mainchain": "cyberrepublic/elastos-mainchain-node:v0.3.7",
		"did":       "cyberrepublic/elastos-sidechain-did-node:v0.1.2",
		"token":     "cyberrepublic/elastos-sidechain-token-node:v0.1.2",
		"eth":       "cyberrepublic/elastos-sidechain-eth-node:v0.0.1",
	}
	NodeDockerDataPathMap = map[string]string{
		"mainchain": "/ela/elastos",
		"did":       "/did/elastos_did",
		"token":     "/token/elastos_token",
		"eth":       "/eth/elastos_eth",
	}
	NodeDockerConfigPathMap = map[string]string{
		"mainchain": "/ela/config.json",
		"did":       "/did/config.json",
		"token":     "/token/config.json",
	}
	NodeDockerPortMainChain = map[string]DockerPort{
		"mainnet": DockerPort{"20334", "20336", "20334", "20336"},
		"testnet": DockerPort{"21334", "21336", "21334", "21336"},
	}
	NodeDockerPortSidechainDID = map[string]DockerPort{
		"mainnet": DockerPort{"20604", "20606", "20604", "20606"},
		"testnet": DockerPort{"21604", "21606", "21604", "21606"},
	}
	NodeDockerPortSidechainToken = map[string]DockerPort{
		"mainnet": DockerPort{"20614", "20616", "20614", "20616"},
		"testnet": DockerPort{"21614", "21616", "21614", "21616"},
	}
	NodeDockerPortSidechainEth = map[string]DockerPort{
		"mainnet": DockerPort{"20634", "20636", "20634", "20636"},
		"testnet": DockerPort{"20634", "20636", "21634", "21636"},
	}
)
