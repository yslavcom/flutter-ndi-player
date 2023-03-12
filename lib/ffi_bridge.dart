import 'dart:ffi';
import 'dart:isolate';
import 'package:ffi/ffi.dart';

class FFIBridge {
    static bool initialize() {
        nativeNdiMonitorLib = (DynamicLibrary.open('libndi-monitor.so')); // android & linux
        final scanNdiSources_ = nativeNdiMonitorLib.lookup<NativeFunction<Int32 Function()>>('scanNdiSources');
        scanNdiSources = scanNdiSources_.asFunction<int Function()>();

        // initialize the native dart API
        final initializeApi = nativeNdiMonitorLib.lookupFunction<IntPtr Function(Pointer<Void>),
            int Function(Pointer<Void>)>("initializeApiDLData");
        var initRet = initializeApi(NativeApi.initializeApiDLData);
        if (initRet != 0) {
          throw "Failed to initialize Dart API: $initRet";
        }

        final interactiveCppRequests = ReceivePort()
            ..listen((message) { });

        final int nativePort = interactiveCppRequests.sendPort.nativePort;
        final void Function(int port) setDartApiMessagePort = nativeNdiMonitorLib.lookup<NativeFunction<Void Function(Int64)>>('setDartApiMessagePort').asFunction();
        setDartApiMessagePort(nativePort);

        return true;
    }
    static late DynamicLibrary nativeNdiMonitorLib;
    static late Function scanNdiSources;
}
