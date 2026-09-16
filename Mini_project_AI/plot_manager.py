import json
import os

from crop_profiles import CROP_PROFILES


PLOT_FILE = "plots.json"


def load_plots():
    """Load saved plots from JSON."""

    if not os.path.exists(PLOT_FILE):
        return {}

    with open(PLOT_FILE, "r") as file:
        return json.load(file)


def save_plots(plots):
    """Save plots to JSON."""

    with open(PLOT_FILE, "w") as file:
        json.dump(plots, file, indent=4)


def generate_plot_id(plots):
    """Generate the next plot ID."""

    if not plots:
        return "P01"

    numbers = []

    for plot_id in plots:

        number = int(
            plot_id.replace("P", "")
        )

        numbers.append(number)

    next_number = max(numbers) + 1

    return f"P{next_number:02d}"


def create_plot(plot_name, crop):
    """Create and save a new plot."""

    crop = crop.lower().strip()

    if crop not in CROP_PROFILES:

        return {
            "success": False,
            "message": "Invalid crop."
        }

    plots = load_plots()

    plot_id = generate_plot_id(plots)

    plots[plot_id] = {
        "name": plot_name,
        "crop": crop,
        "readings": []
    }

    save_plots(plots)

    return {
        "success": True,
        "plot_id": plot_id,
        "plot": plots[plot_id]
    }


def show_plots():
    """Display all saved plots."""

    plots = load_plots()

    print("\n================================")
    print("          🌱 MY PLOTS")
    print("================================")

    if not plots:

        print("\nNo plots created yet.")
        return

    for plot_id, plot in plots.items():

        crop_name = CROP_PROFILES[
            plot["crop"]
        ]["name"]

        print(
            f"\n{plot_id}"
            f" | {plot['name']}"
            f" | {crop_name}"
            f" | {len(plot['readings'])} readings"
        )


def create_plot_interactively():

    print("\n================================")
    print("       🌱 CREATE NEW PLOT")
    print("================================")

    plot_name = input(
        "\nEnter plot name: "
    ).strip()

    if not plot_name:

        print("\n❌ Plot name cannot be empty.")
        return

    crops = list(CROP_PROFILES.keys())

    print("\nSelect crop:")

    for index, crop in enumerate(crops, start=1):

        print(
            f"{index}. "
            f"{CROP_PROFILES[crop]['name']}"
        )

    while True:

        choice = input(
            "\nChoose crop number: "
        ).strip()

        if choice.isdigit():

            choice = int(choice)

            if 1 <= choice <= len(crops):

                selected_crop = crops[
                    choice - 1
                ]

                break

        print("❌ Invalid choice. Try again.")

    result = create_plot(
        plot_name,
        selected_crop
    )

    if result["success"]:

        print("\n================================")
        print("      ✅ PLOT CREATED")
        print("================================")

        print(
            f"\nPlot ID : "
            f"{result['plot_id']}"
        )

        print(
            f"Plot    : "
            f"{result['plot']['name']}"
        )

        print(
            f"Crop    : "
            f"{CROP_PROFILES[selected_crop]['name']}"
        )

    else:

        print(
            f"\n❌ {result['message']}"
        )


def main():

    while True:

        print("\n================================")
        print("       🌱 PLOT MANAGER")
        print("================================")

        print("\n1. Create new plot")
        print("2. View my plots")
        print("3. Exit")

        choice = input(
            "\nChoose an option: "
        ).strip()

        if choice == "1":

            create_plot_interactively()

        elif choice == "2":

            show_plots()

        elif choice == "3":

            print("\nGoodbye!")
            break

        else:

            print(
                "\n❌ Invalid option."
            )


if __name__ == "__main__":

    main()