// import 'package:flutter/cupertino.dart';
import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'pages.dart';
import 'my_app_state.dart';
import 'sources_page.dart';

class MyHomePage extends StatefulWidget {
    const MyHomePage({super.key});
    @override
    State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
    PageIndex selectedPage = PageIndex.home;

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
                // page = PlayerPage();
                throw UnimplementedError('no widget for $selectedPage');
            default:
                throw UnimplementedError('no widget for $selectedPage');
        }

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
}

class HomePage extends StatelessWidget {
    @override
    Widget build(BuildContext context){
        var appState = context.watch<NdiMonitorState>();
        IconData icon;
        icon = Icons.search;

        return Center(
            child: Column(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                    Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                            ElevatedButton.icon(
                                onPressed: () {
                                    print("Pressed the button");
                                },
                                icon: Icon(icon),
                                label: const Text('Search NDI sources'),
                            ),
                        ],)
                ],)
        );
    }
}