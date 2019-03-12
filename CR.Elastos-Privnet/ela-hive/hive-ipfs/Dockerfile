FROM busybox:1-glibc
LABEL maintainer="kpachhai"

ENV TINI_VERSION v0.18.0
ENV IPFS_PATH /data/ipfs
ENV IPFS_LOGGING ""

# Get the ipfs cluster binary and entrypoint script from the build container.
COPY ./cmd/ipfs/ipfs /usr/local/bin/ipfs
COPY ./bin/container_daemon /usr/local/bin/start_ipfs
COPY ./sbin/su-exec /sbin/su-exec
COPY ./swarm.key $IPFS_PATH/.ipfs/swarm.key
COPY ./data-ipfs $IPFS_PATH

# This shared lib (part of glibc) doesn't seem to be included with busybox.
COPY ./lib/x86_64-linux-gnu/libdl-2.27.so /lib/libdl.so.2

ADD https://github.com/krallin/tini/releases/download/${TINI_VERSION}/tini /tini
RUN chmod +x /tini

# Swarm TCP; should be exposed to the public
EXPOSE 4001
# Daemon API; must not be exposed publicly but to client services under you control
EXPOSE 5001
# Web Gateway; can be exposed publicly with a proxy, e.g. as https://ipfs.example.org
EXPOSE 8080
# Swarm Websockets; must be exposed publicly when the node is listening using the websocket transport (/ipX/.../tcp/8081/ws).
EXPOSE 8081

# Create the fs-repo directory and switch to a non-privileged user.
RUN mkdir -p $IPFS_PATH \
  && adduser -D -h $IPFS_PATH -u 1000 -G users ipfs \
  && chown -R ipfs:users $IPFS_PATH

USER ipfs 

WORKDIR $IPFS_PATH

# This just makes sure that:
# 1. There's an fs-repo, and initializes one if there isn't.
# 2. The API and Gateway are accessible from outside the container.
ENTRYPOINT ["/tini", "--", "/usr/local/bin/start_ipfs"]

# Execute the daemon subcommand by default
CMD ["daemon", "--migrate=true"]