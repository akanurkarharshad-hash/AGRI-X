import 'package:flutter/material.dart';

class ScanningScreen extends StatelessWidget {
  const ScanningScreen({super.key});

  Widget buildStep(IconData icon, String text) {
    return ListTile(
      leading: CircleAvatar(
        backgroundColor: Colors.green.shade100,
        child: Icon(icon, color: Colors.green),
      ),
      title: Text(
        text,
        style: const TextStyle(
          fontWeight: FontWeight.w600,
          fontSize: 16,
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFF5FFF6),
      body: Center(
        child: Padding(
          padding: const EdgeInsets.all(24),
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              const Icon(
                Icons.eco,
                size: 90,
                color: Colors.green,
              ),

              const SizedBox(height: 20),

              const Text(
                "AGRI-X",
                style: TextStyle(
                  fontSize: 34,
                  fontWeight: FontWeight.bold,
                ),
              ),

              const SizedBox(height: 10),

              const Text(
                "Analyzing your crop...",
                style: TextStyle(
                  color: Colors.grey,
                  fontSize: 18,
                ),
              ),

              const SizedBox(height: 35),

              const CircularProgressIndicator(),

              const SizedBox(height: 40),

              buildStep(Icons.cloud_upload, "Uploading Image"),
              buildStep(Icons.search, "Detecting Disease"),
              buildStep(Icons.psychology, "Running AI Model"),
              buildStep(Icons.medical_services, "Preparing Recommendation"),
            ],
          ),
        ),
      ),
    );
  }
}