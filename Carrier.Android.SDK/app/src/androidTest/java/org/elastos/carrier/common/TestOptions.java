package org.elastos.carrier.common;

import java.io.File;
import java.util.ArrayList;

import org.elastos.carrier.Carrier.Options;
import org.elastos.carrier.Log;

public class TestOptions extends Options {
    public TestOptions(String path) {
        super();

        File file = new File(path);
        if (file.exists())
            file.delete();
        file.mkdir();

        try {
            setUdpEnabled(true);
            setPersistentLocation(path);

            ArrayList<BootstrapNode> bootstraps = new ArrayList<>();
            BootstrapNode node = new BootstrapNode();
            node.setIpv4("13.58.208.50");
            node.setPort("33445");
            node.setPublicKey("89vny8MrKdDKs7Uta9RdVmspPjnRMdwMmaiEW27pZ7gh");
            bootstraps.add(node);

            node = new BootstrapNode();
            node.setIpv4("18.216.102.47");
            node.setPort("33445");
            node.setPublicKey("G5z8MqiNDFTadFUPfMdYsYtkUDbX5mNCMVHMZtsCnFeb");
            bootstraps.add(node);

            node = new BootstrapNode();
            node.setIpv4("18.216.6.197");
            node.setPort("33445");
            node.setPublicKey("H8sqhRrQuJZ6iLtP2wanxt4LzdNrN2NNFnpPdq1uJ9n2");
            bootstraps.add(node);

            node = new BootstrapNode();
            node.setIpv4("52.83.171.135");
            node.setPort("33445");
            node.setPublicKey("5tuHgK1Q4CYf4K5PutsEPK5E3Z7cbtEBdx7LwmdzqXHL");
            bootstraps.add(node);

            node = new BootstrapNode();
            node.setIpv4("52.83.191.228");
            node.setPort("33445");
            node.setPublicKey("3khtxZo89SBScAMaHhTvD68pPHiKxgZT6hTCSZZVgNEm");
            bootstraps.add(node);

            setBootstrapNodes(bootstraps);

            ArrayList<ExpressNode> expressNodes = new ArrayList<>();
            ExpressNode enode = new ExpressNode();
            enode.setIpv4("ece00.trinity-tech.io");
            enode.setPort("443");
            enode.setPublicKey("FyTt6cgnoN1eAMfmTRJCaX2UoN6ojAgCimQEbv1bruy9");
            expressNodes.add(enode);

            enode = new ExpressNode();
            enode.setIpv4("ece01.trinity-tech.io");
            enode.setPort("443");
            enode.setPublicKey("FyTt6cgnoN1eAMfmTRJCaX2UoN6ojAgCimQEbv1bruy9");
            expressNodes.add(enode);

            enode = new ExpressNode();
            enode.setIpv4("ece01.trinity-tech.cn");
            enode.setPort("443");
            enode.setPublicKey("FyTt6cgnoN1eAMfmTRJCaX2UoN6ojAgCimQEbv1bruy9");
            expressNodes.add(enode);

            setExpressNodes(expressNodes);

        } catch (Exception e){
            e.printStackTrace();
        }
    }
}
