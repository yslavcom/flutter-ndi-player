import 'package:flutter/material.dart';

import 'player.dart';

class PlayerWindow extends StatefulWidget {
  const PlayerWindow({super.key});

    @override
    _PlayerWindowState createState() => _PlayerWindowState();
}

class _PlayerWindowState extends State<PlayerWindow> {
    @override
    void initState() {
        super.initState();
    }

    @override
    Widget build(BuildContext context) {
        return Player();
    }
}