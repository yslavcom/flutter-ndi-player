import 'dart:ffi';
import 'ffi_bridge.dart';

import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'my_app_state.dart';

class SourcesPage extends StatelessWidget {
  const SourcesPage({super.key});

    @override
    Widget build(BuildContext context){

        var theme = Theme.of(context);
        var appState = context.watch<NdiMonitorState>();
        IconData icon;
        icon = Icons.search;

        List<String> textList = FFIBridge.getNdiSourceNames();

      return Scaffold(
        appBar: AppBar(
          title: Text('Inputs'),
          backgroundColor: theme.colorScheme.surfaceTint,
          foregroundColor: Colors.white,
          ),
        backgroundColor: theme.colorScheme.background,
        body: ListView.separated(
            itemCount: textList.length,
            separatorBuilder: (BuildContext context, int index) => Divider(
              height: 1,
              color: theme.colorScheme.surfaceVariant,
              thickness: 1,
            ),
            itemBuilder: (BuildContext context, int index) => ListTile(
              textColor: theme.colorScheme.primary,
              title: Text(textList[index]),
              onLongPress: () {
                  FFIBridge.startProgram(index);
              },
              onTap: (){
                  FFIBridge.startProgram(index);
              },
            ),
        ),
      );
    }
}