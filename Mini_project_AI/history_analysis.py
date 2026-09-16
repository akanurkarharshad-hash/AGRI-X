from datetime import datetime


# =====================================
# PERCENTAGE CHANGE
# =====================================

def percentage_change(old, new):

    if old == 0:
        return None

    return (
        (new - old) / old
    ) * 100


# =====================================
# TIMESTAMP FORMATTER
# =====================================

def format_timestamp(timestamp):

    if not timestamp:
        return "Date not recorded"

    try:

        dt = datetime.strptime(
            timestamp,
            "%Y-%m-%d %H:%M:%S"
        )

        return dt.strftime(
            "%d-%m-%Y %H:%M"
        )

    except (ValueError, TypeError):

        return "Invalid date"


# =====================================
# TREND CLASSIFICATION
# =====================================

def classify_trend(values):

    # Need at least 2 readings

    if len(values) < 2:

        return {
            "trend": "insufficient_data",
            "confidence": 0
        }

    changes = []

    # Calculate change between
    # every consecutive reading

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

        return {
            "trend": "insufficient_data",
            "confidence": 0
        }

    # ---------------------------------
    # Count directions
    # ---------------------------------

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

    increasing_ratio = (
        increasing / total
    )

    decreasing_ratio = (
        decreasing / total
    )

    stable_ratio = (
        stable / total
    )

    # ---------------------------------
    # Classify trend
    # ---------------------------------

    if increasing_ratio >= 0.70:

        return {
            "trend": "increasing",
            "confidence": round(
                increasing_ratio * 100
            )
        }

    if decreasing_ratio >= 0.70:

        return {
            "trend": "decreasing",
            "confidence": round(
                decreasing_ratio * 100
            )
        }

    if stable_ratio >= 0.70:

        return {
            "trend": "stable",
            "confidence": round(
                stable_ratio * 100
            )
        }

    return {
        "trend": "fluctuating",
        "confidence": 100
    }


# =====================================
# ANALYZE PLOT HISTORY
# =====================================

def analyze_plot_history(readings):

    parameters = [
        "nitrogen",
        "phosphorus",
        "potassium",
        "ph"
    ]

    result = {}

    # =================================
    # TIMELINE
    # =================================

    timeline = []

    for index, reading in enumerate(
        readings,
        start=1
    ):

        timestamp = reading.get(
            "timestamp"
        )

        timeline.append({
            "reading_number": index,
            "timestamp": timestamp,
            "formatted_timestamp":
                format_timestamp(timestamp)
        })

    # =================================
    # ANALYZE PARAMETERS
    # =================================

    for parameter in parameters:

        values = [
            reading[parameter]
            for reading in readings
        ]

        # ---------------------------------
        # No readings
        # ---------------------------------

        if not values:

            result[parameter] = {

                "first_value": None,

                "latest_value": None,

                "percentage_change": None,

                "trend":
                    "insufficient_data",

                "confidence": 0,

                "number_of_readings": 0,

                "first_timestamp": None,

                "latest_timestamp": None
            }

            continue

        # ---------------------------------
        # Trend
        # ---------------------------------

        trend_result = classify_trend(
            values
        )

        first_value = values[0]

        latest_value = values[-1]

        change = percentage_change(
            first_value,
            latest_value
        )

        # ---------------------------------
        # Timestamps
        # ---------------------------------

        first_timestamp = readings[0].get(
            "timestamp"
        )

        latest_timestamp = readings[-1].get(
            "timestamp"
        )

        # ---------------------------------
        # Store result
        # ---------------------------------

        result[parameter] = {

            "first_value":
                first_value,

            "latest_value":
                latest_value,

            "percentage_change":
                change,

            "trend":
                trend_result["trend"],

            "confidence":
                trend_result["confidence"],

            "number_of_readings":
                len(values),

            "first_timestamp":
                first_timestamp,

            "latest_timestamp":
                latest_timestamp
        }

    # =================================
    # TIMELINE RESULT
    # =================================

    result["_timeline"] = timeline

    return result