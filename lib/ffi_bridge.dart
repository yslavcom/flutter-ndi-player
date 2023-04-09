import 'dart:ffi';
import 'dart:isolate';
import 'dart:convert';
import 'package:flutter/foundation.dart';
import 'package:nsd/nsd.dart';

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

class Nds {
  static init() async {

    if (kDebugMode)
    {
      print("!!! Nds::init");
    }


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

//    discovery.then((value) => handleDiscovery(value)).catchError(handleDiscoveryError);
  }

//  static handleDiscovery(value){
//    if (kDebugMode)
//    {
//      print('handleDiscovery value:$value');
//    }
//  }
//
//  static handleDiscoveryError(e){
//    if (kDebugMode)
//    {
//      print('handleDiscoveryError value:$e');
//    }
//  }
}
