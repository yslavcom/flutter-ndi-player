import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'my_app_state.dart';

class SourcesPage extends StatelessWidget {
  const SourcesPage({super.key});

    @override
    Widget build(BuildContext context){
        var appState = context.watch<NdiMonitorState>();
        IconData icon;
        icon = Icons.search;

        var textList = <String>[];
        for (var i = 0; i < 100; i ++)
        {
            textList.add('Text+$i');
        }


        return ListView(
            children: <Widget>[
                for (var el in textList)
                    ListTile(
                        title: Text(el),
                    ),
            ],
        );
    }
}