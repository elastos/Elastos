FROM busybox:1-glibc
LABEL maintainer="kpachhai"

ENV TINI_VERSION v0.18.0
ENV IPFS_CLUSTER_PATH /data/ipfs-cluster

# Get the ipfs binary and entrypoint script from the build container.
COPY ./cmd/ipfs-cluster-service/ipfs-cluster-service /usr/local/bin/ipfs-cluster-service
COPY ./cmd/ipfs-cluster-ctl/ipfs-cluster-ctl /usr/local/bin/ipfs-cluster-ctl
COPY ./entrypoint.sh /usr/local/bin/entrypoint.sh
COPY ./sbin/su-exec /sbin/su-exec
COPY ./data-ipfs-cluster $IPFS_CLUSTER_PATH

# This shared lib (part of glibc) doesn't seem to be included with busybox.
COPY ./lib/x86_64-linux-gnu/libdl-2.27.so /lib/libdl.so.2

ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini

EXPOSE 9094
EXPOSE 9095
EXPOSE 9096

RUN mkdir -p $IPFS_CLUSTER_PATH && \
    adduser -D -h $IPFS_CLUSTER_PATH -u 1000 -G users ipfs && \
    chown -R ipfs:users $IPFS_CLUSTER_PATH

USER ipfs 

WORKDIR $IPFS_CLUSTER_PATH

ENTRYPOINT ["/tini", "--", "/usr/local/bin/entrypoint.sh"]

# Defaults for ipfs-cluster-service go here
CMD ["daemon", "--upgrade"]
## Use the following line if this is a slave node
## CMD ["daemon", "--bootstrap /dns4/ela-hive-cluster/tcp/9096/ipfs/QmQt7khnFb3CTnLCjrzKcmAcUWPVFTR68pXfUjmQMxzL7H", "--upgrade"]