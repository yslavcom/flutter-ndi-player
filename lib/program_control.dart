/* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-12-24 08:48:08  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-12-24 08:48:08 . All rights reserved *//* * (C) Copyright   @Author: iaroslav.dochien  * @Date: 2023-12-24 08:48:02  * @Last Modified by:   iaroslav.dochien  * @Last Modified time: 2023-12-24 08:48:02 . All rights reserved */
import 'ffi_bridge.dart';
import 'package:flutter/foundation.dart';

abstract class IProgramControl
{
    void scanPrograms();
    void startProgram(int programIdx);
    List<String> getProgramsName();
}

class ProgramControl extends IProgramControl {
    @override
    void scanPrograms()
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
}
