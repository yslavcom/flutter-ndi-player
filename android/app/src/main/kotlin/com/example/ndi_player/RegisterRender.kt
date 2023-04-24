package com.example.ndi_player

import java.nio.ByteBuffer

interface RenderCallback {
    fun onCallback(data: ByteBuffer)
}

class RegisterRender
{
    external fun registerCallback(callback: RenderCallback)

    companion object {
        init {
            System.loadLibrary("libndi-monitor")
        }
    }
}

//val render = RegisterRender();
//
//val myCallback = object : RenderCallback {
//    override fun onCallback(data: ByteBuffer) {
//        // Handle the callback data
//    }
//}
//render.registerCallback(myCallback);