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

import (
	"fmt"
	"log"
	"strconv"
	"strings"
	"time"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/client"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

// ListCmd represents the list command
var ListCmd = &cobra.Command{
	Use:   "list",
	Short: "List blockchain nodes",
	Long:  `List blockchain nodes`,
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Printf("blockchain list called with environment: [%s]\n\n", Env)
		ctx := context.Background()
		cli, err := client.NewClientWithOpts(client.FromEnv, client.WithAPIVersionNegotiation())
		if err != nil {
			log.Fatal(err)
		}

		containers, err := cli.ContainerList(ctx, types.ContainerListOptions{})
		if err != nil {
			log.Fatal(err)
		}

		for _, container := range containers {
			for _, containerName := range container.Names {
				if strings.Contains(containerName, "develap") && strings.Contains(containerName, Env) {
					i, err := strconv.ParseInt(strconv.FormatInt(container.Created, 10), 10, 64)
					if err != nil {
						log.Fatal(err)
					}
					created := time.Unix(i, 0)
					ports := make(map[string]string, 0)
					for _, port := range container.Ports {
						if port.IP == "0.0.0.0" {
							portString := fmt.Sprintf("%v", port.PublicPort)
							if strings.Contains(containerName, "mainchain") {
								ports[portString] = getPortMapping(portString, NodeDockerPath["mainchain"].PortMapping)
							} else if strings.Contains(containerName, "did") {
								ports[portString] = getPortMapping(portString, NodeDockerPath["did"].PortMapping)
							} else if strings.Contains(containerName, "token") {
								ports[portString] = getPortMapping(portString, NodeDockerPath["token"].PortMapping)
							} else if strings.Contains(containerName, "eth") {
								ports[portString] = getPortMapping(portString, NodeDockerPath["eth"].PortMapping)
							}
						}
					}
					fmt.Printf("Name: %v\nID: %v\nImage: %v\nCmd: %v\nCreated: %v\nStatus: %v\nPorts: %v\n\n",
						containerName[1:], container.ID[:10], container.Image, container.Command,
						created, container.Status, ports)
					break
				}
			}

		}
	},
}

func getPortMapping(port string, nodeDockerPorts map[string]DockerPort) string {
	var portType string
	for _, dockerPorts := range nodeDockerPorts {
		if port == dockerPorts.HostRESTPort {
			portType = "REST"
		} else if port == dockerPorts.HostRPCPort {
			portType = "RPC"
		}
	}
	return portType
}

func init() {
}
