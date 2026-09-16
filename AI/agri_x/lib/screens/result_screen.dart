import 'dart:io';

import 'package:flutter/material.dart';
import 'package:percent_indicator/circular_percent_indicator.dart';

class ResultScreen extends StatelessWidget {
  final File image;
  final Map<String, dynamic> result;

  const ResultScreen({
    super.key,
    required this.image,
    required this.result,
  });

  Color getSeverityColor(String severity) {
    switch (severity.toLowerCase()) {
      case "healthy":
        return const Color(0xFF2E7D32);

      case "low":
        return Colors.lightGreen;

      case "medium":
        return Colors.orange;

      case "high":
        return Colors.deepOrange;

      case "very high":
        return Colors.red;

      default:
        return Colors.grey;
    }
  }

  IconData getSeverityIcon(String severity) {
    switch (severity.toLowerCase()) {
      case "healthy":
        return Icons.check_circle;

      case "low":
        return Icons.health_and_safety;

      case "medium":
        return Icons.warning_amber;

      case "high":
        return Icons.error_outline;

      case "very high":
        return Icons.dangerous;

      default:
        return Icons.help_outline;
    }
  }

  @override
  Widget build(BuildContext context) {
    final recommendations =
        List<String>.from(result["recommendation"] ?? []);

    final confidence =
        (result["confidence"] as num).toDouble();

    final severity = result["severity"].toString();

    return Scaffold(
      backgroundColor: const Color(0xffF4FFF6),

      appBar: AppBar(
        title: const Text("AI Diagnosis"),
        centerTitle: true,
      ),

      body: SingleChildScrollView(
        padding: const EdgeInsets.all(20),

        child: Column(
          crossAxisAlignment: CrossAxisAlignment.stretch,

          children: [

            ClipRRect(
              borderRadius: BorderRadius.circular(22),
              child: Image.file(
                image,
                height: 260,
                fit: BoxFit.cover,
              ),
            ),

            const SizedBox(height: 25),

            Card(
              child: Padding(
                padding: const EdgeInsets.all(22),

                child: Column(
                  children: [

                    Text(
                      result["disease"],
                      textAlign: TextAlign.center,
                      style: const TextStyle(
                        fontSize: 28,
                        fontWeight: FontWeight.bold,
                      ),
                    ),

                    const SizedBox(height: 8),

                    Text(
                      result["crop"],
                      style: const TextStyle(
                        color: Colors.grey,
                        fontSize: 18,
                      ),
                    ),

                    const SizedBox(height: 30),

                    const Text(
                      "AI Confidence",
                      style: TextStyle(
                        fontSize: 22,
                        fontWeight: FontWeight.bold,
                      ),
                    ),

                    const SizedBox(height: 20),

                    CircularPercentIndicator(
                      radius: 80,
                      lineWidth: 12,
                      animation: true,
                      animationDuration: 1500,
                      percent:
                          (confidence.clamp(0, 100)) / 100,
                      circularStrokeCap:
                          CircularStrokeCap.round,
                      progressColor: Colors.green,
                      backgroundColor:
                          Colors.green.shade100,

                      center: Column(
                        mainAxisAlignment:
                            MainAxisAlignment.center,
                        children: [

                          Text(
                            "${confidence.toStringAsFixed(1)}%",
                            style: const TextStyle(
                              fontSize: 26,
                              fontWeight:
                                  FontWeight.bold,
                            ),
                          ),

                          const Text(
                            "Confidence",
                            style: TextStyle(
                              color: Colors.grey,
                            ),
                          ),
                        ],
                      ),
                    ),

                    const SizedBox(height: 30),

                    Container(
                      width: double.infinity,
                      padding:
                          const EdgeInsets.all(18),

                      decoration: BoxDecoration(
                        color: getSeverityColor(
                                severity)
                            .withOpacity(.12),

                        borderRadius:
                            BorderRadius.circular(18),

                        border: Border.all(
                          color:
                              getSeverityColor(severity),
                          width: 2,
                        ),
                      ),

                      child: Row(
                        children: [

                          CircleAvatar(
                            radius: 25,
                            backgroundColor:
                                getSeverityColor(
                                    severity),

                            child: Icon(
                              getSeverityIcon(
                                  severity),
                              color: Colors.white,
                            ),
                          ),

                          const SizedBox(width: 18),

                          Expanded(
                            child: Column(
                              crossAxisAlignment:
                                  CrossAxisAlignment
                                      .start,
                              children: [

                                const Text(
                                  "Disease Severity",
                                  style: TextStyle(
                                    color:
                                        Colors.grey,
                                  ),
                                ),

                                const SizedBox(
                                    height: 4),

                                Text(
                                  severity
                                      .toUpperCase(),
                                  style: TextStyle(
                                    color:
                                        getSeverityColor(
                                            severity),
                                    fontSize: 22,
                                    fontWeight:
                                        FontWeight
                                            .bold,
                                  ),
                                ),
                              ],
                            ),
                          ),
                        ],
                      ),
                    ),
                  ],
                ),
              ),
            ),

            const SizedBox(height: 30),

            const Text(
              "Treatment Recommendations",
              style: TextStyle(
                fontSize: 22,
                fontWeight: FontWeight.bold,
              ),
            ),

            const SizedBox(height: 15),
                        ...recommendations.map(
              (item) => Padding(
                padding: const EdgeInsets.only(bottom: 12),
                child: Container(
                  decoration: BoxDecoration(
                    color: Colors.white,
                    borderRadius: BorderRadius.circular(18),
                    boxShadow: [
                      BoxShadow(
                        color: Colors.black.withOpacity(.05),
                        blurRadius: 8,
                        offset: const Offset(0, 3),
                      ),
                    ],
                  ),
                  child: ListTile(
                    contentPadding: const EdgeInsets.symmetric(
                      horizontal: 18,
                      vertical: 8,
                    ),
                    leading: Container(
                      height: 45,
                      width: 45,
                      decoration: BoxDecoration(
                        color: Colors.green.shade100,
                        borderRadius: BorderRadius.circular(12),
                      ),
                      child: const Icon(
                        Icons.medication,
                        color: Colors.green,
                      ),
                    ),
                    title: Text(
                      item,
                      style: const TextStyle(
                        fontWeight: FontWeight.w600,
                        fontSize: 16,
                      ),
                    ),
                  ),
                ),
              ),
            ),

            const SizedBox(height: 30),

            Row(
              children: [
                Expanded(
                  child: OutlinedButton.icon(
                    onPressed: () {
                      ScaffoldMessenger.of(context).showSnackBar(
                        const SnackBar(
                          content: Text(
                            "Share feature coming soon 🚀",
                          ),
                        ),
                      );
                    },
                    icon: const Icon(Icons.share),
                    label: const Text("Share"),
                    style: OutlinedButton.styleFrom(
                      minimumSize: const Size.fromHeight(55),
                      side: const BorderSide(
                        color: Colors.green,
                        width: 2,
                      ),
                      foregroundColor: Colors.green,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(15),
                      ),
                    ),
                  ),
                ),

                const SizedBox(width: 15),

                Expanded(
                  child: ElevatedButton.icon(
                    onPressed: () {
                      Navigator.pop(context);
                    },
                    icon: const Icon(Icons.refresh),
                    label: const Text("Analyze Again"),
                    style: ElevatedButton.styleFrom(
                      minimumSize: const Size.fromHeight(55),
                      backgroundColor: Colors.green,
                      foregroundColor: Colors.white,
                      shape: RoundedRectangleBorder(
                        borderRadius: BorderRadius.circular(15),
                      ),
                    ),
                  ),
                ),
              ],
            ),

            const SizedBox(height: 25),

            const Center(
              child: Text(
                "Powered by AGRI-X AI",
                style: TextStyle(
                  color: Colors.grey,
                  fontSize: 13,
                ),
              ),
            ),

            const SizedBox(height: 20),
          ],
        ),
      ),
    );
  }
}