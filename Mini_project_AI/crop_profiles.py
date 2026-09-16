CROP_PROFILES = {

  "wheat": {
    "name": "Wheat",
    "ph_type": "reference_required",

    "nutrients": {
        "nitrogen": {
            "importance": "high",
            "interpretation": "Monitor nitrogen carefully across repeated readings."
        },

        "phosphorus": {
            "importance": "high",
            "interpretation": "Monitor phosphorus across repeated readings."
        },

        "potassium": {
            "importance": "high",
            "interpretation": "Monitor potassium across repeated readings."
        }
    },

    "soil_notes": [
        "Use soil-test-based nutrient interpretation.",
        "Monitor nitrogen trend across repeated readings.",
        "Use site-specific nutrient management."
    ]
},

    "rice": {
        "name": "Rice",
        "ph_type": "system_specific",

"nutrients": {
    "nitrogen": {
        "importance": "high",
        "interpretation": (
            "Monitor nitrogen carefully "
            "across repeated readings."
        )
    },

    "phosphorus": {
        "importance": "high",
        "interpretation": (
            "Monitor phosphorus across "
            "repeated readings."
        )
    },

    "potassium": {
        "importance": "high",
        "interpretation": (
            "Monitor potassium across "
            "repeated readings."
        )
    }
},

        "soil_notes": [
            "Consider soil and water regime.",
            "Monitor nutrient trends.",
            "Use soil-test-based nutrient management."
        ]
    },

    "soybean": {
        "name": "Soybean",

        "ph_type": "preferred_range",
        "ph_range": (6.5, 7.5),

 "nutrients": {
    "nitrogen": {
        "importance": "special",
        "interpretation": (
            "Monitor nitrogen carefully and "
            "interpret alongside crop condition "
            "and soil-test results."
        )
    },

    "phosphorus": {
        "importance": "high",
        "interpretation": (
            "Monitor phosphorus across "
            "repeated readings."
        )
    },

    "potassium": {
        "importance": "high",
        "interpretation": (
            "Monitor potassium across "
            "repeated readings."
        )
    }
},

        "soil_notes": [
            "Prefer well-drained soil.",
            "Near-neutral pH is preferred.",
            "Avoid saline or strongly alkaline conditions.",
            "Avoid poorly drained soil."
        ]
    },

    "cotton": {
        "name": "Cotton",
        "ph_type": "reference_required",

 "nutrients": {
    "nitrogen": {
        "importance": "high",
        "interpretation": (
            "Monitor nitrogen across repeated "
            "readings and use soil-test results "
            "for nutrient decisions."
        )
    },

    "phosphorus": {
        "importance": "important",
        "interpretation": (
            "Monitor phosphorus across repeated "
            "readings."
        )
    },

    "potassium": {
        "importance": "high",
        "interpretation": (
            "Monitor potassium carefully across "
            "repeated readings."
        )
    }
},

        "soil_notes": [
            "Prefer well-drained soil.",
            "Use balanced nutrient management.",
            "Use soil-test-based fertilizer decisions."
        ]
    },

    "sugarcane": {
        "name": "Sugarcane",
        "ph_type": "reference_required",

      "nutrients": {
    "nitrogen": {
        "importance": "high",
        "interpretation": (
            "Monitor nitrogen status across "
            "repeated readings."
        )
    },

    "phosphorus": {
        "importance": "high",
        "interpretation": (
            "Monitor phosphorus across "
            "repeated readings."
        )
    },

    "potassium": {
        "importance": "high",
        "interpretation": (
            "Monitor potassium carefully across "
            "repeated readings."
        )
    }
},

        "soil_notes": [
            "Monitor nutrient status.",
            "Use soil-test-based nutrient management.",
            "Consider soil drainage and soil condition."
        ]
    }
}