# Elastos Wallet Development Environment Document

## 开发服务器获取

1. 使用邮箱将 `ssh public key` 发送给 `lifayi@elastos.org` 请求分配开发服务器，管理员分配成功后回复相应的服务器地址和访问方法。

    通常推荐使用证书方式登录服务器，如果需要使用密码登录也可以在邮件中进行说明。

2. 根据需求后会给您分配两台服务器：`链节点服务器` 和 `钱包服务器`。

## ELA节点服务

### 环境说明

1. 应用程序默认放置在 `/opt/ela_node` 目录中，一个运行中的包含命令行客户端的节点目录内容如下：

    ```bash
    ls -al /opt/ela_node

    total 24924
    drwxrwxr-x 4 elastos elastos     4096 Apr 20 17:19 ./
    drwxrwxr-x 5 elastos elastos     4096 Apr 20 11:50 ../
    drwxr-xr-x 2 elastos elastos     4096 Apr 20 12:08 Chain/
    -rw-rw-r-- 1 elastos elastos       34 Apr 20 11:51 cli-config.json
    -rw-rw-r-- 1 elastos elastos     1088 Apr 20 12:02 config.json
    -rwxrwxr-x 1 elastos elastos 13565344 Apr 20 11:51 ela-cli*
    -rw-rw-r-- 1 elastos elastos      444 Apr 20 12:01 keystore.dat
    drwxrw-r-- 2 elastos elastos     4096 Apr 20 12:07 Log/
    -rwxrwxr-x 1 elastos elastos 11648196 Apr 20 11:50 node*
    -rw-r--r-- 1 elastos elastos   274432 Apr 20 17:19 wallet.db
    ```

   * `node` 是链节点的启动程序，相应的配置文件为 `config.json`。

   * `ela-cli` 是一个命令行管理工具，可以用来创建钱包，转账和查看区块数据，配置文件为`cli-config.json`。

   * `keystore.dat` 和 `wallet.db` 是钱包的秘钥和钱包数据。

   * `Chain` 和 `Log` 目录是区块链账本数据以及日志数据。

   * 链节点和命令行客户端代码和编译方法以及配置和命令行参数可以参考：

     * [ela](https://github.com/elastos/Elastos.ELA/blob/master/README.md)
     * [ela-cli](https://github.com/elastos/Elastos.ELA/blob/master/docs/cli_user_guide_CN.md)

2. 默认情况下分配给你的节点服务器会连接到一个专门用于开发的一组节点服务器，自动挖矿并且把所获取的币发送到本地的钱包中；你可以参考 ela-cli 中的相关查询命令进行查询或者其他操作。

   备注：部分命令行操作需要提供一个钱包密码，默认是 `elastos`。

3. 节点服务器使用如下端口提供服务：

    ```bash
    2[*]334  提供 Http Rest api
    2[*]335  提供 Web Socket api
    2[*]336  用于命令行客户端连接到节点获取数据
    2[*]338  提供节点之间的数据同步
    ```

   备注：`*` 表示这个数字不确定，会在成功分配服务器后确定。

### Node API

可参考API示例：

* [Restful_API](Restful_API_CN.md)

## ELA钱包服务

1. ELA钱包服务包含两个前台的服务：`browser` 和 `wallet`；以及另外三个幕后工作的服务 `worker`、 `api`、 `notification`。

2. 服务的职责简介：

    ```bash
    browser:  区块浏览器用户界面，可以查看区块数据，查询交易
    wallet:   web钱包用户界面，可以进行钱包创建，资产查询，转账操作
    worker:   从节点获取区块信息，写入缓存数据库
    api:      为 wallet 提供接口
    notification:
    ```

3. 上面的这些服务的应用程序都在 `/data/www/` 目录下并使用 `xxxx.test.elastos.org` 作为目录名称；目前所有服务都是使用 `supervisor` 进行管理，相关配置文件在 `/etc/supervisor/conf.d` 目录下，可以使用 `supervisorctl [start|stop|restart] <service_name>` 来启动或者停止某个服务：

    ```bash
    supervisorctl stop worker.test.elastos.org

    supervisorctl restart all
    ```

   使用 `supervisor` 管理服务的详细文档可以在 [这里](http://www.supervisord.org/) 查看。

4. ELA钱包的 `web` 服务器使用 `nginx`, 相关配置文件在 `/etc/nginx/conf.d` 目录。

   开发环境的 `https` 服务使用了自签名的证书，所以使用浏览器进行访问时需要手动添加一下对证书的信任，另外分配的开发环境默认不会使用标准的`80` `443`端口，所以访问时候要加上相应的端口。
