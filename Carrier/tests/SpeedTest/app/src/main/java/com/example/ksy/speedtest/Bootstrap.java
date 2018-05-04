package com.example.ksy.speedtest;

/*
public class Bootstrap {
    boolean enableUdp;
    int logLevel;
    String logFile;
    String dataDir;
}
*/

public class Bootstrap {
    private String ipv4;
    private int port;
    private String pubicKey;

    public Bootstrap() {
        setIpv4(null);
        setPort(0);
        setPubicKey(null);
    }

    public Bootstrap(String ipv4, int port, String pubicKey) {
        this.setIpv4(ipv4);
        this.setPort(port);
        this.setPubicKey(pubicKey);
    }


    public String getIpv4() {
        return ipv4;
    }

    public void setIpv4(String ipv4) {
        this.ipv4 = ipv4;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public String getPubicKey() {
        return pubicKey;
    }

    public void setPubicKey(String pubicKey) {
        this.pubicKey = pubicKey;
    }
}