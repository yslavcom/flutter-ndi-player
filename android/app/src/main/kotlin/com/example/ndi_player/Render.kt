package com.example.ndi_player

import java.nio.ByteBuffer
import android.util.Log

class Render
{
    companion object {
        init {
            System.loadLibrary("ndi-monitor")
        }

        @JvmStatic
        fun onCallback(data: ByteBuffer) {
            // Handle the callback data
            Log.w("Render", "got frame")
        }
    }
}