import 'dart:convert';
import 'package:http/http.dart' as http;

class ApiService {
  // Replace with your PC's IP if it changes
  static const String baseUrl = "http://10.64.226.204:8000";

  static Future<Map<String, dynamic>> predict(
      String imagePath) async {
    var request = http.MultipartRequest(
      'POST',
      Uri.parse("$baseUrl/predict"),
    );

    request.files.add(
      await http.MultipartFile.fromPath(
        'file',
        imagePath,
      ),
    );

    var response = await request.send();

    if (response.statusCode == 200) {
      var body = await response.stream.bytesToString();
      return jsonDecode(body);
    } else {
      throw Exception("Prediction Failed");
    }
  }
}