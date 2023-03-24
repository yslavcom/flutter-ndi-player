import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'ffi_bridge.dart';
import 'my_app_state.dart';
import 'home_page.dart';

void main() {
//    FFIBridge.initialize();
    runApp(const MainApp());
}

class MainApp extends StatelessWidget {
  const MainApp({super.key});

  @override
  Widget build(BuildContext context) {
    return ChangeNotifierProvider(
      create: (context) => NdiMonitorState(),
      child: MaterialApp(
        title: 'Ndi Monitor',
        theme: ThemeData(
          useMaterial3: true,
          colorScheme: ColorScheme.fromSeed(
              seedColor: const Color.fromRGBO(157, 240, 240, 1)),
        ),
        home: const MyHomePage(),
      ),
    );
  }
}
