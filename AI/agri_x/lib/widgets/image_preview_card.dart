import 'dart:io';

import 'package:flutter/material.dart';

class ImagePreviewCard extends StatelessWidget {
  final File? image;

  const ImagePreviewCard({
    super.key,
    required this.image,
  });

  @override
  Widget build(BuildContext context) {
    return Container(
      height: 320,
      width: double.infinity,
      decoration: BoxDecoration(
        color: Colors.white,
        borderRadius: BorderRadius.circular(28),
        boxShadow: [
          BoxShadow(
            color: Colors.green.withOpacity(.12),
            blurRadius: 20,
            offset: const Offset(0, 8),
          ),
        ],
      ),
      child: ClipRRect(
        borderRadius: BorderRadius.circular(28),
        child: image == null
            ? _buildPlaceholder()
            : Stack(
                fit: StackFit.expand,
                children: [
                  Image.file(
                    image!,
                    fit: BoxFit.cover,
                  ),

                  Positioned(
                    top: 15,
                    right: 15,
                    child: Container(
                      padding: const EdgeInsets.symmetric(
                        horizontal: 12,
                        vertical: 8,
                      ),
                      decoration: BoxDecoration(
                        color: Colors.black54,
                        borderRadius: BorderRadius.circular(25),
                      ),
                      child: const Row(
                        mainAxisSize: MainAxisSize.min,
                        children: [
                          Icon(
                            Icons.check_circle,
                            color: Colors.greenAccent,
                            size: 18,
                          ),
                          SizedBox(width: 6),
                          Text(
                            "Image Selected",
                            style: TextStyle(
                              color: Colors.white,
                              fontWeight: FontWeight.w600,
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                ],
              ),
      ),
    );
  }

  Widget _buildPlaceholder() {
    return Container(
      decoration: const BoxDecoration(
        gradient: LinearGradient(
          colors: [
            Color(0xFFF7FFF7),
            Color(0xFFE8F5E9),
          ],
          begin: Alignment.topCenter,
          end: Alignment.bottomCenter,
        ),
      ),
      child: Column(
        mainAxisAlignment: MainAxisAlignment.center,
        children: const [
          Icon(
            Icons.document_scanner_rounded,
            size: 90,
            color: Color(0xFF2E7D32),
          ),

          SizedBox(height: 20),

          Text(
            "Upload a Leaf Image",
            style: TextStyle(
              fontSize: 24,
              fontWeight: FontWeight.bold,
            ),
          ),

          SizedBox(height: 10),

          Padding(
            padding: EdgeInsets.symmetric(horizontal: 35),
            child: Text(
              "Capture a fresh leaf or choose an image from your gallery for AI-powered disease detection.",
              textAlign: TextAlign.center,
              style: TextStyle(
                color: Colors.grey,
                fontSize: 15,
                height: 1.5,
              ),
            ),
          ),

          SizedBox(height: 25),

          Icon(
            Icons.touch_app,
            color: Colors.green,
            size: 32,
          ),

          SizedBox(height: 8),

          Text(
            "Ready to Scan",
            style: TextStyle(
              color: Colors.green,
              fontWeight: FontWeight.w600,
            ),
          ),
        ],
      ),
    );
  }
}