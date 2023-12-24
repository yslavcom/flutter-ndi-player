import 'pages.dart';
import 'program_control.dart';
import 'texture_controller.dart';

import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
import 'package:wakelock_plus/wakelock_plus.dart';

import 'dart:io';

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key});
  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  PageIndex selectedPage = PageIndex.home;

  @override
  initState() {
    super.initState();
    SystemChrome.setPreferredOrientations([
      DeviceOrientation.landscapeRight,
      DeviceOrientation.landscapeLeft,
    ]);

    ProgramControl().scanPrograms();
  }

  Widget homePage = HomePage();

  @override
  Widget build(BuildContext context) {
    // To keep the screen on
    WakelockPlus.enable();
    Widget page = homePage;
    switch (selectedPage) {
      case PageIndex.home:
        page = HomePage();
        break;
      default:
        throw UnimplementedError('no widget for $selectedPage');
    }

    if (Platform.isAndroid) {
      return LayoutBuilder(builder: (context, constraints) {
        return Scaffold(
            body: Row(
          children: [
            Expanded(
              child: Container(
                color: Theme.of(context).colorScheme.primaryContainer,
                child: page,
              ),
            ),
          ],
        ));
      });
    } else {
      // Linux code
      return LayoutBuilder(builder: (context, constraints) {
        return Scaffold(
            body: Row(
          children: [
            Expanded(
              child: Container(
                color: Theme.of(context).colorScheme.primaryContainer,
                child: page,
              ),
            ),
          ],
        ));
      });
    }
  }
}

class HomePage extends StatefulWidget {
  @override
  _HomePageState createState() => _HomePageState();
}

class _HomePageState extends State<HomePage> {
  List<String> programNames = [];
  bool isSourceListVisible = true;

  // Texture controller
  double textureOpacity = 0.5;
  final _controller = OpenGLTextureController();
  final _width = 1920;
  final _height = 1080;

  @override
  initState() {
    super.initState();
    ProgramControl().registerListUpdateCallback(updateListView);

    initializeController();
  }

  @override
  Widget build(BuildContext context) {
    var theme = Theme.of(context);

    // Program names list
    programNames = ProgramControl().getProgramsName();
    ProgramControl().scanPrograms();

    // Build
    return Scaffold(
        backgroundColor: theme.colorScheme.background,
        body: Stack(children: [
          // The widget that appears later in the list will be drawn on top of the widgets that come before it
          visibleWidget(true,
              Opacity(opacity: textureOpacity, child: textureContainer())),
          visibleWidget(isSourceListVisible,
              Opacity(opacity: 1.0, child: sourceListContainer(theme))),
          GestureDetector(
            onDoubleTap: () {
              setState(() {
                toggleSourceListVisibility();
                adjustTextureTransparency(isSourceListVisible ? 0.5 : 1.0);
              });
            },
          )
        ]));
  }

  // Callback function to update the ListView
  void updateListView(List<String> newData) {
    setState(() {
      programNames = newData;
      if (kDebugMode) {
        print('NDI inputs list:$programNames');
      }
    });
  }

  Future<void> initializeController() async {
    await _controller.initialize(_width, _height);
    setState(() {});
  }

  Widget visibleWidget(bool isVisible, Widget wid) {
    return Visibility(
      visible: isVisible,
      maintainSize: true,
      maintainAnimation: true,
      maintainState: true,
      child: wid,
    );
  }

  Widget textureContainer() {
    return Center(
      child: SizedBox(
        width: _width.toDouble(),
        height: _height.toDouble(),
        child: _controller.isInitialized
            ? Texture(textureId: _controller.textureId)
            : null,
      ),
    );
  }

  Widget sourceListContainer(ThemeData theme) {
    return ListView.separated(
      itemCount: programNames.length,
      separatorBuilder: (BuildContext context, int index) => Divider(
        height: 1,
        color: theme.colorScheme.surfaceVariant,
        thickness: 1,
      ),
      itemBuilder: (BuildContext context, int index) => ListTile(
        textColor: theme.colorScheme.primary,
        title: Text(programNames[index]),
        onLongPress: () {
          if (kDebugMode) {
            print('onLongPress');
          }
          displaySelectedSourceInfo();
        },
        onTap: () {
          ProgramControl().startProgram(index);
          if (kDebugMode) {
            print('onTap');
          }
          setState(() {
            setSourceListVisibility(false);
            adjustTextureTransparency(isSourceListVisible ? 0.5 : 1.0);
          });
        },
      ),
    );
  }

  void setSourceListVisibility(bool isVisible) {
    isSourceListVisible = isVisible;
  }

  void toggleSourceListVisibility() {
    isSourceListVisible = !isSourceListVisible;
  }

  void adjustTextureTransparency(double opacity) {
    textureOpacity = opacity;
  }

  void displaySelectedSourceInfo() {
    setState(() {});
  }
}
