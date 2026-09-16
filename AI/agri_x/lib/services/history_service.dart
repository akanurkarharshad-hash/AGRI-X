import 'dart:convert';

import 'package:shared_preferences/shared_preferences.dart';

import '../models/history_model.dart';

class HistoryService {
  static const String _historyKey = 'scan_history';

  /// Save a new scan
  static Future<void> saveScan(HistoryModel history) async {
    final prefs = await SharedPreferences.getInstance();

    final List<HistoryModel> historyList = await getHistory();

    // Add newest scan at the top
    historyList.insert(0, history);

    final jsonList =
        historyList.map((e) => jsonEncode(e.toJson())).toList();

    await prefs.setStringList(_historyKey, jsonList);
  }

  /// Load all scans
  static Future<List<HistoryModel>> getHistory() async {
    final prefs = await SharedPreferences.getInstance();

    final jsonList = prefs.getStringList(_historyKey) ?? [];

    return jsonList
        .map(
          (e) => HistoryModel.fromJson(
            jsonDecode(e),
          ),
        )
        .toList();
  }

  /// Delete all history
  static Future<void> clearHistory() async {
    final prefs = await SharedPreferences.getInstance();
    await prefs.remove(_historyKey);
  }

  /// Delete one scan
  static Future<void> deleteScan(int index) async {
    final prefs = await SharedPreferences.getInstance();

    final history = await getHistory();

    if (index >= 0 && index < history.length) {
      history.removeAt(index);

      final jsonList =
          history.map((e) => jsonEncode(e.toJson())).toList();

      await prefs.setStringList(_historyKey, jsonList);
    }
  }
}