import 'dart:ffi';
import 'package:ffi/ffi.dart';

class FFIBridge {
    static bool initialize() {
        nativeNdiMonitorLib = (DynamicLibrary.open('libndi-monitor.so')); // android & linux
        final scanNdiSources_ = nativeNdiMonitorLib.lookup<NativeFunction<Int32 Function()>>('scanNdiSources');
        scanNdiSources = scanNdiSources_.asFunction<int Function()>();

        return true;
    }
    static late DynamicLibrary nativeNdiMonitorLib;
    static late Function scanNdiSources;
}