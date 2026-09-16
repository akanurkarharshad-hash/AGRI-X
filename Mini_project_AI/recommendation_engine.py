from crop_profiles import CROP_PROFILES


def get_importance_text(importance):
    """
    Convert nutrient importance information
    into readable text.

    Supports both:

        "high_importance"

    and:

        {
            "importance": "high_importance"
        }
    """

    if isinstance(importance, dict):

        importance = importance.get(
            "importance",
            "important"
        )

    if importance is None:

        importance = "important"

    return str(
        importance
    ).replace(
        "_",
        " "
    )


def generate_recommendations(
    crop,
    history_analysis,
    nutrient_status,
    ph_analysis
):

    crop_key = crop.lower().strip()

    # =====================================
    # CHECK CROP
    # =====================================

    if crop_key not in CROP_PROFILES:

        return {
            "crop": crop,
            "recommendations": [
                "Unsupported crop. "
                "Use a configured crop profile."
            ]
        }

    profile = CROP_PROFILES[crop_key]

    recommendations = []

    # =====================================
    # CROP CONTEXT
    # =====================================

    recommendations.append(
        f"{profile['name']}: use crop-specific "
        "nutrient management and site conditions "
        "when interpreting soil results."
    )

    # =====================================
    # NUTRIENT RECOMMENDATIONS
    # =====================================

    for parameter in [
        "nitrogen",
        "phosphorus",
        "potassium"
    ]:

        status = nutrient_status[
            parameter
        ]["status"]

        analysis = history_analysis[
            parameter
        ]

        importance = profile[
            "nutrients"
        ].get(
            parameter,
            "important"
        )

        importance_text = get_importance_text(
            importance
        )

        parameter_name = parameter.capitalize()

        # =================================
        # INSUFFICIENT DATA
        # =================================

        if status == "insufficient_data":

            recommendations.append(
                f"{parameter_name}: collect more "
                "readings before making a "
                "trend-based decision."
            )

        # =================================
        # VERIFY
        # =================================

        elif status == "verify":

            change = analysis[
                "percentage_change"
            ]

            if change is not None:

                if change > 0:

                    direction = "increased"

                elif change < 0:

                    direction = "decreased"

                else:

                    direction = "changed"

                recommendations.append(
                    f"{parameter_name}: {direction} by "
                    f"{abs(change):.2f}% across the "
                    "recorded period. Because "
                    f"{parameter_name.lower()} has "
                    f"{importance_text} importance "
                    "in this crop, verify the "
                    "measurement with a standard "
                    "soil test before fertilizer "
                    "decisions."
                )

            else:

                recommendations.append(
                    f"{parameter_name}: verify the "
                    "measurement with a standard "
                    "soil test."
                )

        # =================================
        # MONITOR
        # =================================

        elif status == "monitor":

            trend = analysis["trend"]

            if trend == "decreasing":

                recommendations.append(
                    f"{parameter_name}: declining "
                    "trend observed. Continue "
                    "monitoring and verify with "
                    "soil testing if the decline "
                    "continues."
                )

            elif trend == "fluctuating":

                recommendations.append(
                    f"{parameter_name}: fluctuation "
                    "detected. Continue monitoring "
                    "future readings."
                )

            elif trend == "increasing":

                recommendations.append(
                    f"{parameter_name}: increasing "
                    "trend observed. Continue "
                    "monitoring."
                )

        # =================================
        # NORMAL
        # =================================

        elif status == "normal":

            recommendations.append(
                f"{parameter_name}: currently stable "
                "across the recorded readings."
            )

    # =====================================
    # pH RECOMMENDATION
    # =====================================

    ph_status = ph_analysis["status"]

    if ph_status == "below_preferred":

        recommendations.append(
            "pH: below the configured crop "
            "preferred range. Verify soil pH "
            "with a standard soil test before "
            "making corrective decisions."
        )

    elif ph_status == "above_preferred":

        recommendations.append(
            "pH: above the configured crop "
            "preferred range. Verify soil pH "
            "with a standard soil test before "
            "making corrective decisions."
        )

    elif ph_status == "within_preferred":

        recommendations.append(
            "pH: within the configured crop "
            "preferred range. Continue monitoring."
        )

    elif ph_status == "system_specific":

        recommendations.append(
            "pH: interpretation depends on the "
            "crop production system, soil conditions "
            "and water regime."
        )

    elif ph_status == "reference_required":

        recommendations.append(
            "pH: obtain a validated crop-specific "
            "reference before making a pH-based "
            "recommendation."
        )

    # =====================================
    # CROP SOIL NOTES
    # =====================================

    for note in profile.get(
        "soil_notes",
        []
    ):

        recommendations.append(
            f"Crop note: {note}"
        )

    # =====================================
    # SAFETY
    # =====================================

    recommendations.append(
        "Do not make major fertilizer changes "
        "from sensor trends alone. Confirm "
        "important decisions with a standard "
        "soil test and site-specific agronomic "
        "guidance."
    )

    # =====================================
    # FINAL RESULT
    # =====================================

    return {
        "crop": profile["name"],
        "recommendations": recommendations
    }