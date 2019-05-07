package org.elastos.wallet.ela.rxjavahelp;

import org.json.JSONException;

import io.reactivex.ObservableEmitter;

public interface ObservableListener {
    BaseEntity subscribe() throws JSONException;
}
