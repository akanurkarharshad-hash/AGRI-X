import 'package:flutter/material.dart';
import 'package:camera/camera.dart';

import '../services/camera_service.dart';
import '../services/location_service.dart';
import '../services/history_service.dart';
import '../models/history_model.dart';

class LiveCameraScreen extends StatefulWidget {
  const LiveCameraScreen({super.key});

  @override
  State<LiveCameraScreen> createState() => _LiveCameraScreenState();
}

class _LiveCameraScreenState extends State<LiveCameraScreen> {
  final CameraService _cameraService = CameraService();

  String crop = "-";
  String disease = "-";
  String severity = "-";
  double confidence = 0;

  List<dynamic> recommendations = [];

  bool loading = true;

  String _lastDisease = "";
  DateTime? _lastSaved;

  @override
  void initState() {
    super.initState();
    _initialize();
  }

  Future<void> _initialize() async {
    await _cameraService.initialize();

    _cameraService.onPrediction =
        (prediction, imagePath) async {
      if (!mounted) return;

      setState(() {
        crop = prediction["crop"] ?? "-";
        disease = prediction["disease"] ?? "-";
        severity = prediction["severity"] ?? "-";

        confidence =
            (prediction["confidence"] ?? 0).toDouble();

        recommendations =
            prediction["recommendation"] ?? [];

        loading = false;
      });

      await _saveDetection(imagePath);
    };

    _cameraService.startDetection();

    setState(() {});
  }

  Future<void> _saveDetection(String imagePath) async {
    try {
      if (disease == "-" || disease.isEmpty) {
        return;
      }

      if (_lastDisease == disease &&
          _lastSaved != null &&
          DateTime.now()
                  .difference(_lastSaved!)
                  .inSeconds <
              10) {
        return;
      }

      final position =
          await LocationService.getCurrentLocation();

      final history = HistoryModel(
        imagePath: imagePath,
        crop: crop,
        disease: disease,
        confidence: confidence,
        severity: severity,
        latitude: position.latitude,
        longitude: position.longitude,
        date: DateTime.now(),
      );

      await HistoryService.saveScan(history);

      _lastDisease = disease;
      _lastSaved = DateTime.now();
    } catch (e) {
      debugPrint("Save Error: $e");
    }
  }

  @override
  void dispose() {
    _cameraService.dispose();
    super.dispose();
  }

  Widget _infoTile(
    String title,
    String value,
    Color color,
  ) {
    return Card(
      elevation: 3,
      child: ListTile(
        title: Text(title),
        subtitle: Text(
          value,
          style: TextStyle(
            color: color,
            fontWeight: FontWeight.bold,
            fontSize: 18,
          ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    if (!_cameraService.isInitialized ||
        _cameraService.controller == null) {
      return const Scaffold(
        body: Center(
          child: CircularProgressIndicator(),
        ),
      );
    }

    return Scaffold(
      appBar: AppBar(
        title: const Text("Live Disease Detection"),
        centerTitle: true,
      ),
      body: Stack(
        children: [
          Positioned.fill(
            child: CameraPreview(
              _cameraService.controller!,
            ),
          ),

          Positioned(
            left: 12,
            right: 12,
            bottom: 12,
            child: Card(
              color: Colors.white.withOpacity(0.95),
              elevation: 10,
              child: Padding(
                padding:
                    const EdgeInsets.all(15),
                child: loading
                    ? const Center(
                        child:
                            CircularProgressIndicator(),
                      )
                    : SingleChildScrollView(
                        child: Column(
                          crossAxisAlignment:
                              CrossAxisAlignment
                                  .start,
                          children: [
                            _infoTile(
                              "Crop",
                              crop,
                              Colors.green,
                            ),

                            _infoTile(
                              "Disease",
                              disease,
                              Colors.red,
                            ),

                            _infoTile(
                              "Severity",
                              severity,
                              Colors.orange,
                            ),

                            const SizedBox(
                                height: 10),

                            const Text(
                              "Confidence",
                              style: TextStyle(
                                fontWeight:
                                    FontWeight.bold,
                                fontSize: 16,
                              ),
                            ),

                            const SizedBox(
                                height: 8),

                            LinearProgressIndicator(
                              value:
                                  confidence / 100,
                              minHeight: 10,
                            ),

                            const SizedBox(
                                height: 8),

                            Text(
                              "${confidence.toStringAsFixed(2)} %",
                              style:
                                  const TextStyle(
                                fontWeight:
                                    FontWeight.bold,
                              ),
                            ),

                            const SizedBox(
                                height: 15),

                            const Text(
                              "Recommendations",
                              style: TextStyle(
                                fontWeight:
                                    FontWeight.bold,
                                fontSize: 18,
                              ),
                            ),

                            const SizedBox(
                                height: 8),
                                                            ...recommendations.map(
                              (item) => Padding(
                                padding:
                                    const EdgeInsets.symmetric(
                                  vertical: 3,
                                ),
                                child: Row(
                                  crossAxisAlignment:
                                      CrossAxisAlignment.start,
                                  children: [
                                    const Text("• "),
                                    Expanded(
                                      child: Text(
                                        item.toString(),
                                      ),
                                    ),
                                  ],
                                ),
                              ),
                            ),
                          ],
                        ),
                      ),
              ),
            ),
          ),
        ],
      ),
    );
  }
}