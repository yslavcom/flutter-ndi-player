// import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'pages.dart';
import 'my_app_state.dart';
import 'sources_page.dart';
import 'program_control.dart';
import 'player_texture.dart';

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

    @override
    Widget build(BuildContext context) {
        Widget page;
        switch(selectedPage) {
            case PageIndex.home:
                page = HomePage();
                break;
            case PageIndex.sources:
                page = SourcesPage();
                break;
            case PageIndex.player:
                page = const PlayerTex();
                break;
            default:
                throw UnimplementedError('no widget for $selectedPage');
        }

        if (Platform.isAndroid)
        {
          return LayoutBuilder(
              builder: (context, constraints) {
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
                                          NavigationRailDestination(
                                              icon: Icon(Icons.list),
                                              label: Text('Sources'),
                                          ),
                                          NavigationRailDestination(
                                              icon: Icon(Icons.abc),
                                              label: Text('Player'),
                                          ),
                                      ],
                                      selectedIndex: selectedPage.index,
                                      onDestinationSelected: (value) {
                                          setState(() {
                                            selectedPage = PageIndex.values[value];
                                          });
                                      }
                                      )
                              ),
                              // Display the page on the right handside
                              Expanded(
                                      child: Container(
                                      color: Theme.of(context).colorScheme.primaryContainer,
                                      child: page,
                                  ),
                              ),
                          ],
                      )
                  );
              }
          );
        }
        else
        {
          // Linux code
          return LayoutBuilder(
              builder: (context, constraints) {
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
                                          NavigationRailDestination(
                                              icon: Icon(Icons.list),
                                              label: Text('Sources'),
                                          ),
                                      ],
                                      selectedIndex: selectedPage.index,
                                      onDestinationSelected: (value) {
                                          setState(() {
                                            selectedPage = PageIndex.values[value];
                                          });
                                      }
                                      )
                              ),
                              // Display the page on the right handside
                              Expanded(
                                      child: Container(
                                      color: Theme.of(context).colorScheme.primaryContainer,
                                      child: page,
                                  ),
                              ),
                          ],
                      )
                  );
              }
          );
        }
    }
}

class HomePage extends StatelessWidget {

  var onButtonPressCounter = 0;

  HomePage({super.key});

  @override
  Widget build(BuildContext context){
      ProgramControl().scanPrograms();
      return Center(
          child: Column()
      );
  }
}