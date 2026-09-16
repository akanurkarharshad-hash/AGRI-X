import 'package:flutter/material.dart';

class AnalyzeButton extends StatelessWidget {
  final VoidCallback onPressed;
  final bool isEnabled;

  const AnalyzeButton({
    super.key,
    required this.onPressed,
    this.isEnabled = true,
  });

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: double.infinity,
      height: 70,
      child: ElevatedButton(
        onPressed: isEnabled ? onPressed : null,
        style: ElevatedButton.styleFrom(
          padding: EdgeInsets.zero,
          elevation: 8,
          shadowColor: Colors.green.withOpacity(.35),
          shape: RoundedRectangleBorder(
            borderRadius: BorderRadius.circular(24),
          ),
        ),
        child: Ink(
          decoration: BoxDecoration(
            gradient: const LinearGradient(
              colors: [
                Color(0xFF1B5E20),
                Color(0xFF2E7D32),
                Color(0xFF43A047),
              ],
            ),
            borderRadius: BorderRadius.circular(24),
          ),
          child: Container(
            alignment: Alignment.center,
            child: const Row(
              mainAxisAlignment: MainAxisAlignment.center,
              children: [
                Icon(
                  Icons.auto_awesome,
                  color: Colors.white,
                  size: 30,
                ),

                SizedBox(width: 12),

                Text(
                  "Analyze Leaf",
                  style: TextStyle(
                    fontSize: 20,
                    fontWeight: FontWeight.bold,
                    color: Colors.white,
                    letterSpacing: .4,
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}