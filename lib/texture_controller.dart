import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

class OpenGLTextureController {
  static const MethodChannel _channel = const MethodChannel('opengl_texture');

  int textureId = 0;

  Future<int> initialize(double width, double height) async {
    if (kDebugMode) {
      print('textureId-1:$textureId');
    }
    textureId = await _channel.invokeMethod('create', {
      'width': width,
      'height': height,
    });

    if (kDebugMode) {
      print('textureId-2:$textureId');
    }
    return textureId;
  }

  Future<Null> dispose() =>
      _channel.invokeMethod('dispose', {'textureId': textureId});

  bool get isInitialized => textureId != null;
}
