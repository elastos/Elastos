# Elastos privnet - DID Sidechain - elastos.org
# This is an official but unsupported docker image

# alpine3.8 - use image digest for security
FROM node:11.15.0-alpine

ENV SRC_DIR="/oracle"

WORKDIR $SRC_DIR

COPY * ${SRC_DIR}/

RUN apk update \
    && apk add --no-cache curl \ 
    && npm install web3 express \
    && adduser -h $SRC_DIR -D -g '' elauser \
    && chown -R elauser:elauser $SRC_DIR
    
USER elauser

EXPOSE 16666

ENTRYPOINT ["/bin/sh", "-c", "node crosschain_oracle.js"]