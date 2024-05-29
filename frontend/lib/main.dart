import 'package:flutter/material.dart';
import 'package:lottie/lottie.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  void _showDialog() {
    showDialog(
        context: context,
        builder: (context) {
          return StatefulBuilder(builder: (context, StateSetter setState) {
            return Dialog(
              child: Column(
                mainAxisSize: MainAxisSize.min,
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  Padding(
                      padding: const EdgeInsets.all(16),
                      // Animation can be found in animations folder.
                      // You must add the animations folder in your `assets` section
                      // of pubspec.yaml
                      child: Lottie.asset(
                          'animations/running-faucet-lottie.json')),
                  const SizedBox(height: 16),
                  const Padding(
                    padding: EdgeInsets.all(16),
                    child: Text(
                      'Please make sure to close all water outlets before proceeding. Refrain from using the water while the scan is ongoing.',
                      textAlign: TextAlign.justify,
                    ),
                  ),
                  const SizedBox(height: 16),
                  FilledButton(
                    onPressed: () {
                      // run manual scan dito
                      Navigator.pop(context);
                    },
                    child: const Text('Proceed'),
                  ),
                  const SizedBox(height: 16),
                ],
              ),
            );
          });
        });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: <Widget>[
            // Button to open dialog for manual scan
            ElevatedButton(
                onPressed: () {
                  _showDialog();
                },
                child: Text(
                  'Manual Scan',
                  style: Theme.of(context).textTheme.bodyMedium,
                ))
          ],
        ),
      ),
    );
  }
}
