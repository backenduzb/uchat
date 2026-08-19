import 'dart:async';
import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';
import 'package:flutter/foundation.dart' show kIsWeb;

import 'package:android/api/authorization.dart';


class ChatMessage {
  final String id;
  final String conversationId;
  final String senderId;
  final String text;
  final DateTime receivedAt;

  ChatMessage({
    required this.id,
    required this.conversationId,
    required this.senderId,
    required this.text,
    required this.receivedAt,
  });
}


abstract class ServerEvent {}

class AuthenticatedEvent extends ServerEvent {
  final String userId;
  AuthenticatedEvent(this.userId);
}

class MessageEvent extends ServerEvent {
  final ChatMessage message;
  MessageEvent(this.message);
}

class MessageSentEvent extends ServerEvent {
  final String id;
  MessageSentEvent(this.id);
}

class ErrorEvent extends ServerEvent {
  final String code;
  final String message;
  ErrorEvent(this.code, this.message);
}

class DisconnectedEvent extends ServerEvent {}

class ChatService {
  static String get _host {
    if (!kIsWeb && Platform.isAndroid) {
      return '10.0.2.2';
    }
    return '127.0.0.1';
  }
  static const int _port = 9000;

  Socket? _socket;
  final _eventController = StreamController<ServerEvent>.broadcast();
  final _buffer = BytesBuilder(copy: false);

  bool _connected = false;
  bool get isConnected => _connected;

  Stream<ServerEvent> get events => _eventController.stream;

  Future<bool> connect() async {
    if (_connected) return true;

    try {
      _socket = await Socket.connect(_host, _port)
          .timeout(const Duration(seconds: 5));
      _connected = true;

      _socket!.listen(
        _onData,
        onError: _onError,
        onDone: _onDone,
        cancelOnError: false,
      );

      final token = await Auth.getToken();
      if (token == null) return false;

      _sendFrame({'type': 'Auth', 'data': {'token': token}});
      return true;
    } catch (e) {
      _connected = false;
      return false;
    }
  }

  void disconnect() {
    _connected = false;
    _socket?.destroy();
    _socket = null;
  }


  void sendMessage(String conversationId, String text) {
    if (!_connected) return;
    _sendFrame({
      'type': 'SendMessage',
      'data': {
        'conversation_id': conversationId,
        'text': text,
      },
    });
  }

  void sendPing() {
    if (!_connected) return;
    _sendFrame({'type': 'Ping', 'data': null});
  }


  void _sendFrame(Map<String, dynamic> payload) {
    if (_socket == null) return;

    final bytes = utf8.encode(jsonEncode(payload));
    final header = ByteData(4)..setUint32(0, bytes.length, Endian.big);

    _socket!.add(header.buffer.asUint8List());
    _socket!.add(bytes);
  }

  void _onData(Uint8List data) {
    _buffer.add(data);

    while (true) {
      final bytes = _buffer.toBytes();

      if (bytes.length < 4) break;

      final length =
          ByteData.sublistView(bytes, 0, 4).getUint32(0, Endian.big);

      if (bytes.length < 4 + length) break;

      final frameBytes = bytes.sublist(4, 4 + length);
      final remaining = bytes.sublist(4 + length);

      _buffer.clear();
      if (remaining.isNotEmpty) _buffer.add(remaining);

      _handleFrame(frameBytes);
    }
  }

  void _handleFrame(Uint8List frameBytes) {
    try {
      final json = jsonDecode(utf8.decode(frameBytes)) as Map<String, dynamic>;
      final type = json['type'] as String?;
      final data = json['data'] as Map<String, dynamic>?;

      switch (type) {
        case 'Authenticated':
          _eventController.add(
            AuthenticatedEvent(data?['user_id'] as String? ?? ''),
          );
          break;

        case 'Message':
          _eventController.add(
            MessageEvent(
              ChatMessage(
                id: data?['id'] as String? ?? '',
                conversationId: data?['conversation_id'] as String? ?? '',
                senderId: data?['sender_id'] as String? ?? '',
                text: data?['text'] as String? ?? '',
                receivedAt: DateTime.now(),
              ),
            ),
          );
          break;

        case 'MessageSent':
          _eventController.add(
            MessageSentEvent(data?['id'] as String? ?? ''),
          );
          break;

        case 'Pong':
          break;

        case 'Error':
          _eventController.add(
            ErrorEvent(
              data?['code'] as String? ?? 'UNKNOWN',
              data?['message'] as String? ?? '',
            ),
          );
          break;
      }
    } catch (_) {
    }
  }

  void _onError(Object error) {
    _connected = false;
    _eventController.add(DisconnectedEvent());
  }

  void _onDone() {
    _connected = false;
    _eventController.add(DisconnectedEvent());
  }

  void dispose() {
    disconnect();
    _eventController.close();
  }
}


final chatService = ChatService();
