# Token Sidechain - elastos.org
# This is an official but unsupported docker image

FROM golang:1.13-alpine3.10 AS builder

LABEL maintainer="kpachhai"

RUN apk update 
RUN apk add --no-cache make 
RUN apk add --no-cache curl
RUN apk add --no-cache git 
RUN apk add --no-cache gcc 
RUN apk add --no-cache libc-dev

ENV SRC_DIR /go/src/github.com/elastos
ENV RELEASE 0.1.2

ADD https://github.com/elastos/Elastos.ELA.SideChain.Token/archive/v${RELEASE}.tar.gz ${SRC_DIR}/

# install Glide
RUN curl https://glide.sh/get | sh

# build env
ENV GOPATH="/go"
ENV GOROOT="/usr/local/go"
ENV GOBIN="$GOPATH/bin"
ENV PATH="$GOROOT/bin:$PATH"
ENV PATH="$GOBIN:$PATH"

WORKDIR ${SRC_DIR}

RUN tar -xzvf v${RELEASE}.tar.gz && \
	mv Elastos.ELA.SideChain.Token-${RELEASE} Elastos.ELA.SideChain.Token

RUN cd ${SRC_DIR}/Elastos.ELA.SideChain.Token && \
        printf 'ignore:\n- github.com/russross/blackfriday/v2\n' >> glide.yaml && \
        glide update && glide install && \
		make

# alpine3.10
FROM alpine:3.10

ENV SRC_DIR="/token"

WORKDIR $SRC_DIR

COPY --from=builder /go/src/github.com/elastos/Elastos.ELA.SideChain.Token/token ${SRC_DIR}/token

RUN apk update \
    && apk add --no-cache curl ca-certificates \
    && addgroup -g 1000 -S elauser \
    && adduser -h $SRC_DIR -u 1000 -S elauser -G elauser \
    && chown -R elauser:elauser $SRC_DIR
    
USER elauser

EXPOSE 20614-20618

ENTRYPOINT ["/bin/sh", "-c", "./token"]
