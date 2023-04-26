package com.example.ndi_player

import java.nio.ByteBuffer
import android.util.Log

class RenderHelper
{
    external fun cleanup(ptr: Int)
}

class Render
{

    companion object {
        val mRenderHelper : RenderHelper;
        init {
            System.loadLibrary("ndi-monitor")
            mRenderHelper = RenderHelper();
        }

        @JvmStatic
        fun onCallback(data: ByteBuffer) {
            // Handle the callback data
//            Log.w("Render", "got frame:" + data.remaining())

            mRenderHelper.cleanup(0x55)
        }
    }
}