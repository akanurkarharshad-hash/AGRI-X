# ==========================================
# 🌱 CROP pH REFERENCE ENGINE
# ==========================================

PH_REFERENCES = {

    "wheat": {
        "status": "reference_required",
        "minimum": None,
        "maximum": None,
        "source": None
    },

    "rice": {
        "status": "system_specific",
        "minimum": None,
        "maximum": None,
        "source": None
    },

    "soybean": {
        "status": "configured",
        "minimum": 6.5,
        "maximum": 7.5,
        "source": "Project crop profile"
    },

    "cotton": {
        "status": "reference_required",
        "minimum": None,
        "maximum": None,
        "source": None
    },

    "sugarcane": {
        "status": "reference_required",
        "minimum": None,
        "maximum": None,
        "source": None
    }
}


def get_ph_reference(crop):

    crop = crop.lower().strip()

    if crop not in PH_REFERENCES:

        return {
            "status": "unsupported_crop",
            "minimum": None,
            "maximum": None,
            "source": None
        }

    return PH_REFERENCES[crop]