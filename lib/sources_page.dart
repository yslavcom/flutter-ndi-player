import 'dart:ffi';
import 'program_control.dart';

import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'my_app_state.dart';

class SourcesPage extends StatelessWidget {
  SourcesPage({super.key});

    final ProgramControl programControl = ProgramControl();

    @override
    Widget build(BuildContext context){

        var theme = Theme.of(context);
        var appState = context.watch<NdiMonitorState>();
        IconData icon;
        icon = Icons.search;

        var programNames = programControl.getProgramsName();

      return Scaffold(
        appBar: AppBar(
          title: Text('Inputs'),
          backgroundColor: theme.colorScheme.surfaceTint,
          foregroundColor: Colors.white,
          ),
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
                  programControl.startProgram(index);
              },
              onTap: (){
                  programControl.startProgram(index);
              },
            ),
        ),
      );
    }
}