from reading_manager import add_reading


def receive_sensor_reading(
    plot_id,
    nitrogen,
    phosphorus,
    potassium,
    ph
):
    """
    Simulates a reading received from the ESP32.
    """

    print("\n================================")
    print("      📡 SENSOR DATA RECEIVED")
    print("================================")

    print(f"Plot ID : {plot_id}")
    print(f"N       : {nitrogen} mg/kg")
    print(f"P       : {phosphorus} mg/kg")
    print(f"K       : {potassium} mg/kg")
    print(f"pH      : {ph}")

    result = add_reading(
        plot_id=plot_id,
        nitrogen=nitrogen,
        phosphorus=phosphorus,
        potassium=potassium,
        ph=ph
    )

    if result["success"]:

        print("\n✅ Sensor reading accepted.")
        print("✅ Reading saved to plot history.")

    else:

        print("\n❌ Sensor reading rejected.")

        if "errors" in result:

            for error in result["errors"]:

                print(f"• {error}")

        else:

            print(
                f"• {result['message']}"
            )

    return result


# ==========================================
# ESP32 SIMULATION
# ==========================================

if __name__ == "__main__":

    receive_sensor_reading(
        plot_id="P01",
        nitrogen=40,
        phosphorus=28,
        potassium=105,
        ph=6.4
    )
