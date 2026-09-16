def generate_report(
    plot_id,
    crop,
    current,
    history_analysis,
    ph_analysis,
    alerts,
    nutrient_status,
    crop_decision,
    recommendations,
    fertilizer_advisory=None
):

    report = []

    # =====================================
    # HEADER
    # =====================================

    report.append(
        f"Plot {plot_id} — {crop}"
    )

    # =====================================
    # CURRENT SOIL READING
    # =====================================

    report.append(
        "\nCURRENT SOIL READING"
    )

    report.append(
        f"Nitrogen: {current['nitrogen']} mg/kg"
    )

    report.append(
        f"Phosphorus: {current['phosphorus']} mg/kg"
    )

    report.append(
        f"Potassium: {current['potassium']} mg/kg"
    )

    report.append(
        f"pH: {current['ph']}"
    )

    # =====================================
    # READING TIMELINE
    # =====================================

    report.append(
        "\nREADING TIMELINE"
    )

    timeline = history_analysis.get(
        "_timeline",
        []
    )

    if timeline:

        for item in timeline:

            reading_number = item.get(
                "reading_number",
                "?"
            )

            timestamp = item.get(
                "formatted_timestamp",
                "Date not recorded"
            )

            report.append(
                f"Reading {reading_number} "
                f"→ {timestamp}"
            )

    else:

        report.append(
            "• No reading timeline available."
        )

    # =====================================
    # HISTORICAL TREND
    # =====================================

    report.append(
        "\nHISTORICAL TREND"
    )

    nutrient_names = {

        "nitrogen": "Nitrogen",

        "phosphorus": "Phosphorus",

        "potassium": "Potassium",

        "ph": "pH"
    }

    for parameter, name in nutrient_names.items():

        analysis = history_analysis[
            parameter
        ]

        trend = analysis[
            "trend"
        ]

        change = analysis[
            "percentage_change"
        ]

        confidence = analysis[
            "confidence"
        ]

        readings = analysis[
            "number_of_readings"
        ]

        # ---------------------------------
        # History quality
        # ---------------------------------

        if readings == 0:

            history_quality = (
                "no data"
            )

        elif readings == 1:

            history_quality = (
                "insufficient data"
            )

        elif readings == 2:

            history_quality = (
                "limited history"
            )

        else:

            history_quality = (
                "historical trend available"
            )

        # ---------------------------------
        # Percentage
        # ---------------------------------

        if change is None:

            change_text = "N/A"

        else:

            change_text = (
                f"{change:.2f}%"
            )

        report.append(

            f"{name}: {trend} "

            f"({change_text}) "

            f"| confidence: {confidence}% "

            f"| readings: {readings} "

            f"| {history_quality}"
        )

    # =====================================
    # NUTRIENT STATUS
    # =====================================

    report.append(
        "\nNUTRIENT STATUS"
    )

    for parameter, status in nutrient_status.items():

        report.append(

            f"{parameter.capitalize()}: "

            f"{status['status'].upper()}"
        )

    # =====================================
    # AI OBSERVATIONS
    # =====================================

    report.append(
        "\nAI OBSERVATIONS"
    )

    observation_added = False

    for parameter in [

        "nitrogen",

        "phosphorus",

        "potassium"
    ]:

        analysis = history_analysis[
            parameter
        ]

        trend = analysis[
            "trend"
        ]

        change = analysis[
            "percentage_change"
        ]

        status = nutrient_status[
            parameter
        ]["status"]

        parameter_name = (
            parameter.capitalize()
        )

        # ---------------------------------
        # VERIFY
        # ---------------------------------

        if status == "verify":

            if change is not None:

                if change > 0:

                    direction = "increased"

                elif change < 0:

                    direction = "decreased"

                else:

                    direction = "changed"

                report.append(

                    f"• {parameter_name} "

                    f"{direction} by "

                    f"{abs(change):.2f}% across "

                    "the recorded period. "

                    "Verify the reading before "

                    "making fertilizer decisions."
                )

            else:

                report.append(

                    f"• {parameter_name} requires "

                    "verification before making "

                    "fertilizer decisions."
                )

            observation_added = True

        # ---------------------------------
        # DECREASING
        # ---------------------------------

        elif trend == "decreasing":

            report.append(

                f"• {parameter_name} is showing "

                "a declining trend."
            )

            observation_added = True

        # ---------------------------------
        # FLUCTUATING
        # ---------------------------------

        elif trend == "fluctuating":

            report.append(

                f"• {parameter_name} is fluctuating "

                "across the recorded measurements."
            )

            observation_added = True

        # ---------------------------------
        # INCREASING
        # ---------------------------------

        elif trend == "increasing":

            report.append(

                f"• {parameter_name} is showing "

                "an increasing trend."
            )

            observation_added = True

    if not observation_added:

        report.append(
            "• No current nutrient trend alerts."
        )

    # =====================================
    # pH INTERPRETATION
    # =====================================

    report.append(
        "\nPH INTERPRETATION"
    )

    report.append(
        ph_analysis["message"]
    )

    # =====================================
    # DECISION SUPPORT
    # =====================================

    report.append(
        "\nDECISION SUPPORT"
    )

    report.append(

        f"Overall Status: "

        f"{crop_decision['overall']}"
    )

    for decision in crop_decision[
        "decisions"
    ]:

        report.append(
            f"• {decision}"
        )

    # =====================================
    # FARMER RECOMMENDATIONS
    # =====================================

    report.append(
        "\nRECOMMENDATIONS"
    )

    if recommendations:

        for recommendation in recommendations:

            report.append(
                f"• {recommendation}"
            )

    else:

        report.append(
            "• No specific recommendations "
            "generated."
        )

    report.append("\nFERTILIZER ADVISORY")

    advisories = (fertilizer_advisory or {}).get("advisories", [])
    if not advisories:
        report.append((fertilizer_advisory or {}).get(
            "summary",
            "No fertilizer recommendation is scientifically supported."
        ))
    else:
        report.append((fertilizer_advisory or {}).get("summary", ""))
        for advisory in advisories:
            evidence = advisory["evidence"]
            change = evidence["percentage_change"]
            change_text = "N/A" if change is None else f"{change:.2f}%"
            report.append(f"\nPriority: {advisory['priority']}")
            report.append(f"Nutrient: {advisory['nutrient_name']}")
            if advisory.get("recommendation_available"):
                report.append(f"Suggested fertilizer source: {advisory['fertilizer_full_name']}")
            else:
                report.append("Suggested fertilizer source: Not selected; insufficient validated evidence.")
            report.append(f"Why this assessment: {advisory['reason']}")
            report.append(f"Scientific basis: {advisory['scientific_basis']}")
            report.append(f"Observed evidence: current value {evidence['current_value']}; trend {evidence['trend']}; change {change_text}; readings {evidence['readings']}.")
            report.append(f"Verification: {advisory['verification_message']}")
            report.append(f"Application rate: {advisory['rate_message']}")
            report.append(f"Recommended next action: {advisory['next_action']}")

    # =====================================
    # FINAL NOTE
    # =====================================

    report.append(
        "\nNOTE"
    )

    report.append(
        "Use a standard soil test to confirm "
        "nutrient status before making major "
        "fertilizer decisions."
    )

    # =====================================
    # RETURN FINAL REPORT
    # =====================================

    return "\n".join(report)
