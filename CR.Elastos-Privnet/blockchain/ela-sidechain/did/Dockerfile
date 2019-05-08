# Elastos privnet - DID Sidechain - elastos.org
# This is an official but unsupported docker image

FROM golang@sha256:8dea7186cf96e6072c23bcbac842d140fe0186758bcc215acb1745f584984857 AS builder

LABEL maintainer="kpachhai"

RUN apk update 
RUN apk add --no-cache curl 
RUN apk add --no-cache make 
RUN apk add --no-cache git 
RUN apk add --no-cache gcc 
RUN apk add --no-cache libc-dev

# copy folders
COPY did /go/src/github.com/elastos/Elastos.ELA.SideChain.ID

# build env
ENV GOPATH="/go"
ENV GOROOT="/usr/local/go"
ENV GOBIN="$GOPATH/bin"
ENV PATH="$GOROOT/bin:$PATH"
ENV PATH="$GOBIN:$PATH"

# install Glide
RUN curl https://glide.sh/get | sh

# cwd
WORKDIR /go/src/github.com/elastos/Elastos.ELA.SideChain.ID

RUN rm -rf vendor glide.lock 
RUN glide cc 
RUN glide update
RUN glide install
RUN make

# alpine3.8 - use image digest for security
FROM alpine@sha256:dad671370a148e9d9573e3e10a9f8cc26ce937bea78f3da80b570c2442364406

ENV SRC_DIR="/root/did"

WORKDIR $SRC_DIR

COPY --from=builder /go/src/github.com/elastos/Elastos.ELA.SideChain.ID/did ${SRC_DIR}/did

RUN apk update

EXPOSE 10011-10015

ENTRYPOINT ["/bin/sh", "-c", "ls /root/did/elastos_did && ./did"]