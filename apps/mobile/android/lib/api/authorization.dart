import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;
import 'package:flutter_secure_storage/flutter_secure_storage.dart';
import 'package:android/ui/components/notification.dart';

import 'dart:io' show Platform;
import 'package:flutter/foundation.dart' show kIsWeb;

class Auth {
  static String get baseURL {
    if (!kIsWeb && Platform.isAndroid) {
      return 'http://10.0.2.2:8000/api';
    }
    return 'http://127.0.0.1:8000/api';
  }

  static const _storage = FlutterSecureStorage();

  static Future<void> _saveTokens(String access, String refresh) async {
    await _storage.write(key: 'access', value: access);
    await _storage.write(key: 'refresh', value: refresh);
  }

  static Future<void> _saveToken(String token) async {
    await _storage.write(key: 'access', value: token);
  }

  static Future<bool> updateProfile(Map<String, dynamic> data) async {
    final user = await getProfile();
    final userID = user?['id']?.toString(); 
    if (userID == null) return false;
    final url = Uri.parse('$baseURL/auth/me/');
    String? token = await getToken();

    if (token == null) return false;

    try {
      final response = await http.patch(
        url,
        headers: {
          'Content-Type': 'application/json',
          'Authorization': 'Bearer $token',
        },
        body: jsonEncode(data),
      );

      return response.statusCode == 200 || response.statusCode == 204;
    } catch (e) {
      print("Profile update error: $e");
      return false;
    }
  }

  static Future<String?> getToken() async {
    return await _storage.read(key: 'access');
  }

  static Future<void> logout() async {
    await _storage.delete(key: 'access');
    await _storage.delete(key: 'refresh');
    showTopNotification(
      message: "Tizimdan muvofaqiyatli chiqdingiz!",
      isError: false,
    );
  }

  static Future<bool> isAuthenticated() async {
    String? token = await getToken();
    return token != null;
  }

  static Future<bool> login(String username, String password) async {
    
    final url = Uri.parse('$baseURL/auth/login/');

    try {
      final response = await http.post(
        url,
        headers: {'Content-Type': 'application/json'},
        body: jsonEncode({'username': username, 'password': password}),
      );

      if (response.statusCode == 200) {
        final data = jsonDecode(response.body);
        final String access = data['access'];
        final String refresh = data['refresh'] ?? '';
        await _saveTokens(access, refresh);
        return true;
      } else {
        return false;
      }
    } catch (e) {
      print(e);
      print("Incorrect password or other");
      return false;
    }
  }

  static Future<Map<String, dynamic>?> getProfile() async {
    final url = Uri.parse('$baseURL/auth/me/');
    String? token = await getToken();

    if (token == null) return null;

    try {
      final response = await http.get(
        url,
        headers: {
          'Content-Type': 'application/json',
          'Authorization': 'Bearer $token',
        },
      );

      if (response.statusCode == 200) {
        return jsonDecode(response.body) as Map<String, dynamic>;
      } else {
        return null;
      }
    } catch (e) {
      print("Profile loading error: $e");
      return null;
    }
  }
}
