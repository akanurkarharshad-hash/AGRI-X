import json
import os
from datetime import datetime


PLOT_FILE = "plots.json"


# =====================================
# SENSOR INPUT LIMITS
# =====================================

# Broad input-sanity limits only.
# These are NOT crop-specific nutrient
# recommendations.

NUTRIENT_MIN = 0.0
NUTRIENT_MAX = 1000.0

PH_MIN = 0.0
PH_MAX = 14.0


# =====================================
# LOAD PLOTS
# =====================================

def load_plots():

    if not os.path.exists(PLOT_FILE):
        return {}

    with open(PLOT_FILE, "r") as file:
        return json.load(file)


# =====================================
# SAVE PLOTS
# =====================================

def save_plots(plots):

    with open(PLOT_FILE, "w") as file:
        json.dump(
            plots,
            file,
            indent=4
        )


# =====================================
# GET NUTRIENT VALUE
# =====================================

def get_nutrient_value(prompt):

    while True:

        value = input(prompt).strip()

        try:
            value = float(value)

        except ValueError:
            print(
                "❌ Please enter a valid number."
            )
            continue

        if value < NUTRIENT_MIN:
            print(
                "❌ Value cannot be negative."
            )
            continue

        if value > NUTRIENT_MAX:
            print(
                f"❌ Value is outside the "
                f"allowed sensor input range "
                f"({NUTRIENT_MIN}–{NUTRIENT_MAX} mg/kg)."
            )
            continue

        return value


# =====================================
# GET pH VALUE
# =====================================

def get_ph_value(prompt):

    while True:

        value = input(prompt).strip()

        try:
            value = float(value)

        except ValueError:
            print(
                "❌ Please enter a valid pH value."
            )
            continue

        if value < PH_MIN or value > PH_MAX:
            print(
                "❌ pH must be between 0 and 14."
            )
            continue

        return value


# =====================================
# ADD SOIL READING
# =====================================

def add_soil_reading(
    plot_id,
    nitrogen,
    phosphorus,
    potassium,
    ph
):

    plots = load_plots()

    # ---------------------------------
    # Check plot
    # ---------------------------------

    if plot_id not in plots:

        return {
            "success": False,
            "message": (
                f"Plot {plot_id} does not exist."
            )
        }

    # ---------------------------------
    # Validate N/P/K
    # ---------------------------------

    nutrient_values = {
        "nitrogen": nitrogen,
        "phosphorus": phosphorus,
        "potassium": potassium
    }

    for parameter, value in nutrient_values.items():

        if not isinstance(value, (int, float)):

            return {
                "success": False,
                "message": (
                    f"{parameter.capitalize()} "
                    "must be numeric."
                )
            }

        if value < NUTRIENT_MIN:

            return {
                "success": False,
                "message": (
                    f"{parameter.capitalize()} "
                    "cannot be negative."
                )
            }

        if value > NUTRIENT_MAX:

            return {
                "success": False,
                "message": (
                    f"{parameter.capitalize()} "
                    "is outside the allowed "
                    "sensor input range."
                )
            }

    # ---------------------------------
    # Validate pH
    # ---------------------------------

    if not isinstance(ph, (int, float)):

        return {
            "success": False,
            "message": "pH must be numeric."
        }

    if ph < PH_MIN or ph > PH_MAX:

        return {
            "success": False,
            "message": (
                "pH must be between 0 and 14."
            )
        }

    # ---------------------------------
    # Generate timestamp
    # ---------------------------------

    timestamp = datetime.now().strftime(
        "%Y-%m-%d %H:%M:%S"
    )

    # ---------------------------------
    # Create reading
    # ---------------------------------

    reading = {

        "nitrogen": float(nitrogen),

        "phosphorus": float(phosphorus),

        "potassium": float(potassium),

        "ph": float(ph),

        "timestamp": timestamp
    }

    # ---------------------------------
    # Add reading
    # ---------------------------------

    plots[plot_id]["readings"].append(
        reading
    )

    # ---------------------------------
    # Save
    # ---------------------------------

    save_plots(plots)

    return {

        "success": True,

        "plot_id": plot_id,

        "reading": reading,

        "total_readings": len(
            plots[plot_id]["readings"]
        )
    }


# =====================================
# INTERACTIVE ADD READING
# =====================================

def add_soil_reading_interactively():

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
        "      🌱 ADD SOIL READING"
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
            f"{plot['crop']} → "
            f"{len(plot['readings'])} readings"
        )

    # ---------------------------------
    # Select plot
    # ---------------------------------

    plot_id = input(
        "\nSelect Plot ID: "
    ).strip().upper()

    if plot_id not in plots:

        print(
            f"\n❌ Plot {plot_id} does not exist."
        )

        return

    plot = plots[plot_id]

    print(
        "\n--------------------------------"
    )

    print(
        f"Plot : {plot['name']}"
    )

    print(
        f"Crop : {plot['crop'].capitalize()}"
    )

    print(
        "--------------------------------"
    )

    # ---------------------------------
    # Enter values
    # ---------------------------------

    print(
        "\nEnter current soil values:"
    )

    print(
        f"N/P/K allowed input range: "
        f"{NUTRIENT_MIN}–{NUTRIENT_MAX} mg/kg"
    )

    print(
        "pH allowed input range: 0–14"
    )

    nitrogen = get_nutrient_value(
        "Nitrogen (mg/kg): "
    )

    phosphorus = get_nutrient_value(
        "Phosphorus (mg/kg): "
    )

    potassium = get_nutrient_value(
        "Potassium (mg/kg): "
    )

    ph = get_ph_value(
        "pH: "
    )

    # ---------------------------------
    # Save reading
    # ---------------------------------

    result = add_soil_reading(

        plot_id=plot_id,

        nitrogen=nitrogen,

        phosphorus=phosphorus,

        potassium=potassium,

        ph=ph
    )

    if not result["success"]:

        print(
            f"\n❌ {result['message']}"
        )

        return

    # =================================
    # SUCCESS
    # =================================

    print(
        "\n================================"
    )

    print(
        "       ✅ READING SAVED"
    )

    print(
        "================================"
    )

    print(
        f"\nPlot ID : "
        f"{result['plot_id']}"
    )

    print(
        f"N       : "
        f"{result['reading']['nitrogen']} mg/kg"
    )

    print(
        f"P       : "
        f"{result['reading']['phosphorus']} mg/kg"
    )

    print(
        f"K       : "
        f"{result['reading']['potassium']} mg/kg"
    )

    print(
        f"pH      : "
        f"{result['reading']['ph']}"
    )

    print(
        f"Time    : "
        f"{result['reading']['timestamp']}"
    )

    print(
        f"\nTotal readings: "
        f"{result['total_readings']}"
    )

    # =================================
    # RUN AI ANALYSIS
    # =================================

    print(
        "\n🔄 Running Plot AI analysis..."
    )

    from plot_ai import analyze_plot

    analyze_plot(plot_id)


# =====================================
# MAIN
# =====================================

if __name__ == "__main__":

    add_soil_reading_interactively()