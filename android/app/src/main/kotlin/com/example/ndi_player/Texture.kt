package com.example.ndi_player

import android.graphics.SurfaceTexture
import android.view.Surface
import androidx.annotation.NonNull
import android.util.Log

import io.flutter.embedding.engine.plugins.FlutterPlugin
import io.flutter.embedding.android.FlutterActivity
import io.flutter.embedding.engine.FlutterEngine

import io.flutter.plugin.common.MethodCall
import io.flutter.plugin.common.MethodChannel
import io.flutter.plugin.common.MethodChannel.MethodCallHandler
import io.flutter.plugin.common.MethodChannel.Result

class TextureHelper
{
    init{}
    external fun setTextureSize(width: Int, height: Int)
    external fun setTextureCb(surface: Surface)
    external fun disposeTexture()
}

class Texture: MethodCallHandler, FlutterPlugin {
    private lateinit var channel: MethodChannel
    private lateinit var mFlutterPluginBinding: FlutterPlugin.FlutterPluginBinding
    private lateinit var mSurfaceTexture: SurfaceTexture

    init {
    }

    companion object {
        private lateinit var mSurface: Surface
        val mTextureHelper : TextureHelper;
        init {
            System.loadLibrary("ndi-monitor")
            mTextureHelper = TextureHelper()
        }

        @JvmStatic
        fun onRequestTex() {
            // Handle the callback data
        //    Log.w("onRequestTex:" + mSurface.hashCode())
            mTextureHelper.setTextureCb(mSurface)
        }
    }

    override fun onAttachedToEngine(@NonNull flutterPluginBinding: FlutterPlugin.FlutterPluginBinding) {
        channel = MethodChannel(flutterPluginBinding.binaryMessenger, "opengl_texture")
        channel.setMethodCallHandler(this)
        mFlutterPluginBinding = flutterPluginBinding
    }

   override fun onDetachedFromEngine(@NonNull binding: FlutterPlugin.FlutterPluginBinding) {
        channel.setMethodCallHandler(null)
    }

    override fun onMethodCall(@NonNull call: MethodCall, @NonNull result: Result) {
        if (call.method == "create") {
            val width = call.argument<Int>("width")
            val height = call.argument<Int>("height")
            if (width != null && height != null)
            {
                mTextureHelper.setTextureSize(width, height)

                val textureId = generateSurfaceTextureId(width, height)
                result.success(textureId)
            }
            else
            {
                result.notImplemented()
            }
        }
        else if (call.method == "dispose") {
            val textureId = call.argument<Int>("textureId")
            mTextureHelper.disposeTexture()
            // do something here
        } else {
            result.notImplemented()
        }
    }

    private fun generateSurfaceTextureId(surfaceWidth: Int, surfaceHeight: Int): Long {
        val textureRegistry = mFlutterPluginBinding.textureRegistry
        val surfaceTextureEntry = textureRegistry.createSurfaceTexture()
        mSurfaceTexture = surfaceTextureEntry.surfaceTexture()
        mSurfaceTexture.setDefaultBufferSize(surfaceWidth, surfaceHeight)

        mSurface = Surface(mSurfaceTexture)

//        val canvas = mSurface.lockCanvas(null)
//        canvas.drawRGB(255, 230, 15)
//        mSurface.unlockCanvasAndPost(canvas)

//        mTextureHelper.setTexture(mSurface)

        return surfaceTextureEntry.id()
    }
}
