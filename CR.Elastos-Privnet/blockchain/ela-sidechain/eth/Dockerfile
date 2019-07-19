# Elastos privnet - DID Sidechain - elastos.org
# This is an official but unsupported docker image

FROM golang@sha256:8dea7186cf96e6072c23bcbac842d140fe0186758bcc215acb1745f584984857 AS builder

LABEL maintainer="kpachhai"

RUN apk update
RUN apk add --no-cache curl 
RUN apk add --no-cache git
RUN apk add --no-cache make 
RUN apk add --no-cache gcc
RUN apk add --no-cache musl-dev
RUN apk add --no-cache linux-headers

# copy folders
COPY eth /go/src/github.com/elastos/Elastos.ELA.SideChain.ETH

# build env
ENV GOPATH="/go"
ENV GOROOT="/usr/local/go"
ENV GOBIN="$GOPATH/bin"
ENV PATH="$GOROOT/bin:$PATH"
ENV PATH="$GOBIN:$PATH"

# install Glide
RUN curl https://glide.sh/get | sh

# cwd
WORKDIR /go/src/github.com/elastos/Elastos.ELA.SideChain.ETH

RUN rm -rf vendor glide.lock 
RUN glide cc 
RUN glide update
RUN glide install
RUN make geth

# alpine3.8 - use image digest for security
FROM alpine@sha256:dad671370a148e9d9573e3e10a9f8cc26ce937bea78f3da80b570c2442364406

ENV SRC_DIR="/eth"

WORKDIR $SRC_DIR

COPY --from=builder /go/src/github.com/elastos/Elastos.ELA.SideChain.ETH/build/bin/geth ${SRC_DIR}/geth

RUN apk update \
    && apk add --no-cache curl ca-certificates \
    && adduser -h $SRC_DIR -D -g '' elauser \
    && chown -R elauser:elauser $SRC_DIR
    
USER elauser

EXPOSE 8545 30303 30303/udp

ENTRYPOINT ["/bin/sh", "-c", "./geth --mine --datadir elastos_eth --ethash.dagdir elastos_ethash --rpc --rpcaddr 0.0.0.0 --rpccorsdomain '*' --rpcvhosts '*' --rpcport 8545 --rpcapi 'personal,db,eth,net,web3,txpool,miner' --unlock 0x961386e437294f9171040e2d56d4522c4f55187d --password ./eth-accounts-pass.txt"]