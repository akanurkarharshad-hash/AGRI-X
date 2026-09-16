def generate_crop_decision(
    crop,
    current,
    history_analysis,
    nutrient_status,
    ph_analysis
):

    decisions = []

    # =====================================
    # NUTRIENT TREND DECISIONS
    # =====================================

    for parameter in [
        "nitrogen",
        "phosphorus",
        "potassium"
    ]:

        analysis = history_analysis[parameter]

        trend = analysis["trend"]

        readings = analysis["number_of_readings"]

        status = nutrient_status[parameter]["status"]

        # ---------------------------------
        # Not enough data
        # ---------------------------------

        if readings < 2:

            decisions.append(
                f"{parameter.capitalize()}: "
                "collect more readings before "
                "making a trend-based decision."
            )

        # ---------------------------------
        # Declining
        # ---------------------------------

        elif trend == "decreasing":

            decisions.append(
                f"{parameter.capitalize()}: "
                "declining trend detected. "
                "Verify with a standard soil test."
            )

        # ---------------------------------
        # Fluctuating
        # ---------------------------------

        elif trend == "fluctuating":

            change = history_analysis[
                parameter
            ]["percentage_change"]

            if change is not None and abs(change) >= 50:

                decisions.append(
                    f"{parameter.capitalize()}: "
                    f"large variation detected "
                    f"({change:.2f}%). "
                    "Verify the reading and "
                    "continue monitoring."
                )

            else:

                decisions.append(
                    f"{parameter.capitalize()}: "
                    "fluctuation detected. "
                    "Continue monitoring."
                )

        # ---------------------------------
        # Increasing
        # ---------------------------------

        elif trend == "increasing":

            decisions.append(
                f"{parameter.capitalize()}: "
                "increasing trend detected. "
                "Continue monitoring."
            )

        # ---------------------------------
        # Stable
        # ---------------------------------

        elif trend == "stable":

            decisions.append(
                f"{parameter.capitalize()}: "
                "stable across recorded readings."
            )

    # =====================================
    # pH DECISION
    # =====================================

    ph_status = ph_analysis["status"]

    if ph_status == "below_preferred":

        decisions.append(
            "pH: below the configured crop "
            "preferred range. Verify soil pH "
            "with a standard soil test."
        )

    elif ph_status == "above_preferred":

        decisions.append(
            "pH: above the configured crop "
            "preferred range. Verify soil pH "
            "with a standard soil test."
        )

    elif ph_status == "within_preferred":

        decisions.append(
            "pH: within the configured crop "
            "preferred range."
        )

    elif ph_status == "system_specific":

        decisions.append(
            "pH: interpretation depends on "
            "the production system."
        )

    else:

        decisions.append(
            "pH: crop-specific reference "
            "requires validation."
        )

       # =====================================
    # OVERALL DECISION
    # =====================================

    attention_required = False
    insufficient_data = False

    for parameter in [
        "nitrogen",
        "phosphorus",
        "potassium"
    ]:

        analysis = history_analysis[parameter]

        trend = analysis["trend"]

        change = analysis["percentage_change"]

        readings = analysis["number_of_readings"]

        # ---------------------------------
        # Insufficient history
        # ---------------------------------

        if readings < 2:

            insufficient_data = True

        # ---------------------------------
        # Declining nutrient
        # ---------------------------------

        elif trend == "decreasing":

            attention_required = True

        # ---------------------------------
        # Large fluctuation
        # ---------------------------------

        elif (
            trend == "fluctuating"
            and change is not None
            and abs(change) >= 50
        ):

            attention_required = True

    # =====================================
    # pH
    # =====================================

    if ph_status in [
        "below_preferred",
        "above_preferred"
    ]:

        attention_required = True

    # =====================================
    # FINAL STATUS
    # =====================================

    if attention_required:

        overall = "ATTENTION"

    elif insufficient_data:

        overall = "INSUFFICIENT DATA"

    else:

        overall = "MONITOR"

    # =====================================
    # FINAL RESULT
    # =====================================

    return {

        "crop": crop,

        "overall": overall,

        "decisions": decisions
    }