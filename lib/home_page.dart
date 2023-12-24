import 'pages.dart';
import 'program_control.dart';
import 'texture_controller.dart';

import 'package:flutter/material.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

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
            SafeArea(
                child: NavigationRail(
                    extended: constraints.maxWidth >= 600,
                    destinations: const [
                      NavigationRailDestination(
                        icon: Icon(Icons.home),
                        label: Text('Home'),
                      ),
                    ],
                    selectedIndex: selectedPage.index,
                    onDestinationSelected: (value) {
                      setState(() {
                        selectedPage = PageIndex.values[value];
                      });
                    })),
            // Display the page on the right handside
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
            SafeArea(
                child: NavigationRail(
                    extended: constraints.maxWidth >= 600,
                    destinations: const [
                      NavigationRailDestination(
                        icon: Icon(Icons.home),
                        label: Text('Home'),
                      ),
                    ],
                    selectedIndex: selectedPage.index,
                    onDestinationSelected: (value) {
                      setState(() {
                        selectedPage = PageIndex.values[value];
                      });
                    })),
            // Display the page on the right handside
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

  // Texture controller
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
          Opacity(opacity: 0.5, child: TextureContainer()),
          Opacity(opacity: 1.0, child: SourceListContainer(theme)),
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

  Widget TextureContainer() {
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

  Widget SourceListContainer(ThemeData theme) {
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
          ProgramControl().startProgram(index);
        },
        onTap: () {
          ProgramControl().startProgram(index);
        },
      ),
    );
  }
}
