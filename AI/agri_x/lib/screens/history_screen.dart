import 'dart:io';

import 'package:flutter/material.dart';

import '../models/history_model.dart';
import '../services/history_service.dart';

class HistoryScreen extends StatefulWidget {
  const HistoryScreen({super.key});

  @override
  State<HistoryScreen> createState() => _HistoryScreenState();
}

class _HistoryScreenState extends State<HistoryScreen> {
  List<HistoryModel> history = [];

  @override
  void initState() {
    super.initState();
    loadHistory();
  }

  Future<void> loadHistory() async {
    final data = await HistoryService.getHistory();

    setState(() {
      history = data;
    });
  }

  Color severityColor(String severity) {
    switch (severity.toLowerCase()) {
      case "healthy":
        return Colors.green;
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

  Future<void> deleteItem(int index) async {
    await HistoryService.deleteScan(index);
    loadHistory();
  }

  Future<void> clearAll() async {
    await HistoryService.clearHistory();
    loadHistory();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFF4FFF6),

      appBar: AppBar(
        title: const Text("Scan History"),
        centerTitle: true,
        actions: [
          if (history.isNotEmpty)
            IconButton(
              onPressed: clearAll,
              icon: const Icon(Icons.delete_sweep),
            ),
        ],
      ),

      body: history.isEmpty
          ? const Center(
              child: Column(
                mainAxisAlignment: MainAxisAlignment.center,
                children: [
                  Icon(
                    Icons.history,
                    size: 80,
                    color: Colors.grey,
                  ),
                  SizedBox(height: 20),
                  Text(
                    "No Scan History Yet",
                    style: TextStyle(
                      fontSize: 22,
                      fontWeight: FontWeight.bold,
                    ),
                  ),
                  SizedBox(height: 8),
                  Text(
                    "Your previous scans will appear here.",
                    style: TextStyle(color: Colors.grey),
                  ),
                ],
              ),
            )
          : ListView.builder(
              padding: const EdgeInsets.all(16),
              itemCount: history.length,
              itemBuilder: (context, index) {
                final item = history[index];

                return Card(
                  elevation: 4,
                  margin: const EdgeInsets.only(bottom: 16),
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(18),
                  ),
                  child: Padding(
                    padding: const EdgeInsets.all(14),
                    child: Row(
                      children: [
                        ClipRRect(
                          borderRadius: BorderRadius.circular(14),
                          child: Image.file(
                            File(item.imagePath),
                            width: 85,
                            height: 85,
                            fit: BoxFit.cover,
                            errorBuilder: (_, __, ___) {
                              return Container(
                                width: 85,
                                height: 85,
                                color: Colors.grey.shade300,
                                child: const Icon(Icons.image_not_supported),
                              );
                            },
                          ),
                        ),

                        const SizedBox(width: 15),

                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                item.crop,
                                style: const TextStyle(
                                  fontSize: 18,
                                  fontWeight: FontWeight.bold,
                                ),
                              ),

                              const SizedBox(height: 4),

                              Text(
                                item.disease,
                                style: const TextStyle(
                                  color: Colors.grey,
                                ),
                              ),

                              const SizedBox(height: 8),

                              Text(
                                "Confidence: ${item.confidence.toStringAsFixed(1)}%",
                                style: const TextStyle(
                                  fontWeight: FontWeight.w600,
                                ),
                              ),

                              const SizedBox(height: 8),

                              Container(
                                padding: const EdgeInsets.symmetric(
                                  horizontal: 10,
                                  vertical: 4,
                                ),
                                decoration: BoxDecoration(
                                  color: severityColor(item.severity)
                                      .withOpacity(.15),
                                  borderRadius:
                                      BorderRadius.circular(20),
                                ),
                                child: Text(
                                  item.severity.toUpperCase(),
                                  style: TextStyle(
                                    color: severityColor(item.severity),
                                    fontWeight: FontWeight.bold,
                                  ),
                                ),
                              ),

                              const SizedBox(height: 8),

                              Text(
                                "${item.date.day}/${item.date.month}/${item.date.year}",
                                style: const TextStyle(
                                  color: Colors.grey,
                                  fontSize: 12,
                                ),
                              ),
                            ],
                          ),
                        ),

                        IconButton(
                          onPressed: () => deleteItem(index),
                          icon: const Icon(
                            Icons.delete,
                            color: Colors.red,
                          ),
                        ),
                      ],
                    ),
                  ),
                );
              },
            ),
    );
  }
}