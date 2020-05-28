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

package org.elastos.wallet.ela.ui.did.presenter;

import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.net.RetrofitManager;
import org.elastos.wallet.ela.rxjavahelp.NewPresenterAbstract;

import java.util.HashMap;
import java.util.Map;

import io.reactivex.Observable;
import io.reactivex.Observer;

public class AuthorizationPresenter extends NewPresenterAbstract {
    public void postData(String url,String jwt, BaseFragment baseFragment) {
        Map<String,  Object> map = new HashMap();
        map.put("jwt", jwt);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).postData(url,map);
        Observer observer = createObserver(baseFragment, "postData");
        subscriberObservable(observer, observable, baseFragment,5);
    }
    public void jwtSave(String did, String jwt,BaseFragment baseFragment) {
        Map<String, String> map = new HashMap();
        map.put("did", did);
        map.put("jwt", jwt);
        Observable observable = RetrofitManager.getApiService(baseFragment.getContext()).jwtSave(map);
        Observer observer = createObserver(baseFragment, "jwtSave");
        subscriberObservable(observer, observable, baseFragment);
    }
}
