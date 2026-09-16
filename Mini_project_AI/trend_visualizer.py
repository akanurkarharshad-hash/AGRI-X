import json
import os
from datetime import datetime

import matplotlib.pyplot as plt


PLOT_FILE = "plots.json"


# =====================================
# LOAD PLOTS
# =====================================

def load_plots():

    if not os.path.exists(PLOT_FILE):
        return {}

    with open(PLOT_FILE, "r") as file:
        return json.load(file)


# =====================================
# PERCENTAGE CHANGE
# =====================================

def percentage_change(old, new):

    if old == 0:
        return None

    return (
        (new - old) / abs(old)
    ) * 100


# =====================================
# CLASSIFY TREND
# =====================================

def classify_trend(values):

    if len(values) < 2:
        return "insufficient_data"

    changes = []

    for i in range(1, len(values)):

        previous = values[i - 1]
        current = values[i]

        if previous == 0:
            continue

        change = (
            (current - previous)
            / abs(previous)
        ) * 100

        changes.append(change)

    if not changes:
        return "insufficient_data"

    increasing = sum(
        1
        for change in changes
        if change > 2
    )

    decreasing = sum(
        1
        for change in changes
        if change < -2
    )

    stable = sum(
        1
        for change in changes
        if -2 <= change <= 2
    )

    total = len(changes)

    if increasing / total >= 0.70:
        return "increasing"

    if decreasing / total >= 0.70:
        return "decreasing"

    if stable / total >= 0.70:
        return "stable"

    return "fluctuating"


# =====================================
# FORMAT TIMESTAMP
# =====================================

def format_timestamp(timestamp):

    if not timestamp:
        return None

    try:

        dt = datetime.strptime(
            timestamp,
            "%Y-%m-%d %H:%M:%S"
        )

        return dt.strftime(
            "%d-%m %H:%M"
        )

    except (ValueError, TypeError):

        return None


# =====================================
# CREATE X LABELS
# =====================================

def create_x_labels(readings):

    labels = []

    for index, reading in enumerate(
        readings,
        start=1
    ):

        timestamp = reading.get(
            "timestamp"
        )

        formatted = format_timestamp(
            timestamp
        )

        if formatted:

            labels.append(
                formatted
            )

        else:

            labels.append(
                f"Reading {index}"
            )

    return labels


# =====================================
# PRINT TREND SUMMARY
# =====================================

def print_trend_summary(
    readings,
    crop_name
):

    parameters = [
        "nitrogen",
        "phosphorus",
        "potassium",
        "ph"
    ]

    names = {
        "nitrogen": "Nitrogen",
        "phosphorus": "Phosphorus",
        "potassium": "Potassium",
        "ph": "pH"
    }

    print("\n")

    print(
        "=========================================="
    )

    print(
        "             🤖 TREND SUMMARY"
    )

    print(
        "=========================================="
    )

    print(
        f"\nCrop: {crop_name}"
    )

    print(
        f"Readings analyzed: {len(readings)}"
    )

    print(
        "\nTREND STATUS"
    )

    print(
        "────────────────────────────"
    )

    analysis = {}

    for parameter in parameters:

        values = [
            reading[parameter]
            for reading in readings
        ]

        trend = classify_trend(
            values
        )

        first_value = values[0]
        latest_value = values[-1]

        change = percentage_change(
            first_value,
            latest_value
        )

        analysis[parameter] = {
            "trend": trend,
            "change": change
        }

        print(
            f"{names[parameter]:<12}: "
            f"{trend.upper()}"
        )

    # ---------------------------------
    # Find largest change
    # ---------------------------------

    largest_parameter = None
    largest_change = None

    for parameter, data in analysis.items():

        change = data["change"]

        if change is None:
            continue

        if largest_change is None:

            largest_parameter = parameter
            largest_change = change

        elif abs(change) > abs(
            largest_change
        ):

            largest_parameter = parameter
            largest_change = change

    print(
        "\nMOST IMPORTANT CHANGE"
    )

    print(
        "────────────────────────────"
    )

    if largest_parameter is None:

        print(
            "No percentage change available."
        )

    else:

        if largest_change > 0:

            direction = "increased"

        elif largest_change < 0:

            direction = "decreased"

        else:

            direction = "changed"

        print(
            f"{names[largest_parameter]} "
            f"{direction} by "
            f"{abs(largest_change):.2f}%."
        )

    # ---------------------------------
    # AI note
    # ---------------------------------

    print(
        "\nAI NOTE"
    )

    print(
        "────────────────────────────"
    )

    significant_change = any(

        data["change"] is not None
        and abs(data["change"]) >= 50

        for data in analysis.values()
    )

    if significant_change:

        print(
            "Significant variation was detected. "
            "Verify the measurement with additional "
            "readings or a standard soil test before "
            "making fertilizer decisions."
        )

    elif any(
        data["trend"] == "decreasing"
        for data in analysis.values()
    ):

        print(
            "A declining trend was detected. "
            "Continue monitoring and verify important "
            "changes before fertilizer decisions."
        )

    elif any(
        data["trend"] == "fluctuating"
        for data in analysis.values()
    ):

        print(
            "Some parameters are fluctuating. "
            "Continue collecting readings to improve "
            "trend confidence."
        )

    else:

        print(
            "No major trend concern was detected "
            "from the available readings."
        )

    print(
        "=========================================="
    )


# =====================================
# COMBINED N/P/K GRAPH
# =====================================

def plot_npk_trends(
    readings,
    plot_name,
    crop_name
):

    nitrogen = [
        reading["nitrogen"]
        for reading in readings
    ]

    phosphorus = [
        reading["phosphorus"]
        for reading in readings
    ]

    potassium = [
        reading["potassium"]
        for reading in readings
    ]

    x_values = list(
        range(1, len(readings) + 1)
    )

    x_labels = create_x_labels(
        readings
    )

    # ---------------------------------
    # Create figure
    # ---------------------------------

    plt.figure(
        figsize=(12, 7)
    )

    # ---------------------------------
    # Nitrogen
    # ---------------------------------

    plt.plot(
        x_values,
        nitrogen,
        marker="o",
        linewidth=2,
        label="Nitrogen"
    )

    # ---------------------------------
    # Phosphorus
    # ---------------------------------

    plt.plot(
        x_values,
        phosphorus,
        marker="o",
        linewidth=2,
        label="Phosphorus"
    )

    # ---------------------------------
    # Potassium
    # ---------------------------------

    plt.plot(
        x_values,
        potassium,
        marker="o",
        linewidth=2,
        label="Potassium"
    )

    # ---------------------------------
    # Title
    # ---------------------------------

    plt.title(
        f"Soil Nutrient Trends — "
        f"{plot_name} ({crop_name})"
    )

    plt.xlabel(
        "Measurement Time"
    )

    plt.ylabel(
        "Nutrient Concentration (mg/kg)"
    )

    # ---------------------------------
    # X-axis
    # ---------------------------------

    plt.xticks(
        x_values,
        x_labels,
        rotation=45,
        ha="right"
    )

    # ---------------------------------
    # Grid
    # ---------------------------------

    plt.grid(
        True,
        alpha=0.3
    )

    # ---------------------------------
    # Legend
    # ---------------------------------

    plt.legend()

    # ---------------------------------
    # Information
    # ---------------------------------

    nitrogen_change = percentage_change(
        nitrogen[0],
        nitrogen[-1]
    )

    phosphorus_change = percentage_change(
        phosphorus[0],
        phosphorus[-1]
    )

    potassium_change = percentage_change(
        potassium[0],
        potassium[-1]
    )

    information = (

        f"Readings: {len(readings)}\n\n"

        f"N change: "
        f"{nitrogen_change:.2f}%\n"

        f"P change: "
        f"{phosphorus_change:.2f}%\n"

        f"K change: "
        f"{potassium_change:.2f}%"
    )

    plt.text(
        0.02,
        0.97,
        information,
        transform=plt.gca().transAxes,
        verticalalignment="top",
        bbox=dict(
            boxstyle="round",
            facecolor="white",
            alpha=0.8
        )
    )

    plt.tight_layout()

    plt.show()


# =====================================
# pH GRAPH
# =====================================

def plot_ph_trend(
    readings,
    plot_name,
    crop_name
):

    ph_values = [
        reading["ph"]
        for reading in readings
    ]

    x_values = list(
        range(1, len(readings) + 1)
    )

    x_labels = create_x_labels(
        readings
    )

    first_value = ph_values[0]
    latest_value = ph_values[-1]

    change = percentage_change(
        first_value,
        latest_value
    )

    trend = classify_trend(
        ph_values
    )

    if change is None:

        change_text = "N/A"

    else:

        change_text = (
            f"{change:.2f}%"
        )

    # ---------------------------------
    # Create figure
    # ---------------------------------

    plt.figure(
        figsize=(12, 6)
    )

    plt.plot(
        x_values,
        ph_values,
        marker="o",
        linewidth=2,
        label="pH"
    )

    # ---------------------------------
    # Title
    # ---------------------------------

    plt.title(
        f"Soil pH Trend — "
        f"{plot_name} ({crop_name})"
    )

    plt.xlabel(
        "Measurement Time"
    )

    plt.ylabel(
        "pH"
    )

    # ---------------------------------
    # X-axis
    # ---------------------------------

    plt.xticks(
        x_values,
        x_labels,
        rotation=45,
        ha="right"
    )

    # ---------------------------------
    # pH scale
    # ---------------------------------

    plt.ylim(
        0,
        14
    )

    # ---------------------------------
    # Grid
    # ---------------------------------

    plt.grid(
        True,
        alpha=0.3
    )

    plt.legend()

    # ---------------------------------
    # Information
    # ---------------------------------

    information = (

        f"Readings: {len(readings)}\n\n"

        f"First pH: {first_value}\n"

        f"Latest pH: {latest_value}\n"

        f"Change: {change_text}\n"

        f"Trend: {trend}"
    )

    plt.text(
        0.02,
        0.97,
        information,
        transform=plt.gca().transAxes,
        verticalalignment="top",
        bbox=dict(
            boxstyle="round",
            facecolor="white",
            alpha=0.8
        )
    )

    plt.tight_layout()

    plt.show()


# =====================================
# SHOW SOIL TRENDS
# =====================================

def show_soil_trends(plot_id):

    plots = load_plots()

    # ---------------------------------
    # Check plot
    # ---------------------------------

    if plot_id not in plots:

        print(
            f"\n❌ Plot {plot_id} "
            "does not exist."
        )

        return

    plot = plots[plot_id]

    readings = plot.get(
        "readings",
        []
    )

    # ---------------------------------
    # Check readings
    # ---------------------------------

    if not readings:

        print(
            "\n❌ No soil readings "
            "available for this plot."
        )

        print(
            "Add a soil reading first."
        )

        return

    plot_name = plot[
        "name"
    ]

    crop_name = plot[
        "crop"
    ].capitalize()

    # ---------------------------------
    # Header
    # ---------------------------------

    print(
        "\n=========================================="
    )

    print(
        "          📊 SOIL TREND ANALYSIS"
    )

    print(
        "=========================================="
    )

    print(
        f"\nPlot ID : {plot_id}"
    )

    print(
        f"Plot    : {plot_name}"
    )

    print(
        f"Crop    : {crop_name}"
    )

    print(
        f"Readings: {len(readings)}"
    )

    # ---------------------------------
    # AI Summary
    # ---------------------------------

    print_trend_summary(
        readings,
        crop_name
    )

    print(
        "\nOpening combined soil graphs..."
    )

    # =================================
    # GRAPH 1 — N/P/K
    # =================================

    plot_npk_trends(
        readings,
        plot_name,
        crop_name
    )

    # =================================
    # GRAPH 2 — pH
    # =================================

    plot_ph_trend(
        readings,
        plot_name,
        crop_name
    )

    print(
        "\n✅ Soil trend analysis completed."
    )


# =====================================
# SELECT PLOT
# =====================================

def select_plot_for_trends():

    plots = load_plots()

    if not plots:

        print(
            "\n❌ No plots available."
        )

        return

    print(
        "\n================================"
    )

    print(
        "       📊 SOIL TREND ANALYSIS"
    )

    print(
        "================================"
    )

    print(
        "\nAvailable plots:"
    )

    for plot_id, plot in plots.items():

        print(
            f"{plot_id} → "
            f"{plot['name']} → "
            f"{plot['crop'].capitalize()} → "
            f"{len(plot['readings'])} readings"
        )

    plot_id = input(
        "\nSelect Plot ID: "
    ).strip().upper()

    show_soil_trends(
        plot_id
    )


# =====================================
# MAIN
# =====================================

if __name__ == "__main__":

    select_plot_for_trends()