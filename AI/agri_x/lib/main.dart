import 'package:flutter/material.dart';

import 'screens/main_screen.dart';
import 'theme/app_theme.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  runApp(const AgriXApp());
}

class AgriXApp extends StatelessWidget {
  const AgriXApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'AGRI-X',
      theme: AppTheme.lightTheme,
      home: const MainScreen(),
    );
  }
}