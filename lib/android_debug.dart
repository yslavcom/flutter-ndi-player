import 'dart:ffi';
import 'program_control.dart';

import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

import 'my_app_state.dart';
import 'package:flutter/foundation.dart';

class AndroidDebug extends StatefulWidget {
    @override
    _AndroidDebugState createState() => _AndroidDebugState();
}

class _AndroidDebugState extends State<AndroidDebug> {
    final ProgramControl _programControl = ProgramControl();

    int a = 10;
    int b = -100;

    @override
    Widget build(BuildContext context){
        var appState = context.watch<NdiMonitorState>();
        IconData icon;
        icon = Icons.android;

        var sum = _programControl.sumUp(a, b).toString();

        return Center(
            child: Column(
                mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                children: [
                    Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                            ElevatedButton.icon(
                                onPressed: () {
                                    if (kDebugMode) {
//                                      print("Pressed the button: $onButtonPressCounter");
                                    }
                                    setState(() {
                                      a ++;
                                      b += 10;
                                      sum = _programControl.sumUp(a, b).toString();
                                    });
                                },
                                icon: Icon(icon),
                                label: Text(sum),
                            ),
                        ],)
                ],)
        );
    }
}