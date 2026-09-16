import json

PLOT_FILE = "plots.json"


def reset_readings():

    with open(PLOT_FILE, "r") as file:
        plots = json.load(file)

    print("\n================================")
    print("       🧹 RESET READINGS")
    print("================================")

    print("\nAvailable plots:")

    for plot_id, plot in plots.items():

        print(
            f"{plot_id} → "
            f"{plot['name']} → "
            f"{plot['crop']} → "
            f"{len(plot['readings'])} readings"
        )

    plot_id = input(
        "\nEnter Plot ID to reset: "
    ).strip().upper()

    if plot_id not in plots:

        print("\n❌ Plot does not exist.")

        return

    count = len(
        plots[plot_id]["readings"]
    )

    if count == 0:

        print(
            "\nℹ️ This plot already has no readings."
        )

        return

    confirmation = input(
        f"\nDelete {count} readings from "
        f"{plot_id}? (yes/no): "
    ).strip().lower()

    if confirmation != "yes":

        print("\n❌ Reset cancelled.")

        return

    plots[plot_id]["readings"] = []

    with open(PLOT_FILE, "w") as file:

        json.dump(
            plots,
            file,
            indent=4
        )

    print(
        f"\n✅ Removed {count} readings "
        f"from {plot_id}."
    )


if __name__ == "__main__":

    reset_readings()