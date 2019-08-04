# Build Geth in a stock Go builder container
FROM golang:1.11-alpine as builder

RUN apk add --no-cache make gcc musl-dev linux-headers

ADD . /Elastos.ELA.SideChain.ETH
RUN cd /Elastos.ELA.SideChain.ETH && make geth

# Pull Geth into a second stage deploy alpine container
FROM alpine:latest

RUN apk add --no-cache ca-certificates
COPY --from=builder /Elastos.ELA.SideChain.ETH/build/bin/geth /usr/local/bin/

EXPOSE 20636 20635 20638 20638/udp
ENTRYPOINT ["geth"]
