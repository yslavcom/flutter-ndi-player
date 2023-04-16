import 'package:flutter/foundation.dart';
import 'package:flutter/services.dart';

class OpenGLTextureController {
  static const MethodChannel _channel = MethodChannel('opengl_texture');

    int textureId = -1;

  Future<int> initialize(int width, int height) async {
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

  Future<void> dispose() =>
      _channel.invokeMethod('dispose', {'textureId': textureId});

  bool get isInitialized => textureId >= 0;
}
