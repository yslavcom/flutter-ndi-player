// import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

import 'pages.dart';
import 'my_app_state.dart';
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

  HomePage({super.key});

  @override
  Widget build(BuildContext context){
    var theme = Theme.of(context);
    var programNames = ProgramControl().getProgramsName();

    ProgramControl().scanPrograms();
    return Scaffold(
        backgroundColor: theme.colorScheme.background,
        body: ListView.separated(
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
              onTap: (){
                  ProgramControl().startProgram(index);
              },
            ),
        ),
      );
  }
}