import 'pages.dart';
import 'program_control.dart';
import 'texture_controller.dart';

import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';
// import 'package:wakelock_plus/wakelock_plus.dart';
// import 'package:ffmpeg_kit_flutter/ffmpeg_kit.dart';

import 'dart:io';
import 'dart:async';

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

    ProgramControl().scanNdiSources();
  }

  Widget homePage = HomePage();

  @override
  Widget build(BuildContext context) {
    // To keep the screen on
//    WakelockPlus.enable();
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
  bool isDebugInfoVisible = kDebugMode;
  var _debugInfo = DebugInfo();
  late Timer _timer;

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

    if (isDebugInfoVisible) {
      _timer = Timer.periodic(Duration(seconds: 1), (Timer timer) {
        setState(() {
          _debugInfo.add('VidOverflow:', ProgramControl().getOverflowCount(0).toString());
          _debugInfo.add('AudOverflow:', ProgramControl().getOverflowCount(1).toString());

          _debugInfo.add('VidQueue:', ProgramControl().getRxQueueLen(0).toString());
          _debugInfo.add('AudQueue:', ProgramControl().getRxQueueLen(1).toString());

          var list = ProgramControl().getRxFrameCount();
          _debugInfo.add('Rx Vid:/Aud:', '${list[0]} / ${list[1]}');
        });
      });
    }
  }

  @override
  void dispose() {
    if (isDebugInfoVisible) {
    _timer.cancel();
    }

    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    var theme = Theme.of(context);

    // Program names list
    programNames = ProgramControl().getProgramsName();

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
          ),
          if (isDebugInfoVisible) debugInfo(theme),
        ]));
  }

  // Callback function to update the ListView
  void updateListView(List<String> newData) {
    setState(() {
      programNames = newData;
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
          displaySelectedSourceInfo();
        },
        onTap: () {
          ProgramControl().startProgram(index);
          setState(() {
            setSourceListVisibility(false);
            adjustTextureTransparency(isSourceListVisible ? 0.5 : 1.0);
          });
        },
      ),
    );
  }

  Widget debugInfo(ThemeData theme) {
    var list = _debugInfo.getInfoList();

    return Align(
        alignment: Alignment.bottomRight,
        child: Visibility(
            visible: true,
            child: Opacity(
                opacity: 1.0,
                child: Container(
                    alignment: Alignment.bottomRight,
                    width: 200,
                    child: ListView.separated(
                        itemCount: _debugInfo.len(),
                        separatorBuilder: (BuildContext context, int index) => Divider(
                            height: 1,
                            color: theme.colorScheme.surfaceVariant,
                            thickness: 1,
                          ),
                        itemBuilder: (BuildContext context, int index) {
                          return ListTile(
                            title: Text(list[index],
                                textAlign: TextAlign.left,
                                overflow: TextOverflow.visible,
                                softWrap: true,
                                style: const TextStyle(
                                    fontSize: 14.0, color: Colors.red, fontWeight: FontWeight.bold)),
                          );
                        })))));
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

class DebugInfo {
  Map<String, int> _entriesMap = {};
  List<String> _info = [];

  DebugInfo() {}

  void add(String key, String data) {
    if (_entriesMap.containsKey(key)) {
      var idx = _entriesMap[key];
      _info[idx!] = key + data;
    } else {
      _entriesMap[key] = _info.length;
      _info.add(key + data);
    }
  }

  List<String> getInfoList() {
    return _info;
  }

  int len() {
    return _info.length;
  }
}
