"""Presentation adapter for concise, evidence-based dashboard field assessments.

This module does not make agronomic decisions.  It structures the outputs of
the existing history, nutrient-status, pH, decision, and recommendation engines
for display in the web dashboard.
"""


def _priority_for_nutrient(status, trend):
    if status == "verify":
        return "HIGH PRIORITY"
    if status == "insufficient_data" or trend == "insufficient_data":
        return "DATA NEEDED"
    if status == "normal" or trend == "stable":
        return "NORMAL"
    return "MEDIUM PRIORITY"


def _priority_for_ph(status):
    if status in ("below_preferred", "above_preferred"):
        return "HIGH PRIORITY"
    if status in ("reference_required", "unsupported_crop"):
        return "DATA NEEDED"
    if status == "within_preferred":
        return "NORMAL"
    return "MEDIUM PRIORITY"


def _recommendation_for(parameter, recommendations):
    prefix = f"{parameter}:".lower()
    return next((item for item in recommendations if item.lower().startswith(prefix)), None)


def _change_text(analysis):
    change = analysis["percentage_change"]
    if change is None:
        return "No percentage change is available."
    direction = "increased" if change > 0 else "decreased" if change < 0 else "showed no net change"
    return f"{abs(change):.2f}% {direction}" if direction != "showed no net change" else direction


def _nutrient_card(parameter, analysis, status_result, recommendations):
    label = parameter.capitalize()
    readings = analysis["number_of_readings"]
    priority = _priority_for_nutrient(status_result["status"], analysis["trend"])
    if readings < 2:
        observation = f"{readings} reading is available. A historical trend cannot yet be determined."
    else:
        observation = f"{label} {_change_text(analysis)} between the earliest and latest recorded measurements."
    return {
        "priority": priority,
        "parameter": label,
        "status": status_result["status"].replace("_", " ").upper(),
        "observation": observation,
        "interpretation": status_result["message"],
        "action": _recommendation_for(parameter, recommendations) or status_result["message"],
        "rationale": f"Priority is based on the existing nutrient-status result: {status_result['status'].replace('_', ' ')}.",
        "evidence": {
            "readings": readings,
            "first_value": analysis["first_value"],
            "latest_value": analysis["latest_value"],
            "change": analysis["percentage_change"],
            "trend": analysis["trend"],
            "confidence": analysis["confidence"],
        },
    }


def _ph_card(current, analysis, ph_analysis, recommendations):
    status = ph_analysis["status"]
    return {
        "priority": _priority_for_ph(status),
        "parameter": "Soil pH",
        "status": status.replace("_", " ").upper(),
        "observation": f"Current pH is {current['ph']}. {ph_analysis['message']}",
        "interpretation": ph_analysis["message"],
        "action": _recommendation_for("ph", recommendations) or ph_analysis["message"],
        "rationale": f"This assessment uses the existing crop-specific pH interpretation: {status.replace('_', ' ')}.",
        "evidence": {
            "readings": analysis["number_of_readings"],
            "first_value": analysis["first_value"],
            "latest_value": analysis["latest_value"],
            "change": analysis["percentage_change"],
            "trend": analysis["trend"],
            "confidence": analysis["confidence"],
        },
    }


def build_field_assessment(current, history, nutrient_status, ph_analysis, decision, recommendations):
    """Rank existing AI outputs for a concise, auditable dashboard presentation."""
    cards = [
        _nutrient_card(parameter, history[parameter], nutrient_status[parameter], recommendations)
        for parameter in ("nitrogen", "phosphorus", "potassium")
    ]
    cards.append(_ph_card(current, history["ph"], ph_analysis, recommendations))
    ranks = {"HIGH PRIORITY": 0, "MEDIUM PRIORITY": 1, "DATA NEEDED": 2, "NORMAL": 3}
    cards.sort(key=lambda card: ranks[card["priority"]])
    return {
        "overall_status": decision["overall"],
        "overall_decisions": decision["decisions"],
        "cards": cards,
        "prioritized_actions": [
            {"parameter": card["parameter"], "priority": card["priority"], "action": card["action"]}
            for card in cards
            if card["priority"] != "NORMAL"
        ],
    }
