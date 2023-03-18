import 'ffi_bridge.dart';

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
}