package org.elastos.wallet.utils

import android.content.Context
import java.io.File
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream

internal object Utils {
    val TAG = Utils::class.java.name

    fun copyAssetsTo(context: Context, target: String) {
        val assets = context.assets.list("")
        if (assets.isNullOrEmpty()) {
            return
        }
        for (asset in assets) {
            copyAssetTo(context, asset, target)
        }
    }

    fun copyAssetTo(context: Context, asset: String, target: String) {
        var input: InputStream? = null
        var output: OutputStream? = null
        try {
            val file = File("$target/$asset")
            if (!asset.contains(".") || file.exists()) {
                return
            }
            input = context.assets.open(asset)
            output = file.outputStream()
            val buf = ByteArray(1024)
            var len = input.read(buf)
            while (len > 0) {
                output.write(buf, 0, len)
                len = input.read(buf)
            }
            input.close()
            output.close()

        } catch (e: IOException) {
            input?.close()
            output?.close()
            e.printStackTrace()
        }
    }
}
