import 'dart:ffi';
import 'package:ffi/ffi.dart';

import 'dart:isolate';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:nsd/nsd.dart';

typedef UpdateListCallback = void Function(List<String>);

typedef getArray_func = Pointer<Void> Function(Int32 type, Pointer<Int32> dataSize);
typedef GetArray = Pointer<Void> Function(int type, Pointer<Int32> dataSize);

typedef getArrayArgs_func = Pointer<Void> Function(Pointer<Void>args, Pointer<Int32> dataSize);
typedef GetArrayArgs = Pointer<Void> Function(Pointer<Void>args, Pointer<Int32> dataSize);

typedef freeArray_func = Void Function(Pointer<Void>);
typedef FreeArray = void Function(Pointer<Void> addr);

class FFIBridge {
    static bool initialize() {
        nativeNdiMonitorLib = (DynamicLibrary.open('libndi-monitor.so')); // android & linux

        final scanNdiSources_ = nativeNdiMonitorLib.lookup<NativeFunction<Int32 Function()>>('scanNdiSources');
        scanNdiSources = scanNdiSources_.asFunction<int Function()>();

        final getOverflowCount_ = nativeNdiMonitorLib.lookup<NativeFunction<Uint32 Function(Int32)>>('getOverflowCount');
        getOverflowCount = getOverflowCount_.asFunction<int Function(int)>();

        final getRxQueueLen_ = nativeNdiMonitorLib.lookup<NativeFunction<Uint32 Function(Int32)>>('getRxQueueLen');
        getRxQueueLen = getRxQueueLen_.asFunction<int Function(int)>();

        final getArrayPointer = nativeNdiMonitorLib.lookup<NativeFunction<getArray_func>>('getArray');
        getArray = getArrayPointer.asFunction<GetArray>();

        final freeArray_ = nativeNdiMonitorLib.lookup<NativeFunction<freeArray_func>>('freeArray');
        freeArray = freeArray_.asFunction<FreeArray>();

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
                  print('Native message inputs list:$_ndiSourceNames');
                }

                if (updateListCallback != null)
                {
                  updateListCallback!(_ndiSourceNames);
                }
             });

        final int nativePort = interactiveCppRequests.sendPort.nativePort;
        final void Function(int port) setDartApiMessagePort = nativeNdiMonitorLib.lookup<NativeFunction<Void Function(Int64)>>('setDartApiMessagePort').asFunction();
        setDartApiMessagePort(nativePort);

        startProgram = nativeNdiMonitorLib.lookup<NativeFunction<Void Function(Uint32)>>('startProgram').asFunction();

        return true;
    }

    static getNdiSourceNames()
    {
        return _ndiSourceNames;
    }

    static void registerListUpdateCallback(Function(List<String>) callback)
    {
        updateListCallback = callback;

        if (kDebugMode) {
          print('registerListUpdateCallback:$callback');
        }
    }

    static List<String> _ndiSourceNames = [];
    static UpdateListCallback? updateListCallback;

    static late DynamicLibrary nativeNdiMonitorLib;
    static late void Function(int programIdx) startProgram;
    static late Function scanNdiSources;
    static late Function getOverflowCount;
    static late Function getRxQueueLen;
    static late Function getArray;
    static late Function freeArray;
}

class Nds {
  static init() async {

    final discovery = await startDiscovery('_services._dns-sd._udp', autoResolve: false);
    try
    {
      discovery.addListener(() {
        if (kDebugMode)
        {
          var services = discovery.services;
          print("Discovered:$services");
        }
       });
    }
    catch (e)
    {
      if (kDebugMode)
        {
          print("Discovery error:$e");
        }
    }
  }
}
