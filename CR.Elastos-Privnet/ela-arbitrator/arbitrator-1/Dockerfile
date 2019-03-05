# Elastos privnet - Arbiter 1 - elastos.org
# This is an official but unsupported docker image

# alpine3.8 - use image digest for security
FROM alpine@sha256:dad671370a148e9d9573e3e10a9f8cc26ce937bea78f3da80b570c2442364406

ENV SRC_DIR="/home/elauser"

WORKDIR $SRC_DIR

COPY . .

RUN apk update \
    && apk add --no-cache curl \
    && adduser -h $SRC_DIR -D -g '' elauser \
    && chown -R elauser:elauser $SRC_DIR

USER elauser

EXPOSE 10336
EXPOSE 10338

ENTRYPOINT ["/bin/sh", "-c", "./arbiter -p elastos"]