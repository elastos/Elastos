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
	"os"
	"path/filepath"
	"strings"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/api/types/mount"
	"github.com/docker/docker/api/types/strslice"
	"github.com/docker/docker/client"
	"github.com/docker/go-connections/nat"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

// RunCmd represents the run command
var RunCmd = &cobra.Command{
	Use:   "run",
	Short: "Run blockchain nodes",
	Long:  `Run blockchain nodes`,
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Printf("blockchain run called with environment: [%s] and nodes: [%s]\n", Env, Nodes)

		if !strings.EqualFold(Env, "mainnet") &&
			!strings.EqualFold(Env, "testnet") &&
			!strings.EqualFold(Env, "localnet") {
			log.Fatalf("%s not recognized as a valid net type\n", Env)
		}

		ctx := context.Background()
		cli, err := client.NewClientWithOpts(client.FromEnv, client.WithAPIVersionNegotiation())
		if err != nil {
			log.Fatal(err)
		}

		nodes := strings.Split(strings.Replace(Nodes, " ", "", -1), ",")
		if strings.EqualFold(Env, "localnet") {
			setupLocalNetDockerContainers(ctx, cli)
			fmt.Printf("\nSet up initial localnet successfully")
		} else {
			for _, node := range nodes {
				if node == "eth" && strings.EqualFold(Env, "mainnet") {
					log.Fatalf("%s not recognized as a valid net type for %s\n", Env, node)
				}
				if node == "mainchain" || node == "did" || node == "token" || node == "eth" {
					if resp, err := runDockerContainer(ctx, cli, node); err != nil {
						log.Print(err)
					} else {
						fmt.Printf("\nNetwork: %s\nContainer Type: %s\nContainer ID: %v\n", Env, node, resp.ID[:10])
					}
				} else {
					log.Fatalf("%s not recognized as a valid node type\n", node)
				}
			}
		}
	},
}

// Setup required nodes for Local Net
func setupLocalNetDockerContainers(ctx context.Context, cli *client.Client) {
	var (
		resp container.ContainerCreateCreatedBody
		err  error
	)

	// Create a new network to be used for the localnet
	networkResp, err := cli.NetworkCreate(ctx, "develap-localnet-blockchain-network", types.NetworkCreate{})
	if err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	}

	// Setup mainchain nodes
	if resp, err = setupLocalNetMainchainNode(ctx, cli, networkResp, "crc-1"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "crc-1", resp.ID[:10])
	}
	if resp, err = setupLocalNetMainchainNode(ctx, cli, networkResp, "crc-2"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "crc-2", resp.ID[:10])
	}
	if resp, err = setupLocalNetMainchainNode(ctx, cli, networkResp, "origin-1"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "origin-1", resp.ID[:10])
	}
	if resp, err = setupLocalNetMainchainNode(ctx, cli, networkResp, "origin-2"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "origin-2", resp.ID[:10])
	}
}

func setupLocalNetMainchainNode(ctx context.Context, cli *client.Client, networkResp types.NetworkCreateResponse, name string) (container.ContainerCreateCreatedBody, error) {
	var (
		resp container.ContainerCreateCreatedBody
		err  error
	)

	dockerContainer := DockerContainer{
		ContainerName: fmt.Sprintf("develap-localnet-mainchain-%s", name),
		ImageName:     NodeDockerImageMap["mainchain"],
		Volumes: map[string]DockerContainerDataDir{
			filepath.FromSlash(fmt.Sprintf("%s/data/localnet/mainchain/%s", CurrentDir, name)):         DockerContainerDataDir{true, NodeDockerDataPathMap["mainchain"]},
			filepath.FromSlash(fmt.Sprintf("%s/localnet/mainchain/%s/config.json", CurrentDir, name)):  DockerContainerDataDir{false, NodeDockerConfigPathMap["mainchain"]},
			filepath.FromSlash(fmt.Sprintf("%s/localnet/mainchain/%s/keystore.dat", CurrentDir, name)): DockerContainerDataDir{false, NodeDockerKeystorePathMap["mainchain"]},
		},
		ContainerExposedPorts: nat.PortSet{},
		HostPortMappings:      nat.PortMap{},
	}

	// Pull the image from dockerhub
	_, err = cli.ImagePull(ctx, dockerContainer.ImageName, types.ImagePullOptions{})
	if err != nil {
		return resp, err
	}
	// Create appropriate data directory that will be mounted between host and container
	// Also, create the mount points in the process
	var mounts = []mount.Mount{}
	for hostPath, volume := range dockerContainer.Volumes {
		if volume.HostCreate {
			os.MkdirAll(hostPath, os.ModePerm)
		}
		// Create mountpoints
		mounts = append(mounts, mount.Mount{
			Type:   mount.TypeBind,
			Source: hostPath,
			Target: volume.ContainerPath,
		})
	}
	// Create the container
	resp, err = cli.ContainerCreate(
		ctx,
		&container.Config{
			Hostname:     dockerContainer.ContainerName,
			Image:        dockerContainer.ImageName,
			ExposedPorts: dockerContainer.ContainerExposedPorts,
		},
		&container.HostConfig{
			PortBindings: dockerContainer.HostPortMappings,
			Mounts:       mounts,
		},
		nil,
		dockerContainer.ContainerName,
	)
	if err != nil {
		return resp, err
	}
	// Start the container
	if err = cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		return resp, err
	}
	// Connect to the "blockchain" network
	if err = cli.NetworkConnect(ctx, networkResp.ID, resp.ID, nil); err != nil {
		return resp, err
	}

	return resp, nil
}

func runDockerContainer(ctx context.Context, cli *client.Client, node string) (container.ContainerCreateCreatedBody, error) {
	var (
		resp container.ContainerCreateCreatedBody
		err  error
	)
	imageName := NodeDockerImageMap[node]
	_, err = cli.ImagePull(ctx, imageName, types.ImagePullOptions{})
	if err != nil {
		return resp, err
	}

	var containerRESTPort, containerRPCPort, hostRESTPort, hostRPCPort nat.Port
	if node == "mainchain" {
		containerRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortMainChain[Env].ContainerRESTPort))
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortMainChain[Env].ContainerRPCPort))
		hostRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortMainChain[Env].HostRESTPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortMainChain[Env].HostRPCPort))
	} else if node == "did" {
		containerRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainDID[Env].ContainerRESTPort))
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainDID[Env].ContainerRPCPort))
		hostRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainDID[Env].HostRESTPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainDID[Env].HostRPCPort))
	} else if node == "token" {
		containerRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainToken[Env].ContainerRESTPort))
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainToken[Env].ContainerRPCPort))
		hostRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainToken[Env].HostRESTPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainToken[Env].HostRPCPort))
	} else if node == "eth" {
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainEth[Env].ContainerRPCPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPortSidechainEth[Env].HostRPCPort))
	}

	currentDir, err := os.Getwd()
	if err != nil {
		return resp, err
	}
	volumeData := filepath.FromSlash(fmt.Sprintf("%s/data/%s/%s", currentDir, Env, node))
	os.MkdirAll(volumeData, os.ModePerm)
	mounts := []mount.Mount{
		{
			Type:   mount.TypeBind,
			Source: volumeData,
			Target: NodeDockerDataPathMap[node],
		},
	}
	containerConfig := &container.Config{
		Image:        imageName,
		ExposedPorts: nat.PortSet{},
	}
	hostConfig := &container.HostConfig{
		PortBindings: nat.PortMap{
			containerRESTPort: []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRESTPort.Port()}},
			containerRPCPort:  []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRPCPort.Port()}},
		},
		Mounts: mounts,
	}
	if node == "eth" {
		if Env == "testnet" {
			containerConfig.Entrypoint = strslice.StrSlice{"/bin/sh"}
			containerConfig.Cmd = strslice.StrSlice{
				"-c", "./geth --testnet --datadir elastos_eth --gcmode 'archive' --rpc --rpcaddr 0.0.0.0 --rpccorsdomain '*' --rpcvhosts '*' --rpcport 20636 --rpcapi 'eth,net,web3' --ws --wsaddr 0.0.0.0 --wsorigins '*' --wsport 20635 --wsapi 'eth,net,web3'",
			}
		}
		containerConfig.ExposedPorts = nat.PortSet{
			containerRPCPort: struct{}{},
		}
		hostConfig.PortBindings = nat.PortMap{
			containerRPCPort: []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRPCPort.Port()}},
		}
	} else {
		if node == "mainchain" {
			mounts = append(mounts,
				mount.Mount{
					Type:   mount.TypeBind,
					Source: filepath.FromSlash(fmt.Sprintf("%s/%s/%s/config.json", currentDir, Env, node)),
					Target: NodeDockerConfigPathMap[node],
				},
				mount.Mount{
					Type:   mount.TypeBind,
					Source: filepath.FromSlash(fmt.Sprintf("%s/%s/%s/keystore.dat", currentDir, Env, node)),
					Target: "/ela/keystore.dat",
				},
			)
		} else {
			mounts = append(mounts, mount.Mount{
				Type:   mount.TypeBind,
				Source: filepath.FromSlash(fmt.Sprintf("%s/%s/%s/config.json", currentDir, Env, node)),
				Target: NodeDockerConfigPathMap[node],
			})
		}
		containerConfig.ExposedPorts = nat.PortSet{
			containerRESTPort: struct{}{},
			containerRPCPort:  struct{}{},
		}
		hostConfig.PortBindings = nat.PortMap{
			containerRESTPort: []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRESTPort.Port()}},
			containerRPCPort:  []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRPCPort.Port()}},
		}
	}

	containerName := fmt.Sprintf("develap-%s-%s-node", Env, node)

	resp, err = cli.ContainerCreate(
		ctx,
		containerConfig,
		hostConfig,
		nil,
		containerName,
	)
	if err != nil {
		return resp, err
	}
	if err := cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		return resp, err
	}
	return resp, nil
}

func init() {
	RunCmd.Flags().StringVarP(&Nodes, "nodes", "n", "", "Nodes to use [mainchain,did,token,eth]")
	RunCmd.MarkFlagRequired("nodes")
}
