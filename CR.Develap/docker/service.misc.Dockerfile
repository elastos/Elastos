# API MISC Mainchain - elastos.org
# This is an official but unsupported docker image

FROM golang:1.13-alpine3.10 AS builder

LABEL maintainer="kpachhai"

RUN apk update 
RUN apk add --no-cache curl 
RUN apk add --no-cache make 
RUN apk add --no-cache git 

# copy folders
COPY service.misc /go/src/github.com/elastos/Elastos.ORG.API.Misc

# build env
ENV GOPATH="/go"
ENV GOROOT="/usr/local/go"
ENV GOBIN="$GOPATH/bin"
ENV PATH="$GOROOT/bin:$PATH"
ENV PATH="$GOBIN:$PATH"

# install Glide
RUN curl https://glide.sh/get | sh

# cwd
WORKDIR /go/src/github.com/elastos/Elastos.ORG.API.Misc

RUN glide update
RUN glide install
RUN go build -o misc

# alpine3.10
FROM alpine:3.10

ENV SRC_DIR="/home/elauser"

WORKDIR $SRC_DIR

COPY --from=builder /go/src/github.com/elastos/Elastos.ORG.API.Misc/misc ${SRC_DIR}/misc
COPY --from=builder /go/src/github.com/elastos/Elastos.ORG.API.Misc/docs/sql ${SRC_DIR}/sql

RUN apk update \
    && apk add --no-cache curl ca-certificates mysql-client \
    && addgroup -g 1000 -S elauser \
    && adduser -h $SRC_DIR -u 1000 -S elauser -G elauser \
    && chown -R elauser:elauser $SRC_DIR
    
USER elauser

EXPOSE 8080

CMD sh -c "ls /home/elauser/config.json && ./deploy.sh"