def determine_status(
    parameter,
    current_value,
    trend,
    number_of_readings,
    percentage_change=None
):

    parameter_name = parameter.capitalize()

    # =====================================
    # NOT ENOUGH DATA
    # =====================================

    if number_of_readings < 2:

        return {
            "status": "insufficient_data",

            "message": (
                f"{parameter_name} needs more readings "
                "before trend-based interpretation."
            )
        }

    # =====================================
    # FLUCTUATING
    # =====================================

    if trend == "fluctuating":

        # Large variation
        if (
            percentage_change is not None
            and abs(percentage_change) >= 50
        ):

            return {
                "status": "verify",

                "message": (
                    f"{parameter_name} shows a large "
                    f"variation of "
                    f"{percentage_change:.2f}%. "
                    "Verify the reading and continue "
                    "monitoring."
                )
            }

        # Normal fluctuation
        return {
            "status": "monitor",

            "message": (
                f"{parameter_name} is fluctuating "
                "across the recorded measurements."
            )
        }

    # =====================================
    # DECLINING
    # =====================================

    if trend == "decreasing":

        return {
            "status": "verify",

            "message": (
                f"{parameter_name} shows a declining "
                "trend. Reference soil testing is "
                "recommended before making fertilizer "
                "decisions."
            )
        }

    # =====================================
    # INCREASING
    # =====================================

    if trend == "increasing":

        return {
            "status": "monitor",

            "message": (
                f"{parameter_name} shows an "
                "increasing trend."
            )
        }

    # =====================================
    # STABLE
    # =====================================

    if trend == "stable":

        return {
            "status": "normal",

            "message": (
                f"{parameter_name} is relatively "
                "stable across the recorded "
                "measurements."
            )
        }

    # =====================================
    # FALLBACK
    # =====================================

    return {
        "status": "insufficient_data",

        "message": (
            f"{parameter_name} cannot currently "
            "be interpreted."
        )
    }