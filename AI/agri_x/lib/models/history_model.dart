class HistoryModel {
  final String imagePath;
  final String crop;
  final String disease;
  final double confidence;
  final String severity;

  // NEW
  final double latitude;
  final double longitude;

  final DateTime date;

  const HistoryModel({
    required this.imagePath,
    required this.crop,
    required this.disease,
    required this.confidence,
    required this.severity,
    required this.latitude,
    required this.longitude,
    required this.date,
  });

  Map<String, dynamic> toJson() {
    return {
      'imagePath': imagePath,
      'crop': crop,
      'disease': disease,
      'confidence': confidence,
      'severity': severity,
      'latitude': latitude,
      'longitude': longitude,
      'date': date.toIso8601String(),
    };
  }

  factory HistoryModel.fromJson(Map<String, dynamic> json) {
    return HistoryModel(
      imagePath: json['imagePath'] ?? "",
      crop: json['crop'] ?? "",
      disease: json['disease'] ?? "",
      confidence: (json['confidence'] as num).toDouble(),
      severity: json['severity'] ?? "",

      latitude: (json['latitude'] ?? 0).toDouble(),
      longitude: (json['longitude'] ?? 0).toDouble(),

      date: DateTime.parse(json['date']),
    );
  }
}