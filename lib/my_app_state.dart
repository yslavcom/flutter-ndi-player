import 'package:flutter/material.dart';

class NdiMonitorState extends ChangeNotifier {
    void getNext(){
        notifyListeners();
    }
}
