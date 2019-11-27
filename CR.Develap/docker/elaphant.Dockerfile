# API MISC Mainchain - elastos.org
# This is an official but unsupported docker image

FROM golang:1.13-alpine3.10 AS builder

LABEL maintainer="kpachhai"

RUN apk update 
RUN apk add --no-cache curl 
RUN apk add --no-cache make 
RUN apk add --no-cache git
RUN apk add --no-cache gcc
RUN apk add --no-cache musl-dev
RUN apk add --no-cache linux-headers 

# copy folders
COPY elaphant /go/src/github.com/elaphantapp/ElaphantNode

# install Glide
RUN curl https://glide.sh/get | sh

# cwd
WORKDIR /go/src/github.com/elaphantapp/ElaphantNode

RUN make

# alpine3.10
FROM alpine:3.10

ENV SRC_DIR="/elaphant"

WORKDIR $SRC_DIR

COPY --from=builder /go/src/github.com/elaphantapp/ElaphantNode/elaphant ${SRC_DIR}/elaphant

RUN apk update \
    && apk add --no-cache curl ca-certificates \
    && addgroup -g 1000 -S elauser \
    && adduser -h $SRC_DIR -u 1000 -S elauser -G elauser \
    && chown -R elauser:elauser $SRC_DIR
    
USER elauser

EXPOSE 8080

ENTRYPOINT ["/bin/sh", "-c", "./elaphant"]