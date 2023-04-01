import 'dart:ffi';
import 'dart:isolate';
import 'dart:convert';
import 'package:flutter/foundation.dart';

//import 'dart::abi';

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
            ..listen((message) {
                _ndiSourceNames = const LineSplitter().convert(ascii.decode(message));
                if (kDebugMode) {
                  print('NDI inputs:$_ndiSourceNames');
                }
             });

        final int nativePort = interactiveCppRequests.sendPort.nativePort;
        final void Function(int port) setDartApiMessagePort = nativeNdiMonitorLib.lookup<NativeFunction<Void Function(Int64)>>('setDartApiMessagePort').asFunction();
        setDartApiMessagePort(nativePort);

        startProgram = nativeNdiMonitorLib.lookup<NativeFunction<Void Function(Uint32)>>('startProgram').asFunction();

        return true;
    }
    static late DynamicLibrary nativeNdiMonitorLib;
    static late Function scanNdiSources;
    static late List<String> _ndiSourceNames;
    static getNdiSourceNames()
    {
        return _ndiSourceNames;
    }

    static late void Function(int programIdx) startProgram;
}


class FFINDKBridge {
    static bool initialize() {
//        ndkLib = (DynamicLibrary.open('libhelloworld-c.so')); // android & linux
//
//        sumUp = ndkLib.lookup<NativeFunction<Int32 Function(Int32, Int32)>>('sumUp').asFunction();

        return true;
    }

    //static late DynamicLibrary ndkLib;
    //static late int Function(int a, int b) sumUp;
    static sumUp(int a, int b)
    {
        return a+b;
    }
}