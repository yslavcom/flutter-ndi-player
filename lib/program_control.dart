/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-12-24 08:48:08  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-12-24 08:48:08 . All rights reserved *//* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-12-24 08:48:02  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-12-24 08:48:02 . All rights reserved */
import 'dart:ffi';

import 'ffi_bridge.dart';
import 'package:ffi/ffi.dart';
import 'package:flutter/foundation.dart';

abstract class IProgramControl
{
    void scanNdiSources();
    void startProgram(int programIdx);
    List<String> getProgramsName();
    int getOverflowCount(int type);
    int getRxQueueLen(int type);
    List<int> getRxFrameCount();
}

class ProgramControl extends IProgramControl {
    @override
    void scanNdiSources()
    {
        FFIBridge.scanNdiSources();
    }

    @override
    void startProgram(int programIdx)
    {
        FFIBridge.startProgram(programIdx);
    }

    @override
    List<String> getProgramsName()
    {
        return FFIBridge.getNdiSourceNames();
    }

    void registerListUpdateCallback(Function(List<String>) callback)
    {
        if (kDebugMode) {
          print('registerListUpdateCallback: $callback');
        }
        FFIBridge.registerListUpdateCallback(callback);
    }

    @override
    int getOverflowCount(int type)
    {
      return FFIBridge.getOverflowCount(type);
    }

    @override
    int getRxQueueLen(int type)
    {
      return FFIBridge.getRxQueueLen(type);
    }

    List<int> getArray_(int type)
    {
      // Create a pointer to store the arr_size value
      Pointer<Int32> arrSizePtr = calloc<Int32>();
      arrSizePtr.value = 0;

      var arrayPointer = FFIBridge.getArray(type, arrSizePtr);
      var arrReal = arrayPointer as Pointer<Int32>;
      var size = arrSizePtr.value;

      calloc.free(arrSizePtr);

      var list = List.generate(size, (int index) => arrReal.elementAt(index).value, growable: true);
      FFIBridge.freeArray(arrayPointer);
      return list;
    }

    @override
    List<int> getRxFrameCount()
    {
      return getArray_(0);
    }
}
