"""Scientific fertilizer decision-support layer with explicit safety gates."""

from crop_profiles import CROP_PROFILES
from fertilizer_knowledge import FERTILIZER_KNOWLEDGE


RATE_MESSAGE = "Application rate cannot be calculated from the available sensor data alone."


def calculate_fertilizer_rate(**_validated_inputs):
    return {
        "available": False,
        "reason": "Insufficient validated inputs for rate calculation.",
        "message": RATE_MESSAGE,
    }


def _evidence(current_value, analysis):
    return {
        "current_value": current_value,
        "percentage_change": analysis.get("percentage_change"),
        "trend": analysis.get("trend"),
        "historical_trend_confidence": analysis.get("confidence"),
        "readings": analysis.get("number_of_readings", 0),
    }


def _advisory(nutrient, status, evidence, **values):
    knowledge = FERTILIZER_KNOWLEDGE[nutrient]
    result = {
        "nutrient": nutrient,
        "nutrient_name": knowledge["name"],
        "status": status,
        "recommendation_available": False,
        "fertilizer": None,
        "fertilizer_full_name": None,
        "reason": None,
        "scientific_basis": knowledge["scientific_rationale"],
        "evidence": evidence,
        "verification_required": True,
        "verification_message": knowledge["verification_requirement"],
        "application_rate_available": False,
        "rate_message": RATE_MESSAGE,
        "recommendation_confidence": "LOW / VERIFICATION REQUIRED",
        "scientific_rule_used": "REFERENCE REQUIRED",
        "reference": knowledge["reference"],
        "priority": "NORMAL",
        "next_action": "Continue monitoring and use soil-test guidance for fertilizer planning.",
    }
    result.update(values)
    return result


def _nutrient_advisory(nutrient, current, analysis, status_result):
    status = status_result["status"]
    evidence = _evidence(current[nutrient], analysis)
    name = FERTILIZER_KNOWLEDGE[nutrient]["name"]
    if status == "insufficient_data":
        return _advisory(nutrient, status, evidence, reason="More soil readings are required before making a nutrient-based recommendation.", recommendation_confidence="INSUFFICIENT DATA", priority="DATA NEEDED", next_action="Collect additional readings, then confirm nutrient status with a standard soil test.")
    if status == "monitor":
        return _advisory(nutrient, status, evidence, reason="Continue monitoring. Current sensor evidence is insufficient for a fertilizer recommendation.", next_action="Continue monitoring; do not make an immediate fertilizer decision from this sensor evidence.")
    if status == "verify":
        change = evidence["percentage_change"]
        change_text = "an observed variation" if change is None else f"a {abs(change):.2f}% {'increase' if change > 0 else 'decrease' if change < 0 else 'change'}"
        return _advisory(nutrient, status, evidence, reason=f"{name} shows {change_text} across the recorded period. This sensor trend does not confirm deficiency; verify with a standard soil test before fertilizer application.", priority="MEDIUM PRIORITY", next_action=f"Verify the {nutrient} result with a standard soil test before fertilizer application decisions.")
    return _advisory(nutrient, status, evidence, reason="The nutrient trend is currently stable, but no validated soil-test classification is configured to select a fertilizer source.", verification_required=False, verification_message="Continue monitoring and use soil-test guidance for fertilizer planning.", recommendation_confidence="INSUFFICIENT DATA")


def _ph_advisory(crop_key, current, analysis, ph_analysis):
    evidence = _evidence(current["ph"], analysis)
    status = ph_analysis["status"]
    if status == "below_preferred" and crop_key in FERTILIZER_KNOWLEDGE["ph"]["suitable_crops"]:
        source = FERTILIZER_KNOWLEDGE["ph"]["sources"][0]
        return _advisory("ph", status, evidence, recommendation_available=True, fertilizer="lime", fertilizer_full_name=source["name"], reason="Current pH is below the configured crop-specific preferred range. Agricultural lime is a potential amendment, subject to soil-test confirmation.", scientific_rule_used="Configured crop-specific pH range: below preferred range", recommendation_confidence="MEDIUM CONFIDENCE", priority="MEDIUM PRIORITY", next_action="Confirm soil pH and lime requirement with a standard soil test before any amendment decision.")
    if status == "above_preferred":
        return _advisory("ph", status, evidence, reason="Current pH is above the configured crop-specific preferred range. No amendment is recommended because no validated crop-specific rule is configured for this condition.")
    if status == "within_preferred":
        return _advisory("ph", status, evidence, reason="Current pH is within the configured crop-specific preferred range. No pH amendment is recommended.", verification_required=False, verification_message="Continue monitoring.", recommendation_confidence="HIGH CONFIDENCE", scientific_rule_used="Configured crop-specific pH range: within preferred range")
    return _advisory("ph", status, evidence, reason="Crop-specific pH reference is not configured. No pH amendment recommendation will be generated.")


def generate_fertilizer_advisory(crop, current, history_analysis, nutrient_status, ph_analysis):
    crop_key = crop.lower().strip()
    if crop_key not in CROP_PROFILES:
        return {"crop": crop, "advisories": [], "summary": "Crop-specific fertilizer guidance is not configured yet."}
    advisories = [_nutrient_advisory(nutrient, current, history_analysis[nutrient], nutrient_status[nutrient]) for nutrient in ("nitrogen", "phosphorus", "potassium")]
    advisories.append(_ph_advisory(crop_key, current, history_analysis["ph"], ph_analysis))
    available = [item for item in advisories if item["recommendation_available"]]
    verification_count = sum(item["verification_required"] for item in advisories)
    summary = ("Scientific fertilizer or amendment guidance is available with verification requirements."
               if available else
               f"{verification_count} condition{'s' if verification_count != 1 else ''} require verification before fertilizer decisions. The available sensor data alone does not provide enough validated evidence for a fertilizer recommendation.")
    return {"crop": CROP_PROFILES[crop_key]["name"], "advisories": advisories, "summary": summary}
