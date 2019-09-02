package org.elastos.carrier.common;

import java.io.File;
import java.util.ArrayList;

import org.elastos.carrier.Carrier.Options;
import org.elastos.carrier.Log;

public class TestOptions extends Options {
    private final String[] hiveAddresses = {"52.83.159.189","52.83.119.110","3.16.202.140","18.217.147.205","18.219.53.133"};
    private final String hivePort = "9095" ;

    public TestOptions(String path) {
        super();

        File file = new File(path);
        if (file.exists())
            file.delete();
        file.mkdir();

        try {
            setUdpEnabled(true);
            setPersistentLocation(path);

            ArrayList<BootstrapNode> arrayList = new ArrayList<>();
            BootstrapNode node = new BootstrapNode();
            node.setIpv4("13.58.208.50");
            node.setPort("33445");
            node.setPublicKey("89vny8MrKdDKs7Uta9RdVmspPjnRMdwMmaiEW27pZ7gh");
            arrayList.add(node);

            node = new BootstrapNode();
            node.setIpv4("18.216.102.47");
            node.setPort("33445");
            node.setPublicKey("G5z8MqiNDFTadFUPfMdYsYtkUDbX5mNCMVHMZtsCnFeb");
            arrayList.add(node);

            node = new BootstrapNode();
            node.setIpv4("18.216.6.197");
            node.setPort("33445");
            node.setPublicKey("H8sqhRrQuJZ6iLtP2wanxt4LzdNrN2NNFnpPdq1uJ9n2");
            arrayList.add(node);

            node = new BootstrapNode();
            node.setIpv4("52.83.171.135");
            node.setPort("33445");
            node.setPublicKey("5tuHgK1Q4CYf4K5PutsEPK5E3Z7cbtEBdx7LwmdzqXHL");
            arrayList.add(node);

            node = new BootstrapNode();
            node.setIpv4("52.83.191.228");
            node.setPort("33445");
            node.setPublicKey("3khtxZo89SBScAMaHhTvD68pPHiKxgZT6hTCSZZVgNEm");
            arrayList.add(node);

            setBootstrapNodes(arrayList);

            //Hive
            ArrayList<HiveBootstrapNode> hiveArrayList = new ArrayList<>();
            for (String hiveAdd : hiveAddresses){
                HiveBootstrapNode hiveNode = new HiveBootstrapNode();
                hiveNode.setIpv4(hiveAdd);
                hiveNode.setPort(hivePort);
                hiveArrayList.add(hiveNode);
            }
            setHiveBootstrapNodes(hiveArrayList);
        } catch (Exception e){
            e.printStackTrace();
        }
    }
}
