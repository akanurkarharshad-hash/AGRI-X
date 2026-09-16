"""Flask web layer for the Agriculture AI decision-support system."""
from __future__ import annotations

from datetime import datetime

from flask import Flask, jsonify, render_template, request

from crop_profiles import CROP_PROFILES
from decision_engine import generate_crop_decision
from field_assessment import build_field_assessment
from fertilizer_advisory import generate_fertilizer_advisory
from history_analysis import analyze_plot_history
from nutrient_status import determine_status
from plot_manager import create_plot, load_plots, save_plots
from reading_manager import add_soil_reading
from recommendation_engine import generate_recommendations
from report_generator import generate_report
from soil_ai import analyze_ph
from sensor_reading_manager import add_sensor_reading, validate_sensor_reading_payload

app = Flask(__name__)
app.config["JSON_SORT_KEYS"] = False


def api_error(message: str, status: int = 400):
    return jsonify({"success": False, "message": message}), status


def format_timestamp(timestamp):
    if not timestamp:
        return "Date not recorded"
    try:
        return datetime.strptime(timestamp, "%Y-%m-%d %H:%M:%S").strftime("%d-%m-%Y %H:%M")
    except (TypeError, ValueError):
        return "Invalid date"


def plot_summary(plot_id, plot):
    readings = plot.get("readings", [])
    latest = readings[-1] if readings else None
    status = "NO DATA"
    if latest:
        status = build_analysis(plot_id, plot)["decision"]["overall"]
    return {
        "id": plot_id, "name": plot.get("name", "Unnamed plot"), "crop": plot["crop"],
        "crop_name": CROP_PROFILES[plot["crop"]]["name"], "reading_count": len(readings),
        "latest_reading": latest, "last_updated": format_timestamp(latest.get("timestamp")) if latest else "No readings", "status": status,
    }


def build_analysis(plot_id, plot):
    readings = plot.get("readings", [])
    if not readings:
        return {"has_data": False, "plot": plot_summary_empty(plot_id, plot), "history": None,
                "nutrient_status": {}, "ph_analysis": None, "decision": {"overall": "NO DATA", "decisions": []},
                "recommendations": [], "fertilizer_advisory": {"advisories": [], "summary": "Add a soil reading before generating fertilizer advisory."}, "alerts": [], "report": None}
    current = readings[-1]
    history = analyze_plot_history(readings)
    ph_analysis = analyze_ph(plot["crop"], current["ph"])
    nutrient_status = {key: determine_status(key, current[key], history[key]["trend"], history[key]["number_of_readings"], history[key]["percentage_change"])
                       for key in ("nitrogen", "phosphorus", "potassium")}
    alerts = []
    for key in ("nitrogen", "phosphorus", "potassium", "ph"):
        trend = history[key]["trend"]
        if trend == "decreasing": alerts.append("Soil pH is showing a declining trend." if key == "ph" else f"{key.capitalize()} is showing a declining trend.")
        elif trend == "fluctuating": alerts.append("Soil pH is fluctuating across the recorded measurements." if key == "ph" else f"{key.capitalize()} is fluctuating across the recorded measurements.")
    if ph_analysis["status"] in ("below_preferred", "above_preferred"):
        alerts.append(ph_analysis["message"])
    crop_name = CROP_PROFILES[plot["crop"]]["name"]
    decision = generate_crop_decision(crop_name, current, history, nutrient_status, ph_analysis)
    recommendations = generate_recommendations(crop_name, history, nutrient_status, ph_analysis)["recommendations"]
    fertilizer_advisory = generate_fertilizer_advisory(plot["crop"], current, history, nutrient_status, ph_analysis)
    report = generate_report(plot_id, crop_name, current, history, ph_analysis, alerts, nutrient_status, decision, recommendations, fertilizer_advisory)
    field_assessment = build_field_assessment(current, history, nutrient_status, ph_analysis, decision, recommendations)
    return {"has_data": True, "plot": plot_summary_empty(plot_id, plot), "current": current, "history": history,
            "nutrient_status": nutrient_status, "ph_analysis": ph_analysis, "decision": decision,
            "recommendations": recommendations, "fertilizer_advisory": fertilizer_advisory, "alerts": alerts, "report": report, "field_assessment": field_assessment}


def plot_summary_empty(plot_id, plot):
    readings = plot.get("readings", [])
    latest = readings[-1] if readings else None
    return {"id": plot_id, "name": plot.get("name", "Unnamed plot"), "crop": plot["crop"],
            "crop_name": CROP_PROFILES[plot["crop"]]["name"], "reading_count": len(readings), "latest_reading": latest,
            "last_updated": format_timestamp(latest.get("timestamp")) if latest else "No readings"}


@app.get("/")
def dashboard():
    return render_template("dashboard.html")


@app.get("/api/crops")
def crops():
    return jsonify([{"key": key, "name": profile["name"]} for key, profile in CROP_PROFILES.items()])


@app.get("/api/plots")
def plots():
    return jsonify([plot_summary(pid, plot) for pid, plot in load_plots().items()])


@app.post("/api/plots")
def add_plot():
    data = request.get_json(silent=True) or {}
    name, crop = str(data.get("name", "")).strip(), str(data.get("crop", "")).strip().lower()
    if not name: return api_error("Plot name cannot be empty.")
    if crop not in CROP_PROFILES: return api_error("Select a valid configured crop.")
    result = create_plot(name, crop)
    if not result["success"]: return api_error(result["message"])
    return jsonify({"success": True, "plot": plot_summary(result["plot_id"], result["plot"])}), 201


@app.get("/api/plots/<plot_id>")
def get_plot(plot_id):
    plot = load_plots().get(plot_id.upper())
    if not plot: return api_error("Plot not found.", 404)
    return jsonify(plot_summary(plot_id.upper(), plot))


@app.get("/api/plots/<plot_id>/analysis")
def analysis(plot_id):
    plot = load_plots().get(plot_id.upper())
    if not plot: return api_error("Plot not found.", 404)
    return jsonify(build_analysis(plot_id.upper(), plot))


@app.post("/api/plots/<plot_id>/readings")
def add_reading(plot_id):
    data = request.get_json(silent=True) or {}
    try:
        values = [float(data[key]) for key in ("nitrogen", "phosphorus", "potassium", "ph")]
    except (KeyError, TypeError, ValueError):
        return api_error("Nitrogen, phosphorus, potassium, and pH must all be valid numbers.")
    result = add_soil_reading(plot_id.upper(), *values)
    if not result["success"]: return api_error(result["message"])
    return jsonify(result), 201


@app.post("/api/plots/<plot_id>/readings/sensor")
def add_sensor_reading_endpoint(plot_id):
    """
    Add a sensor reading from ESP32 or other hardware device.
    
    Expected JSON payload:
    {
        "moisture": float (0-100%),
        "temperature": float (-40 to 80°C),
        "ec": float (0-20000 µS/cm),
        "ph": float (3-9),
        "device_id": string (optional),
        "source": string (default "esp32")
    }
    
    Returns:
        Reading object with npk_validation="not_validated" metadata
    """
    data = request.get_json(silent=True) or {}
    
    # Validate payload
    is_valid, error_msg = validate_sensor_reading_payload(data)
    if not is_valid:
        return api_error(error_msg)
    
    # Extract parameters
    plot_id = plot_id.upper()
    moisture = float(data["moisture"])
    temperature = float(data["temperature"])
    ec = float(data["ec"])
    ph = float(data["ph"])
    device_id = data.get("device_id", None)
    source = data.get("source", "esp32")
    
    # Add sensor reading
    result = add_sensor_reading(
        plot_id=plot_id,
        moisture=moisture,
        temperature=temperature,
        ec=ec,
        ph=ph,
        device_id=device_id,
        source=source
    )
    
    if not result["success"]:
        return api_error(result["message"])
    
    return jsonify(result), 201


@app.delete("/api/plots/<plot_id>/readings/<int:reading_index>")
def delete_reading(plot_id, reading_index):
    """Delete one current reading by its position within the selected plot only."""
    plot_id = plot_id.upper()
    plots = load_plots()
    plot = plots.get(plot_id)
    if not plot:
        return api_error("Plot not found.", 404)
    readings = plot.get("readings", [])
    if reading_index < 0 or reading_index >= len(readings):
        return api_error("This reading no longer exists. Refresh and try again.", 404)
    deleted = readings.pop(reading_index)
    save_plots(plots)
    return jsonify({"success": True, "message": "Reading deleted successfully.", "deleted_reading": deleted,
                    "remaining_readings": len(readings)})


@app.delete("/api/plots/<plot_id>/readings")
def clear_readings(plot_id):
    """Remove all readings while preserving the selected plot and crop."""
    plot_id = plot_id.upper()
    plots = load_plots()
    plot = plots.get(plot_id)
    if not plot:
        return api_error("Plot not found.", 404)
    readings = plot.get("readings", [])
    if not readings:
        return api_error("There are no readings to delete.")
    deleted_count = len(readings)
    plot["readings"] = []
    save_plots(plots)
    return jsonify({"success": True, "message": "All readings deleted successfully.", "deleted_count": deleted_count})


@app.get("/api/plots/<plot_id>/report")
def report(plot_id):
    plot = load_plots().get(plot_id.upper())
    if not plot: return api_error("Plot not found.", 404)
    result = build_analysis(plot_id.upper(), plot)
    if not result["has_data"]: return api_error("Add a soil reading before generating a report.")
    return jsonify({"report": result["report"]})


@app.get("/api/plots/<plot_id>/readings-data")
def readings_data(plot_id):
    """Return raw persisted readings for the charts and timeline."""
    plot = load_plots().get(plot_id.upper())
    if not plot:
        return api_error("Plot not found.", 404)
    return jsonify([
        {**reading, "formatted_timestamp": format_timestamp(reading.get("timestamp"))}
        for reading in plot.get("readings", [])
    ])


if __name__ == "__main__":
    app.run(debug=True)
