import 'texture_controller.dart';
import 'package:flutter/material.dart';

class PlayerTex extends StatefulWidget {
  const PlayerTex({super.key});

  @override
  _PlayerTexState createState() => _PlayerTexState();
}

class _PlayerTexState extends State<PlayerTex> {
  final _controller = OpenGLTextureController();
  final _width = 200;
  final _height = 200;

  @override
  initState() {
    super.initState();

    initializeController();
  }

  @override
  void dispose() {
    _controller.dispose();

    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('OpenGL via Texture widget example'),
        ),
        body: Center(
          child: SizedBox(
            width: _width.toDouble(),
            height: _height.toDouble(),
            child: _controller.isInitialized
                ? Texture(textureId: _controller.textureId)
                : null,
          ),
        ),
      ),
    );
  }

  Future<void> initializeController() async {
    await _controller.initialize(_width, _height);
    setState(() {});
  }
}
