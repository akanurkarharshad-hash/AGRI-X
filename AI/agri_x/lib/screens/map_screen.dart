import 'package:flutter/material.dart';
import 'package:google_maps_flutter/google_maps_flutter.dart';

import '../models/history_model.dart';
import '../services/history_service.dart';

class MapScreen extends StatefulWidget {
  const MapScreen({super.key});

  @override
  State<MapScreen> createState() => _MapScreenState();
}

class _MapScreenState extends State<MapScreen> {
  GoogleMapController? _mapController;

  final Set<Marker> _markers = {};

  List<HistoryModel> history = [];

  bool loading = true;

  static const CameraPosition initialCamera =
      CameraPosition(
    target: LatLng(20.5937, 78.9629),
    zoom: 5,
  );

  @override
  void initState() {
    super.initState();
    loadHistory();
  }

  Future<void> loadHistory() async {
    history = await HistoryService.getHistory();

    _markers.clear();

    for (int i = 0; i < history.length; i++) {
      final item = history[i];

      if (item.latitude == 0 &&
          item.longitude == 0) {
        continue;
      }

      _markers.add(
        Marker(
          markerId:
              MarkerId("disease_$i"),

          position: LatLng(
            item.latitude,
            item.longitude,
          ),

          infoWindow: InfoWindow(
            title: item.crop,
            snippet:
                "${item.disease}\nSeverity: ${item.severity}",
          ),
        ),
      );
    }

    if (history.isNotEmpty) {
      final first = history.first;

      _mapController?.animateCamera(
        CameraUpdate.newLatLngZoom(
          LatLng(
            first.latitude,
            first.longitude,
          ),
          18,
        ),
      );
    }

    loading = false;

    if (mounted) {
      setState(() {});
    }
  }

  Widget buildHistoryCard(
      HistoryModel item) {
    return Card(
      margin: const EdgeInsets.symmetric(
        horizontal: 10,
        vertical: 5,
      ),
      child: ListTile(
        leading: const Icon(
          Icons.local_florist,
          color: Colors.green,
        ),
        title: Text(item.crop),

        subtitle: Column(
          crossAxisAlignment:
              CrossAxisAlignment.start,
          children: [
            Text(item.disease),

            Text(
              "Severity : ${item.severity}",
            ),

            Text(
              "Confidence : ${item.confidence.toStringAsFixed(2)}%",
            ),

            Text(
              "Lat : ${item.latitude.toStringAsFixed(5)}",
            ),

            Text(
              "Lng : ${item.longitude.toStringAsFixed(5)}",
            ),
          ],
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title:
            const Text("Disease Map"),
        centerTitle: true,
      ),

      body: loading
          ? const Center(
              child:
                  CircularProgressIndicator(),
            )
          : Column(
              children: [

                Expanded(
                  flex: 2,

                  child: GoogleMap(

                    initialCameraPosition:
                        initialCamera,

                    myLocationEnabled: true,

                    myLocationButtonEnabled:
                        true,

                    zoomControlsEnabled:
                        true,

                    mapToolbarEnabled: true,

                    markers: _markers,

                    onMapCreated:
                        (controller) {

                      _mapController =
                          controller;

                      loadHistory();
                    },
                  ),
                ),

                Expanded(
                  child: history.isEmpty
                                        ? const Center(
                          child: Text(
                            "No disease detections yet.",
                            style: TextStyle(
                              fontSize: 16,
                              fontWeight: FontWeight.bold,
                            ),
                          ),
                        )
                      : RefreshIndicator(
                          onRefresh: loadHistory,
                          child: ListView.builder(
                            itemCount: history.length,
                            itemBuilder: (context, index) {
                              return buildHistoryCard(
                                history[index],
                              );
                            },
                          ),
                        ),
                ),
              ],
            ),

      floatingActionButton: FloatingActionButton(
        onPressed: loadHistory,
        child: const Icon(Icons.refresh),
      ),
    );
  }
}