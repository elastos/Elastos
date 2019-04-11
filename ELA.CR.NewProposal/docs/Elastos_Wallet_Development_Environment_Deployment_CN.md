# Elastos Wallet Development Environment Deployment

## 钱包部署

1. 参考 `docker` 官方 [安装文档](https://docs.docker.com/install/linux/docker-ce/ubuntu/)，安装 `Docker CE`

2. 准备相关文件

    ```bash
    $ sudo ls -al /opt/docker_ela_wallet

    total 36
    drwxrwxr-x 6 elastos elastos 4096 Apr 20 10:18 ./
    drwxr-xr-x 3 root   root   4096 Apr 20 14:05 ../
    -rw-rw-r-- 1 elastos elastos  395 Apr 19 16:50 authorized_keys
    -rw-rw-r-- 1 elastos elastos  502 Apr 20 10:18 Dockerfile
    drwxrwxr-x 2 elastos elastos 4096 Apr 17 16:05 elastos.org/
    drwxrwxr-x 2 elastos elastos 4096 Apr 17 16:05 nginx/
    drwxrwxr-x 2 elastos elastos 4096 Apr 19 18:09 supervisor/
    -rwxrwxr-x 1 elastos elastos 1314 Apr 19 18:55 walletWrapper.sh*
    drwxr-xr-x 7 elastos elastos 4096 Apr 13 12:16 www/

    # authorized_keys 文件包含用户提供的 openssh 公钥，启动镜像时会映射到镜像中，这样用户可以通过 openssh 连接到容器
    # elastos.org 目录中包含 https 访问钱包和浏览器所需的服务器证书和私钥
    # nginx 目录中包含钱包相关应用的配置文件，可以从钱包相关代码中获得
    # supervisor 目录中包含 supervisor 相关配置文件，可以从钱包代码中获得
    # www 目录中包含钱包相关应用的代码
    ```

   **`Dockerfile 文件内容如下:`**

    ```Dockerfile
    FROM ubuntu:16.04

    RUN apt-get update && apt-get install -y vim net-tools openssh-server iputils-ping

    RUN apt-get install -y libxml2-dev libxslt-dev python-dev python-pip libjpeg-dev libcurl4-openssl-dev libgeos-dev libmysqlclient-dev supervisor python nginx mongodb-server redis-server curl rsyslog

    RUN curl -sL https://deb.nodesource.com/setup_6.x | bash - && apt-get install nodejs

    COPY walletWrapper.sh walletWrapper.sh

    CMD ["./walletWrapper.sh"]
    ```

   **`walletWrapper.sh 文件内容如下:`**

    ```bash
    #!/bin/bash

    # Start the sshd
    /etc/init.d/ssh start
    status=$?
    if [ $status -ne 0 ]; then
      echo "Failed to start sshd: $status"
      exit $status
    fi

    # Start the all service
    /etc/init.d/rsyslog start
    if [ $? -ne 0 ];then
      echo "Rsyslog start failed"
      exit 1
    fi

    /etc/init.d/nginx start
    if [ $? -ne 0 ];then
      echo "Nginx start failed"
      exit 1
    fi

    /etc/init.d/mongodb start
    if [ $? -ne 0 ];then
      echo "Mongodb start failed"
      exit 1
    fi

    /etc/init.d/redis-server start
    if [ $? -ne 0 ];then
      echo "Redis start failed"
      exit 1
    fi

    sleep 20

    /etc/init.d/supervisor start
    if [ $? -ne 0 ];then
      echo "Supervisor start failed"
      exit 1
    fi

    while sleep 60; do
      ps aux |grep sshd |grep -q -v grep
      PROCESS_1_STATUS=$?
      # If the greps above find anything, they exit with 0 status
      # If they are not both 0, then something is wrong
      if [ $PROCESS_1_STATUS -ne 0 ]; then
        echo "Process has already exited."
        exit 1
      fi
    done
    ```

3. 创建 `docker` 镜像

    ```bash
    $ cd /opt/docker_ela_wallet

    $ docker build -t ela_wallet_run_01 .
    # 这里在安装nodejs时可能会失败，需要通过代理连接
    ```

4. 启动 `docker` 镜像

    ```bash
    $ docker run -m 2g --cpus=2 -p 922:22 -p 20443:443 -p 20080:80 \
    -v /opt/docker_ela_wallet/authorized_keys:/root/.ssh/authorized_keys \
    -v /opt/docker_ela_wallet/elastos.org:/etc/ssl/elastos.org \
    -v /opt/docker_ela_wallet/nginx:/etc/nginx/conf.d \
    -v /opt/docker_ela_wallet/supervisor:/etc/supervisor/conf.d \
    -v /opt/docker_ela_wallet/www:/data/www \
    ela_wallet_run_01
    ```

   * 这里限制容器可以使用的内存数量为2G，使用的cpu数量为2个.
   * 注意： 如果把所有服务都部署在同一台服务器的话，需要修改 `notification` 的监听端口（否则会和 `api` 端口冲突），然后修改 `api` 服务连接 `notificationUrl`的端口.

## 链节点部署

1. 参考 `docker` 官方 [安装文档](https://docs.docker.com/install/linux/docker-ce/ubuntu/)，安装 `Docker CE`

2. 准备相关文件

    ```bash
    $ sudo ls -al /opt/docker_ela_node/

    total 24
    drwxrwxr-x 3 elastos elastos 4096 Apr 20 10:17 ./
    drwxr-xr-x 4 root   root   4096 Apr 20 14:42 ../
    -rw-rw-r-- 1 elastos elastos  395 Apr 19 19:18 authorized_keys
    -rw-rw-r-- 1 elastos elastos  213 Apr 19 19:34 Dockerfile
    drwxrwxr-x 4 elastos elastos 4096 Apr 19 19:37 ela_node/
    -rwxrwxr-x 1 elastos elastos 1221 Apr 17 11:56 nodeWrapper.sh*

    # authorized_keys 文件包含用户提供的 openssh 公钥，创建镜像时会复制到镜像中，这样用户可以通过 openssh 连接到容器
    # ela_node 目录中包含链节点相关应用和配置文件
    ```

   * 链节点和命令行客户端代码和编译方法以及配置和命令行参数可以参考:

     * [ela](../README.md)
     * [ela-cli](https://github.com/elastos/Elastos.ELA/blob/master/docs/cli_user_guide_CN.md)

   **`Dockerfile文件内容如下`**

    ```Dockerfile
    FROM ubuntu:16.04

    RUN apt-get update && apt-get install -y vim net-tools openssh-server iputils-ping

    COPY nodeWrapper.sh nodeWrapper.sh

    CMD ["./nodeWrapper.sh"]
    ```

   **`nodeWrapper.sh文件内容如下`**

    ```bash
    #!/bin/bash

    # Start the sshd
    /etc/init.d/ssh start
    status=$?
    if [ $status -ne 0 ]; then
      echo "Failed to start sshd: $status"
      exit $status
    fi

    # Start the ELA_NODE
    if [ ! -d "/opt/ela_node" ];then
      echo "ELA_NODE program not exists";
      exit
    fi

    cd /opt/ela_node/ && ./node -p elastos > /dev/null &

    status=$?
    if [ $status -ne 0 ]; then
      echo "Failed to start ELA_NODE: $status"
      exit $status
    fi

    while sleep 60; do
      ps aux |grep sshd |grep -q -v grep
      PROCESS_1_STATUS=$?
      ps aux |grep "./node" |grep -q -v grep
      PROCESS_2_STATUS=$?
      if [ $PROCESS_1_STATUS -ne 0 -o $PROCESS_2_STATUS -ne 0 ]; then
        echo "One of the processes has already exited."
        exit 1
      fi
    done
    ```

3. 创建 `docker` 镜像

    ```bash
    cd /opt/docker_ela_node
    $ docker build -t ela_node_run_01 .
    ```

4. 启动 `docker` 镜像

    ```bash
    $ docker run -m 512m --cpus=2 -p 922:22 -p 20338:20338 -p 20334:20334 -p 20335:20335 \
    -v /opt/docker_ela_node/authorized_keys:/root/.ssh/authorized_keys \
    -v /opt/docker_ela_node/ela_node:/opt/ela_node \
    ela_node_run_01
    ```