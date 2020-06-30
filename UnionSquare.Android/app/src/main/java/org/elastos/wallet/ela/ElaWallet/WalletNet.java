/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ElaWallet;

/**
 * Enums often require more than twice as much memory as static constants.
 * 程序会根据包名自动分配这些  请勿修改此文件
 */
public class WalletNet {
    //-1alpha 默认0正式  1testnet 2 regtest  3私有链
    // 新添加的net必须保证更下面的不一样
    public static final int ALPHAMAINNET = -1;// Mainnet分身特殊配置  为了预演MainNet
    public static final int MAINNET = 0;
    public static final int TESTNET = 1;
    public static final int REGTESTNET = 2;
    public static final int PRVNET = 3;

    //中心化服务器地址
    public static final String SERVERLIST_BASE = "http://118.89.217.101:5739/";//负载均衡服务器地址
    public static final String MAINURL = "https://unionsquare01.elastos.com.cn";//mainnet高可用
    public static final String MAINURL1 = "https://unionsquare.elastos.org";//mainnet高可用1
    public static final String TESTURL = "https://123.207.167.100:442";
    public static final String REGTESTURL = "http://118.89.242.158";
    //  public static final String PRVURL = "http://node.longrunweather.com:18080";
    public static final String PRVURL = "http://cen.longrunweather.com:18080";




    //did resolve网址
    public static final String MAINDID = "http://api.elastos.io:20606";
    public static final String TESTDID = "http://api.elastos.io:21606";
    public static final String REGDID = "http://api.elastos.io:22606";
    //public static final String PRIDID = "http://drpc.longrunweather.com:18080";
    public static final String PRIDID = "http://did1rpc.longrunweather.com:18080";

    //wallet Config
    // config = "{\"ELA\":{\"ChainParameters\":{\"StandardPort\":40086,\"DNSSeeds\":[\"139.198.0.59\"]}},\"IDChain\":{\"ChainParameters\":{\"StandardPort\":40087,\"DNSSeeds\":[\"139.198.0.59\"]}}}";
    public static final String PRICONFIG = "{\"ELA\":{\"ChainParameters\":{\"MagicNumber\":20200501,\"StandardPort\":40008,\"DNSSeeds\":[\"longrunweather.com\"],\"CheckPoints\":[[0,\"d8d33c8a0a632ecc418bd7f09cd315dfc46a7e3e98e48c50c70a253e6062c257\",1513936800,486801407]]}},\"IDChain\":{\"ChainParameters\":{\"MagicNumber\":20200503,\"StandardPort\":41008,\"DNSSeeds\":[\"longrunweather.com\"],\"CheckPoints\":[[0,\"56be936978c261b2e649d58dbfaf3f23d4a868274f5522cd2adb4308a955c4a3\",1530360000,486801407]]}}}";

    //网站服务器地址
    public static final String WEBURlTEST = "http://crapi.longrunweather.com:18080";
    public static final String WEBURlPUBLIC = "https://api.cyberrepublic.org/";

}
