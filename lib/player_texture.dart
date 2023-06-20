import 'texture_controller.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class PlayerTex extends StatefulWidget {
  const PlayerTex({super.key});

  @override
  _PlayerTexState createState() => _PlayerTexState();
}

class _PlayerTexState extends State<PlayerTex> {
  final _controller = OpenGLTextureController();
  // final _width = 284;
  // final _height = 160;
  final _width = 1920;
  final _height = 1080;


  @override
  initState() {
    super.initState();
    SystemChrome.setPreferredOrientations([
      DeviceOrientation.landscapeRight,
      DeviceOrientation.landscapeLeft,
    ]);

    initializeController();
  }

  @override
  void dispose() {
    _controller.dispose();

    SystemChrome.setPreferredOrientations([
      DeviceOrientation.landscapeRight,
      DeviceOrientation.landscapeLeft,
      DeviceOrientation.portraitUp,
      DeviceOrientation.portraitDown,
    ]);
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
