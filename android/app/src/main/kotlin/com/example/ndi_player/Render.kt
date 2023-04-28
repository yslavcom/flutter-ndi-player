package com.example.ndi_player

import java.nio.ByteBuffer
import android.util.Log

class RenderHelper
{
    external fun cleanup(ptr: Long)
}

class Render
{
    companion object {
        val mRenderHelper : RenderHelper;
        var mPtr : Long = 0;
        init {
            System.loadLibrary("ndi-monitor")
            mRenderHelper = RenderHelper();
        }

        @JvmStatic
        fun onCallback(data: ByteBuffer, ptr: Long) {
            // Handle the callback data
            Log.w("Render", "got frame:" + data.remaining())
            mPtr = ptr;

            mRenderHelper.cleanup(mPtr)
        }
    }
}