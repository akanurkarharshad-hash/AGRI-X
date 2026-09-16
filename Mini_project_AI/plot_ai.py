import json

from crop_profiles import CROP_PROFILES
from history_analysis import analyze_plot_history
from report_generator import generate_report
from nutrient_status import determine_status
from decision_engine import generate_crop_decision
from recommendation_engine import generate_recommendations


PLOT_FILE = "plots.json"


def load_plots():

    with open(PLOT_FILE, "r") as file:
        return json.load(file)


def analyze_plot(plot_id):

    plots = load_plots()

    # =====================================
    # CHECK PLOT
    # =====================================

    if plot_id not in plots:

        print(
            f"\n❌ Plot {plot_id} does not exist."
        )

        return

    plot = plots[plot_id]

    crop = plot["crop"]

    readings = plot["readings"]

    profile = CROP_PROFILES[crop]

    # =====================================
    # CHECK READINGS
    # =====================================

    if not readings:

        print(
            "\n================================"
        )

        print(
            "          🌱 PLOT AI"
        )

        print(
            "================================"
        )

        print(
            f"\nPlot : {plot['name']}"
        )

        print(
            f"Crop : {profile['name']}"
        )

        print(
            "\nℹ️ No soil readings available."
        )

        print(
            "Take a sensor reading first."
        )

        return

    # =====================================
    # CURRENT READING
    # =====================================

    current = readings[-1]

    # =====================================
    # HISTORICAL ANALYSIS
    # =====================================

    history_result = analyze_plot_history(
        readings
    )

    # =====================================
    # pH ANALYSIS
    # =====================================

    from soil_ai import analyze_ph

    ph_result = analyze_ph(
        crop,
        current["ph"]
    )

    # =====================================
    # NUTRIENT STATUS
    # =====================================

    nutrient_status = {}

    for parameter in [
        "nitrogen",
        "phosphorus",
        "potassium"
    ]:

        analysis = history_result[parameter]

        status = determine_status(

            parameter=parameter,

            current_value=current[parameter],

            trend=analysis["trend"],

            number_of_readings=(
                analysis["number_of_readings"]
            ),

            percentage_change=(
                analysis["percentage_change"]
            )
        )

        nutrient_status[parameter] = status

    # =====================================
    # ALERTS
    # =====================================

    alerts = []

    for parameter in [
        "nitrogen",
        "phosphorus",
        "potassium",
        "ph"
    ]:

        analysis = history_result[parameter]

        # ---------------------------------
        # Decreasing
        # ---------------------------------

        if analysis["trend"] == "decreasing":

            if parameter == "ph":

                alerts.append(
                    "Soil pH is showing "
                    "a declining trend."
                )

            else:

                alerts.append(
                    f"{parameter.capitalize()} "
                    "is showing a declining trend."
                )

        # ---------------------------------
        # Fluctuating
        # ---------------------------------

        elif analysis["trend"] == "fluctuating":

            if parameter == "ph":

                alerts.append(
                    "Soil pH is fluctuating "
                    "across the recorded measurements."
                )

            else:

                alerts.append(
                    f"{parameter.capitalize()} "
                    "is fluctuating across "
                    "the recorded measurements."
                )

    # =====================================
    # pH ALERT
    # =====================================

    if ph_result["status"] in [
        "below_preferred",
        "above_preferred"
    ]:

        alerts.append(
            ph_result["message"]
        )

    # =====================================
    # CROP DECISION ENGINE
    # =====================================

    crop_decision = generate_crop_decision(

        crop=profile["name"],

        current=current,

        history_analysis=history_result,

        nutrient_status=nutrient_status,

        ph_analysis=ph_result
    )

    # =====================================
    # RECOMMENDATION ENGINE
    # =====================================

    recommendation_result = generate_recommendations(

        crop=profile["name"],

        history_analysis=history_result,

        nutrient_status=nutrient_status,

        ph_analysis=ph_result
    )

    # =====================================
    # GENERATE FINAL REPORT
    # =====================================

    report = generate_report(

        plot_id=plot_id,

        crop=profile["name"],

        current=current,

        history_analysis=history_result,

        ph_analysis=ph_result,

        alerts=alerts,

        nutrient_status=nutrient_status,

        crop_decision=crop_decision,

        recommendations=(
            recommendation_result[
                "recommendations"
            ]
        )
    )

    # =====================================
    # DISPLAY FINAL REPORT
    # =====================================

    print("\n")

    print(
        "=========================================="
    )

    print(
        "          🌱 PLOT AI REPORT"
    )

    print(
        "=========================================="
    )

    print(report)

    print(
        "\n=========================================="
    )


def select_plot():

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
        "          🌱 PLOT AI"
    )

    print(
        "================================"
    )

    print(
        "\nAvailable plots:"
    )

    for plot_id, plot in plots.items():

        crop_name = CROP_PROFILES[
            plot["crop"]
        ]["name"]

        print(
            f"{plot_id} → "
            f"{plot['name']} → "
            f"{crop_name} → "
            f"{len(plot['readings'])} readings"
        )

    plot_id = input(
        "\nSelect Plot ID: "
    ).strip().upper()

    analyze_plot(plot_id)


if __name__ == "__main__":

    select_plot()