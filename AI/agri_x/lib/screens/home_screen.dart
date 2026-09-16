import 'dart:io';

import 'package:flutter/material.dart';
import 'package:image_picker/image_picker.dart';

import '../models/history_model.dart';
import '../services/api_service.dart';
import '../services/history_service.dart';
import '../widgets/action_buttons.dart';
import '../widgets/analyze_button.dart';
import '../widgets/home_header.dart';
import '../widgets/image_preview_card.dart';

import 'live_camera_screen.dart';
import 'result_screen.dart';
import 'scanning_screen.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => _HomeScreenState();
}

class _HomeScreenState extends State<HomeScreen> {
  final ImagePicker _picker = ImagePicker();

  File? _selectedImage;

  Future<void> _pickImage(ImageSource source) async {
    final image = await _picker.pickImage(source: source);

    if (image != null) {
      setState(() {
        _selectedImage = File(image.path);
      });
    }
  }

  Future<void> _analyzeLeaf() async {
    if (_selectedImage == null) {
      ScaffoldMessenger.of(context).showSnackBar(
        const SnackBar(
          content: Text("Please select an image first."),
        ),
      );
      return;
    }

    Navigator.push(
      context,
      MaterialPageRoute(
        builder: (_) => const ScanningScreen(),
      ),
    );

    try {
      final result = await ApiService.predict(
        _selectedImage!.path,
      );

      await HistoryService.saveScan(
        HistoryModel(
          imagePath: _selectedImage!.path,
          crop: result["crop"].toString(),
          disease: result["disease"].toString(),
          confidence:
              (result["confidence"] as num)
                  .toDouble(),
          severity:
              result["severity"].toString(),

          // Added for Google Maps support
          latitude: 0,
          longitude: 0,

          date: DateTime.now(),
        ),
      );

      if (!mounted) return;

      Navigator.pop(context);

      Navigator.push(
        context,
        MaterialPageRoute(
          builder: (_) => ResultScreen(
            image: _selectedImage!,
            result: result,
          ),
        ),
      );
    } catch (e) {
      if (!mounted) return;

      Navigator.pop(context);

      ScaffoldMessenger.of(
        context,
      ).showSnackBar(
        SnackBar(
          content: Text(e.toString()),
        ),
      );
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor:
          const Color(0xFFF4FFF6),
      body: SafeArea(
        child: SingleChildScrollView(
          physics:
              const BouncingScrollPhysics(),
          child: Column(
            children: [
              const HomeHeader(),

              const SizedBox(height: 25),

              Padding(
                padding:
                    const EdgeInsets.symmetric(
                  horizontal: 20,
                ),
                child: ImagePreviewCard(
                  image: _selectedImage,
                ),
              ),

              const SizedBox(height: 25),

              Padding(
                padding:
                    const EdgeInsets.symmetric(
                  horizontal: 20,
                ),
                child: ActionButtons(
                  onCameraTap: () {
                    _pickImage(
                      ImageSource.camera,
                    );
                  },
                  onGalleryTap: () {
                    _pickImage(
                      ImageSource.gallery,
                    );
                  },
                ),
              ),

              const SizedBox(height: 30),

              Padding(
                padding:
                    const EdgeInsets.symmetric(
                  horizontal: 20,
                ),
                child: AnalyzeButton(
                  isEnabled:
                      _selectedImage != null,
                  onPressed: _analyzeLeaf,
                ),
              ),

              const SizedBox(height: 18),

              Padding(
                padding:
                    const EdgeInsets.symmetric(
                  horizontal: 20,
                ),
                child: SizedBox(
                  width: double.infinity,
                  height: 60,
                  child: ElevatedButton.icon(
                    style:
                        ElevatedButton.styleFrom(
                      backgroundColor:
                          const Color(
                        0xFF2E7D32,
                      ),
                      foregroundColor:
                          Colors.white,
                      elevation: 4,
                      shape:
                          RoundedRectangleBorder(
                        borderRadius:
                            BorderRadius
                                .circular(16),
                      ),
                    ),
                    icon: const Icon(
                      Icons.videocam_rounded,
                    ),
                    label: const Text(
                      "Live Camera Detection",
                      style: TextStyle(
                        fontSize: 17,
                        fontWeight:
                            FontWeight.bold,
                      ),
                    ),
                    onPressed: () {
                      Navigator.push(
                        context,
                        MaterialPageRoute(
                          builder: (_) =>
                              const LiveCameraScreen(),
                        ),
                      );
                    },
                  ),
                ),
              ),

              const SizedBox(height: 30),
                            Container(
                margin: const EdgeInsets.symmetric(
                  horizontal: 20,
                ),
                padding: const EdgeInsets.all(20),
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius:
                      BorderRadius.circular(24),
                  boxShadow: [
                    BoxShadow(
                      color: Colors.green
                          .withOpacity(.08),
                      blurRadius: 15,
                      offset: const Offset(0, 5),
                    ),
                  ],
                ),
                child: Column(
                  children: const [
                    Row(
                      children: [
                        Icon(
                          Icons.tips_and_updates,
                          color: Colors.green,
                        ),
                        SizedBox(width: 10),
                        Text(
                          "Tips for Best Results",
                          style: TextStyle(
                            fontSize: 18,
                            fontWeight:
                                FontWeight.bold,
                          ),
                        ),
                      ],
                    ),
                    SizedBox(height: 20),
                    ListTile(
                      leading: CircleAvatar(
                        backgroundColor:
                            Color(0xFFE8F5E9),
                        child: Icon(
                          Icons.wb_sunny,
                          color: Colors.green,
                        ),
                      ),
                      title: Text(
                        "Capture in good lighting",
                      ),
                    ),
                    ListTile(
                      leading: CircleAvatar(
                        backgroundColor:
                            Color(0xFFE8F5E9),
                        child: Icon(
                          Icons
                              .center_focus_strong,
                          color: Colors.green,
                        ),
                      ),
                      title: Text(
                        "Keep the leaf centered",
                      ),
                    ),
                    ListTile(
                      leading: CircleAvatar(
                        backgroundColor:
                            Color(0xFFE8F5E9),
                        child: Icon(
                          Icons.high_quality,
                          color: Colors.green,
                        ),
                      ),
                      title: Text(
                        "Use a clear, high-quality image",
                      ),
                    ),
                    ListTile(
                      leading: CircleAvatar(
                        backgroundColor:
                            Color(0xFFE8F5E9),
                        child: Icon(
                          Icons.no_food,
                          color: Colors.green,
                        ),
                      ),
                      title: Text(
                        "Avoid blurry or damaged photos",
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 30),

              Container(
                margin: const EdgeInsets.symmetric(
                  horizontal: 20,
                ),
                padding: const EdgeInsets.all(20),
                decoration: BoxDecoration(
                  gradient:
                      const LinearGradient(
                    colors: [
                      Color(0xFF1B5E20),
                      Color(0xFF2E7D32),
                    ],
                    begin:
                        Alignment.topLeft,
                    end: Alignment
                        .bottomRight,
                  ),
                  borderRadius:
                      BorderRadius.circular(24),
                  boxShadow: const [
                    BoxShadow(
                      color: Colors.green,
                      blurRadius: 12,
                      offset: Offset(0, 5),
                    ),
                  ],
                ),
                child: const Row(
                  children: [
                    CircleAvatar(
                      radius: 28,
                      backgroundColor:
                          Colors.white,
                      child: Icon(
                        Icons
                            .psychology_alt,
                        color: Color(
                          0xFF2E7D32,
                        ),
                        size: 32,
                      ),
                    ),
                    SizedBox(width: 18),
                    Expanded(
                      child: Column(
                        crossAxisAlignment:
                            CrossAxisAlignment
                                .start,
                        children: [
                          Text(
                            "AI Powered Analysis",
                            style: TextStyle(
                              color:
                                  Colors.white,
                              fontWeight:
                                  FontWeight
                                      .bold,
                              fontSize: 19,
                            ),
                          ),
                          SizedBox(height: 6),
                          Text(
                            "Powered by YOLOv8 and FastAPI to accurately detect crop diseases in seconds.",
                            style: TextStyle(
                              color: Colors
                                  .white70,
                              height: 1.5,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 30),
                            Container(
                margin: const EdgeInsets.symmetric(
                  horizontal: 20,
                ),
                padding: const EdgeInsets.all(18),
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius:
                      BorderRadius.circular(22),
                  boxShadow: const [
                    BoxShadow(
                      color: Colors.black12,
                      blurRadius: 10,
                      offset: Offset(0, 4),
                    ),
                  ],
                ),
                child: const Row(
                  children: [
                    Icon(
                      Icons.security,
                      color: Colors.green,
                      size: 30,
                    ),
                    SizedBox(width: 15),
                    Expanded(
                      child: Text(
                        "Your images are processed securely and scan history is stored locally on your device.",
                        style: TextStyle(
                          fontSize: 15,
                          color: Colors.black87,
                          height: 1.4,
                        ),
                      ),
                    ),
                  ],
                ),
              ),

              const SizedBox(height: 35),

              const Text(
                "AGRI-X v1.0",
                style: TextStyle(
                  fontSize: 20,
                  fontWeight: FontWeight.bold,
                  color: Color(0xFF2E7D32),
                ),
              ),

              const SizedBox(height: 6),

              const Text(
                "Smart Farming • Healthy Crops • Better Future",
                textAlign: TextAlign.center,
                style: TextStyle(
                  color: Colors.grey,
                  fontSize: 14,
                ),
              ),

              const SizedBox(height: 30),
            ],
          ),
        ),
      ),
    );
  }
}