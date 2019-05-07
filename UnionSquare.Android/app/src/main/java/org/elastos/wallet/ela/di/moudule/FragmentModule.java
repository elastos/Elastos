package org.elastos.wallet.ela.di.moudule;

import android.app.Activity;
import android.content.Context;
import android.support.v4.app.Fragment;

import org.elastos.wallet.ela.di.scope.ContextLife;
import org.elastos.wallet.ela.di.scope.PerFragment;

import dagger.Module;
import dagger.Provides;

/**
 * Created by lw on 2017/1/19.
 */
@Module
public class FragmentModule {
    private Fragment mFragment;

    public FragmentModule(Fragment fragment) {
        mFragment = fragment;
    }

    @Provides
    @PerFragment
    @ContextLife("Activity")
    public Context provideActivityContext() {
        return mFragment.getActivity();
    }

    @Provides
    @PerFragment
    public Activity provideActivity() {
        return mFragment.getActivity();
    }

    @Provides
    @PerFragment
    public Fragment provideFragment() {
        return mFragment;
    }
}
