from crop_profiles import CROP_PROFILES
from report_generator import generate_report
from history_analysis import analyze_plot_history
from nutrient_status import determine_status
from ph_reference import get_ph_reference

import json


def analyze_ph(crop, ph):

    reference = get_ph_reference(crop)

    # =====================================
    # REFERENCE REQUIRED
    # =====================================

    if reference["status"] == "reference_required":

        return {
            "status": "reference_required",
            "message": (
                "A verified crop-specific pH reference "
                "has not been configured yet."
            )
        }

    # =====================================
    # SYSTEM SPECIFIC
    # =====================================

    if reference["status"] == "system_specific":

        return {
            "status": "system_specific",
            "message": (
                "pH interpretation depends on the "
                "soil and crop production system."
            )
        }

    # =====================================
    # CONFIGURED RANGE
    # =====================================

    if reference["status"] == "configured":

        minimum = reference["minimum"]
        maximum = reference["maximum"]

        if ph < minimum:

            return {
                "status": "below_preferred",
                "message": (
                    f"pH {ph} is below the preferred "
                    f"range of {minimum}-{maximum} "
                    f"for {CROP_PROFILES[crop]['name']}."
                )
            }

        elif ph > maximum:

            return {
                "status": "above_preferred",
                "message": (
                    f"pH {ph} is above the preferred "
                    f"range of {minimum}-{maximum} "
                    f"for {CROP_PROFILES[crop]['name']}."
                )
            }

        else:

            return {
                "status": "within_preferred",
                "message": (
                    f"pH {ph} is within the preferred "
                    f"range of {minimum}-{maximum} "
                    f"for {CROP_PROFILES[crop]['name']}."
                )
            }

    # =====================================
    # UNKNOWN
    # =====================================

    return {
        "status": "reference_required",
        "message": (
            "A verified crop-specific pH reference "
            "has not been configured yet."
        )
    }


# ==========================================
# NUTRIENT PROFILE
# ==========================================

def get_nutrient_profile(crop):

    profile = CROP_PROFILES[crop]

    nutrient_profile = {}

    for parameter, information in profile["nutrients"].items():

        if isinstance(information, dict):

            nutrient_profile[parameter] = {
                "importance": information["importance"],
                "interpretation": information["interpretation"]
            }

        else:

            nutrient_profile[parameter] = {
                "importance": information,
                "interpretation": (
                    "Use crop-specific "
                    "nutrient interpretation."
                )
            }

    return nutrient_profile


# ==========================================
# SOIL ANALYSIS
# ==========================================

def analyze_soil(
    plot_id,
    crop,
    nitrogen,
    phosphorus,
    potassium,
    ph,
    history
):

    crop = crop.lower().strip()

    # =====================================
    # CHECK CROP
    # =====================================

    if crop not in CROP_PROFILES:

        return {
            "success": False,
            "error": f"Unsupported crop: {crop}"
        }

    profile = CROP_PROFILES[crop]

    current_reading = {

        "nitrogen": nitrogen,

        "phosphorus": phosphorus,

        "potassium": potassium,

        "ph": ph
    }

    # =====================================
    # pH ANALYSIS
    # =====================================

    ph_result = analyze_ph(
        crop,
        ph
    )

    # =====================================
    # HISTORICAL ANALYSIS
    # =====================================

    history_result = analyze_plot_history(
        history
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

            status = determine_status(
             parameter=parameter,
             current_value=current_reading[parameter],
             trend=analysis["trend"],
             number_of_readings=analysis["number_of_readings"],
            percentage_change=analysis["percentage_change"]
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
    # OVERALL STATUS
    # =====================================

    if alerts:

        overall_status = "attention"

    else:

        overall_status = "normal"

    # =====================================
    # REPORT
    # =====================================

    report = generate_report(

        plot_id=plot_id,

        crop=profile["name"],

        current=current_reading,

        history_analysis=history_result,

        ph_analysis=ph_result,

        alerts=alerts,

        nutrient_status=nutrient_status,

        crop_decision={
            "overall": overall_status.upper(),
            "decisions": []
        }
    )

    # =====================================
    # FINAL RESULT
    # =====================================

    return {

        "success": True,

        "plot": plot_id,

        "crop": profile["name"],

        "current_reading": current_reading,

        "pH_analysis": ph_result,

        "history_analysis": history_result,

        "nutrient_status": nutrient_status,

        "alerts": alerts,

        "overall_status": overall_status,

        "report": report
    }


# ==========================================
# TEST
# ==========================================

if __name__ == "__main__":

    with open(
        "plots.json",
        "r"
    ) as file:

        data = json.load(file)

    plot = data["P01"]

    readings = plot["readings"]

    if not readings:

        print(
            "\n❌ No readings available "
            "for P01."
        )

    else:

        current = readings[-1]

        result = analyze_soil(

            plot_id="P01",

            crop=plot["crop"],

            nitrogen=current["nitrogen"],

            phosphorus=current["phosphorus"],

            potassium=current["potassium"],

            ph=current["ph"],

            history=readings
        )

        print("\n")

        print(
            "=========================================="
        )

        print(
            "       🌱 AGRICULTURE AI REPORT"
        )

        print(
            "=========================================="
        )

        print(
            f"\nPlot : {result['plot']}"
        )

        print(
            f"Crop : {result['crop']}"
        )

        print(
            "\n--- CURRENT READING ---"
        )

        print(
            f"N  : {current['nitrogen']} mg/kg"
        )

        print(
            f"P  : {current['phosphorus']} mg/kg"
        )

        print(
            f"K  : {current['potassium']} mg/kg"
        )

        print(
            f"pH : {current['ph']}"
        )

        print(
            "\n--- pH ANALYSIS ---"
        )

        print(
            result["pH_analysis"]["message"]
        )

        print(
            "\n--- NUTRIENT STATUS ---"
        )

        for parameter, status in (
            result["nutrient_status"].items()
        ):

            print(
                f"{parameter.capitalize()}: "
                f"{status['status']}"
            )

        print(
            "\n--- HISTORICAL TREND ---"
        )

        for parameter, analysis in (
            result["history_analysis"].items()
        ):

            change = analysis["percentage_change"]

            if change is None:

                change_text = "N/A"

            else:

                change_text = f"{change:.2f}%"

            print(
                f"{parameter.upper():<12}"
                f" {analysis['trend']:<15}"
                f" {change_text}"
            )

        print(
            "\n--- AI ALERTS ---"
        )

        if result["alerts"]:

            for alert in result["alerts"]:

                print(
                    f"⚠ {alert}"
                )

        else:

            print(
                "No current alerts."
            )

        print(
            "\n--- OVERALL STATUS ---"
        )

        print(
            result["overall_status"].upper()
        )

        print(
            "\n=========================================="
        )