import 'dart:async';

import 'package:camera/camera.dart';
import 'package:flutter/foundation.dart';

import 'api_service.dart';

typedef PredictionCallback = void Function(
  Map<String, dynamic> prediction,
  String imagePath,
);

class CameraService {
  CameraController? controller;

  List<CameraDescription> cameras = [];

  Timer? _timer;

  bool isInitialized = false;
  bool isCapturing = false;

  int _tick = 0;

  PredictionCallback? onPrediction;

  Future<void> initialize() async {
    cameras = await availableCameras();

    if (cameras.isEmpty) {
      throw Exception("No camera found");
    }

    controller = CameraController(
      cameras.first,
      ResolutionPreset.medium,
      enableAudio: false,
    );

    await controller!.initialize();

    isInitialized = true;
  }

  void startDetection() {
    if (!isInitialized) return;

    _timer?.cancel();

    _timer = Timer.periodic(
      const Duration(milliseconds: 100),
      (_) async {
        if (isCapturing) return;

        _tick++;

        // Capture every 7th frame
        if (_tick % 7 != 0) return;

        await _captureAndPredict();
      },
    );
  }

  void stopDetection() {
    _timer?.cancel();
  }

  Future<void> dispose() async {
    stopDetection();
    await controller?.dispose();
  }

  Future<void> _captureAndPredict() async {
    if (controller == null) return;

    if (!controller!.value.isInitialized) return;

    if (controller!.value.isTakingPicture) return;

    isCapturing = true;

    try {
      final XFile picture = await controller!.takePicture();

      final result = await ApiService.predict(
        picture.path,
      );

      onPrediction?.call(
        result,
        picture.path,
      );
    } catch (e) {
      debugPrint("Prediction Error: $e");
    } finally {
      isCapturing = false;
    }
  }
}