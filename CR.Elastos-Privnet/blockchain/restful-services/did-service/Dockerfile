# Elastos privnet - DID Service - elastos.org
# This is an official but unsupported docker image

FROM maven:3.6.0-jdk-8-alpine

ENV SRC_DIR="/home/elauser"

WORKDIR $SRC_DIR

COPY . .

RUN apk update \
    && apk add --no-cache curl \
    && adduser -h $SRC_DIR -D -g '' elauser \
    && chown -R elauser:elauser $SRC_DIR

USER elauser

EXPOSE 8092

ENTRYPOINT ["java", "-jar", "./did.api-0.0.6.jar"]