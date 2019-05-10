package org.elastos.wallet.ela.di.moudule;

import android.content.Context;

import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.di.scope.ContextLife;
import org.elastos.wallet.ela.di.scope.PerApp;

import dagger.Module;
import dagger.Provides;


/**
 * Created by lw on 2017/1/19.
 */
@Module
public class ApplicationModule {
    private MyApplication mApplication;

    public ApplicationModule(MyApplication application) {
        mApplication = application;
    }

    @Provides
    @PerApp
    @ContextLife("Application")
    public Context provideApplicationContext() {
        return mApplication.getApplicationContext();
    }
}
