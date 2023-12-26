import 'dart:ffi';
import 'dart:isolate';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:nsd/nsd.dart';

typedef UpdateListCallback = void Function(List<String>);

class FFIBridge {
    static bool initialize() {
        nativeNdiMonitorLib = (DynamicLibrary.open('libndi-monitor.so')); // android & linux

        final scanNdiSources_ = nativeNdiMonitorLib.lookup<NativeFunction<Int32 Function()>>('scanNdiSources');
        scanNdiSources = scanNdiSources_.asFunction<int Function()>();

        final getOverflowCount_ = nativeNdiMonitorLib.lookup<NativeFunction<Uint32 Function(Int32)>>('getOverflowCount');
        getOverflowCount = getOverflowCount_.asFunction<int Function(int)>();

        final getRxQueueLen_ = nativeNdiMonitorLib.lookup<NativeFunction<Uint32 Function(Int32)>>('getRxQueueLen');
        getRxQueueLen = getRxQueueLen_.asFunction<int Function(int)>();

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
    static late DynamicLibrary nativeNdiMonitorLib;
    static late Function scanNdiSources;
    static late Function getOverflowCount;
    static late Function getRxQueueLen;

    static List<String> _ndiSourceNames = [];
    static getNdiSourceNames()
    {
        return _ndiSourceNames;
    }

    static late void Function(int programIdx) startProgram;

    static void registerListUpdateCallback(Function(List<String>) callback)
    {
        updateListCallback = callback;

        if (kDebugMode) {
          print('registerListUpdateCallback:$callback');
        }
    }
    static UpdateListCallback? updateListCallback;
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
          print("!!! discovery error:$e");
        }
    }
  }
}
