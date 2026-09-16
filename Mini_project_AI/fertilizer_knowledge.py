"""Traceable, conservative fertilizer knowledge for decision support.

Nutrient-source facts are deliberately separate from soil-test classification
rules. A source must never be selected merely because a sensor value changes.
"""

SOURCE_REFERENCE = (
    "FAO Fertilizer Specifications and Fertilizer Procurement guidance "
    "(typical nutrient grades; local product labels and regulations prevail)."
)

FERTILIZER_KNOWLEDGE = {
    "nitrogen": {
        "name": "Nitrogen",
        "symbol": "N",
        "agronomic_role": "Supports vegetative growth and chlorophyll and protein formation.",
        "sources": [{"name": "Urea", "nutrient_percentage": "46% N"}],
        "suitable_crops": ["wheat", "rice", "cotton", "sugarcane"],
        "scientific_rationale": "A source can be selected only after a validated soil-test class and crop nutrient requirement are available.",
        "conditions_required": ["Validated soil-test nutrient classification", "Crop-specific nutrient requirement", "Local agronomic guidance"],
        "verification_requirement": "Confirm with a standard soil test before fertilizer application.",
        "exact_rate_available": False,
        "soil_test_classification_configured": False,
        "reference": SOURCE_REFERENCE,
    },
    "phosphorus": {
        "name": "Phosphorus",
        "symbol": "P",
        "agronomic_role": "Contributes to energy transfer, root development, and early crop establishment.",
        "sources": [{"name": "Diammonium Phosphate (DAP)", "nutrient_percentage": "18-46-0"}, {"name": "Single Superphosphate (SSP)", "nutrient_percentage": "16-20% P2O5"}],
        "suitable_crops": ["wheat", "rice", "soybean", "cotton", "sugarcane"],
        "scientific_rationale": "A source can be selected only after a validated soil-test class and crop nutrient requirement are available.",
        "conditions_required": ["Validated soil-test nutrient classification", "Crop-specific nutrient requirement", "Local agronomic guidance"],
        "verification_requirement": "Confirm with a standard soil test before fertilizer application.",
        "exact_rate_available": False,
        "soil_test_classification_configured": False,
        "reference": SOURCE_REFERENCE,
    },
    "potassium": {
        "name": "Potassium",
        "symbol": "K",
        "agronomic_role": "Supports water regulation, enzyme activation, and crop development.",
        "sources": [{"name": "Muriate of Potash (MOP)", "nutrient_percentage": "58-62% K2O"}, {"name": "Sulphate of Potash (SOP)", "nutrient_percentage": "48-52% K2O"}],
        "suitable_crops": ["wheat", "rice", "soybean", "cotton", "sugarcane"],
        "scientific_rationale": "A source can be selected only after a validated soil-test class and crop nutrient requirement are available.",
        "conditions_required": ["Validated soil-test nutrient classification", "Crop-specific nutrient requirement", "Local agronomic guidance"],
        "verification_requirement": "Confirm with a standard soil test before fertilizer application.",
        "exact_rate_available": False,
        "soil_test_classification_configured": False,
        "reference": SOURCE_REFERENCE,
    },
    "ph": {
        "name": "Soil pH",
        "symbol": "pH",
        "agronomic_role": "Influences nutrient availability and the suitability of soil conditions for crop growth.",
        "sources": [{"name": "Agricultural lime", "nutrient_percentage": None}],
        "suitable_crops": ["soybean"],
        "scientific_rationale": "A configured crop-specific preferred pH range supports a conditional lime advisory only when current pH is below that range.",
        "conditions_required": ["Configured crop-specific pH range", "Current pH below the configured range", "Standard soil-test confirmation of lime requirement"],
        "verification_requirement": "Confirm soil pH and lime requirement using a standard soil test before application.",
        "exact_rate_available": False,
        "soil_test_classification_configured": True,
        "reference": "FAO guidance notes that lime requirement depends on soil buffering capacity and laboratory testing.",
    },
}
