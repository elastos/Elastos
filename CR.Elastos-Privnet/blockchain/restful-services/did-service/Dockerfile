# Elastos privnet - DID Service - elastos.org
# This is an official but unsupported docker image

FROM maven:3.6.0-jdk-8-alpine AS builder

LABEL maintainer="kpachhai"

RUN apk update

# copy folders
COPY service /restful-services/did-service

RUN cp /restful-services/did-service/did.api/src/main/resources/application.properties.in /restful-services/did-service/did.api/src/main/resources/application.properties
RUN sed -i "s#@SIDE_REST_API@#http://ela-sidechain-did-1:10012#g" /restful-services/did-service/did.api/src/main/resources/application.properties
RUN cd /restful-services/did-service    \
    && mvn clean                        \
    && mvn install -Dmaven.test.skip -Dgpg.skip

FROM maven:3.6.0-jdk-8-alpine

ENV SRC_DIR="/home/elauser"

WORKDIR $SRC_DIR

COPY --from=builder /restful-services/did-service/did.api/target/did.api-0.0.6.jar ${SRC_DIR}/did.api-0.0.6.jar

RUN apk update \
    && apk add --no-cache curl \
    && adduser -h $SRC_DIR -D -g '' elauser \
    && chown -R elauser:elauser $SRC_DIR

USER elauser

EXPOSE 8080

ENTRYPOINT ["java", "-jar", "./did.api-0.0.6.jar"]