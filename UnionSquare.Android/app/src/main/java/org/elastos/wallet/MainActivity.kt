/*
// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet

import android.content.res.Resources
import android.os.Build
import android.os.Bundle
import android.support.v7.app.AppCompatActivity
import android.util.Log
import kotlinx.android.synthetic.main.activity_main.*
import org.elastos.wallet.core.SubWalletCallback
import org.elastos.wallet.core.MasterWalletManager
import org.elastos.wallet.core.SubWallet
import org.elastos.wallet.utils.Utils


class MainActivity : AppCompatActivity() {

    val TAG = MainActivity::class.java.name

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val rootPath = this.filesDir.absolutePath
        Utils.copyAssetsTo(this, rootPath)

        root_path.text = rootPath
        val masterWalletManager = MasterWalletManager(rootPath)

        val language = getLanguage()
        tv_language.text = language

        val mnemonic = masterWalletManager.GenerateMnemonic(language)
        tv_mnemonic.text = mnemonic

        val masterWallets = masterWalletManager.GetAllMasterWallets();
        if (masterWallets.size == 0) {
            var masterWallet = masterWalletManager.CreateMasterWallet("WalletID", mnemonic, "", "payPassword", false);
            var ELASubWallet = masterWallet.CreateSubWallet("ELA", 10000);
            var IDSubWallet = masterWallet.CreateSubWallet("IdChain", 10000);
            ELASubWallet.AddCallback(SubWalletCallback("WalletID", "ELA"));
            IDSubWallet.AddCallback(SubWalletCallback("WalletID", "IdChain"));

            val ELABalance = ELASubWallet.GetBalance(SubWallet.BalanceType.Total);
            val IDBalance = IDSubWallet.GetBalance(SubWallet.BalanceType.Total);
            Log.i(TAG, "balance = " + ELABalance + ", " + IDBalance)
        } else {
            for (masterWallet in masterWallets) {
                val subWallets = masterWallet.GetAllSubWallets();
                for (subWallet in subWallets) {
                    subWallet.AddCallback(SubWalletCallback(masterWallet.GetID(), subWallet.GetChainID()));
                    val balance = subWallet.GetBalance(SubWallet.BalanceType.Total);
                    Log.i(TAG, masterWallet.GetID() + ":" + subWallet.GetChainID() + " balance = " + balance);
                }
            }
        }
    }

    private fun getLanguage(): String {
        val locale = when {
            Build.VERSION.SDK_INT >= Build.VERSION_CODES.N ->
                Resources.getSystem().configuration.locales.get(0)

            else -> Resources.getSystem().configuration.locale
        }

        when (locale.language) {
            "en" -> return getString(R.string.ENGLISH)
            "zh" -> return getString(R.string.CHINESE)
            "fr" -> return getString(R.string.FRENCH)
            "it" -> return getString(R.string.ITALIAN)
            "ja" -> return getString(R.string.JAPANESE)
            "es" -> return getString(R.string.SPANISH)
        }
        return getString(R.string.ENGLISH)
    }


    companion object {

        // Used to load the 'wallet' library on application startup.
        init {
            System.loadLibrary("spvsdk_jni")
        }
    }
}
*/
