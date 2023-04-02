import 'ffi_bridge.dart';

abstract class IProgramControl
{
    void scanPrograms();
    void startProgram(int programIdx);
    List<String> getProgramsName();

    // debug android
    int sumUp(int a, int b);
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

    @override
    int sumUp(int a, int b)
    {
        return a+b;
    }
}