package org.elastos.wallet.ela.di.component;

import android.app.Activity;
import android.content.Context;

import org.elastos.wallet.ela.di.moudule.FragmentModule;
import org.elastos.wallet.ela.di.scope.ContextLife;
import org.elastos.wallet.ela.di.scope.PerFragment;

import dagger.Component;

/**
 * author: fangxiaogang
 * date: 2018/9/13
 */

@PerFragment
@Component(dependencies = ApplicationComponent.class, modules = FragmentModule.class)
public interface FragmentComponent {
    @ContextLife("Activity")
    Context getActivityContext();

    @ContextLife("Application")
    Context getApplicationContext();

    Activity getActivity();



//    void inject(CreateWalletFragment fragment);

    //void inject(MnemonicWordFragment fragment);

  // void inject(VerifyMnemonicWordsFragment fragment);
   // void inject(OutportMnemonicFragment fragment);


}