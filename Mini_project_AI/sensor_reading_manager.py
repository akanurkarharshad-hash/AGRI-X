"""
Sensor-specific reading management for ESP32 and other hardware devices.
Extends the base reading model to support validated sensor data.
"""

import json
import os
from datetime import datetime


PLOT_FILE = "plots.json"

# Sensor input validation ranges (documented from ZTS-3002 datasheet)
MOISTURE_MIN = 0.0
MOISTURE_MAX = 100.0  # %

TEMPERATURE_MIN = -40.0
TEMPERATURE_MAX = 80.0  # °C

EC_MIN = 0.0
EC_MAX = 20000.0  # µS/cm

PH_MIN = 3.0
PH_MAX = 9.0

# N/P/K are not validated from the ZTS sensor
# They should be marked as not_validated


def load_plots():
    """Load plots from JSON file."""
    if not os.path.exists(PLOT_FILE):
        return {}
    with open(PLOT_FILE, "r") as file:
        return json.load(file)


def save_plots(plots):
    """Save plots to JSON file."""
    with open(PLOT_FILE, "w") as file:
        json.dump(plots, file, indent=4)


def add_sensor_reading(
    plot_id,
    moisture,
    temperature,
    ec,
    ph,
    device_id=None,
    source="esp32"
):
    """
    Add a reading from an ESP32 or other sensor device.
    
    Parameters:
        plot_id: Plot identifier (e.g., "P01")
        moisture: Soil moisture in % (0-100)
        temperature: Temperature in °C (-40 to 80)
        ec: Electrical conductivity in µS/cm (0-20000)
        ph: pH value (3-9 for documented sensor range)
        device_id: Optional device identifier (e.g., "AGRIX-ESP32-01")
        source: Data source (default "esp32")
    
    Returns:
        Dictionary with success flag and response details
    """
    
    plots = load_plots()

    # =====================================
    # Validate plot exists
    # =====================================

    if plot_id not in plots:
        return {
            "success": False,
            "message": f"Plot {plot_id} does not exist."
        }

    # =====================================
    # Validate sensor inputs
    # =====================================

    sensor_fields = {
        "moisture": (moisture, MOISTURE_MIN, MOISTURE_MAX, "%"),
        "temperature": (temperature, TEMPERATURE_MIN, TEMPERATURE_MAX, "°C"),
        "ec": (ec, EC_MIN, EC_MAX, "µS/cm"),
        "ph": (ph, PH_MIN, PH_MAX, "pH units")
    }

    for field_name, (value, min_val, max_val, unit) in sensor_fields.items():
        if not isinstance(value, (int, float)):
            return {
                "success": False,
                "message": f"{field_name.capitalize()} must be numeric."
            }
        
        if value < min_val or value > max_val:
            return {
                "success": False,
                "message": (
                    f"{field_name.capitalize()} {value} is outside "
                    f"valid range ({min_val}–{max_val} {unit})."
                )
            }

    # =====================================
    # Generate timestamp (server-side)
    # =====================================

    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    # =====================================
    # Create reading with sensor data
    # NPK is set to placeholder values with validation flag
    # =====================================

    reading = {
        "timestamp": timestamp,
        
        # Validated sensor measurements from ZTS-3002
        "moisture": float(moisture),
        "temperature": float(temperature),
        "ec": float(ec),
        "ph": float(ph),
        
        # Placeholder N/P/K (not validated from sensor)
        "nitrogen": 0.0,
        "phosphorus": 0.0,
        "potassium": 0.0,
        
        # Metadata
        "source": source,
        "npk_validation": "not_validated"
    }
    
    # Add device ID if provided
    if device_id:
        reading["device_id"] = device_id

    # =====================================
    # Add reading to plot history
    # =====================================

    if "readings" not in plots[plot_id]:
        plots[plot_id]["readings"] = []

    plots[plot_id]["readings"].append(reading)

    # =====================================
    # Save and return
    # =====================================

    save_plots(plots)

    return {
        "success": True,
        "plot_id": plot_id,
        "reading": reading,
        "total_readings": len(plots[plot_id]["readings"]),
        "message": (
            "Sensor reading recorded. NPK values marked as NOT VALIDATED. "
            "Use soil-test results for nutrient recommendations."
        )
    }


def validate_sensor_reading_payload(data):
    """
    Validate a sensor reading payload from HTTP request.
    
    Expected fields:
        - plot_id: string
        - moisture: float (0-100)
        - temperature: float (-40 to 80)
        - ec: float (0-20000)
        - ph: float (3-9)
        - device_id: string (optional)
        - source: string (default "esp32")
    
    Returns:
        (is_valid, error_message_or_none)
    """
    
    required_fields = ["plot_id", "moisture", "temperature", "ec", "ph"]
    
    for field in required_fields:
        if field not in data:
            return False, f"Missing required field: {field}"
    
    # Validate types
    try:
        plot_id = str(data["plot_id"]).strip().upper()
        moisture = float(data["moisture"])
        temperature = float(data["temperature"])
        ec = float(data["ec"])
        ph = float(data["ph"])
    except (ValueError, TypeError) as e:
        return False, f"Invalid data type: {str(e)}"
    
    # Validate ranges
    if not (MOISTURE_MIN <= moisture <= MOISTURE_MAX):
        return False, f"Moisture {moisture}% out of range ({MOISTURE_MIN}–{MOISTURE_MAX}%)"
    
    if not (TEMPERATURE_MIN <= temperature <= TEMPERATURE_MAX):
        return False, f"Temperature {temperature}°C out of range ({TEMPERATURE_MIN}–{TEMPERATURE_MAX}°C)"
    
    if not (EC_MIN <= ec <= EC_MAX):
        return False, f"EC {ec}µS/cm out of range ({EC_MIN}–{EC_MAX}µS/cm)"
    
    if not (PH_MIN <= ph <= PH_MAX):
        return False, f"pH {ph} out of range ({PH_MIN}–{PH_MAX})"
    
    return True, None
