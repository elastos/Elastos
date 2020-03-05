package org.elastos.wallet.ela.DID.util;

import org.elastos.did.DID;
import org.elastos.did.DIDStore;
import org.elastos.did.exception.DIDException;
import org.elastos.did.exception.DIDStoreException;

import java.util.List;

public class DIDUtil {
    public static DID getDid(DIDStore didStore) throws DIDStoreException {
        List<DID> dids = didStore.listDids(DIDStore.DID_ALL);
        return dids.get(0);
    }

    public static String getDidString(DIDStore didStore) {

        try {
            DID did = getDid(didStore);
            return did.toString();
        } catch (DIDException e) {
            e.printStackTrace();
        }
        return null;
    }

}
