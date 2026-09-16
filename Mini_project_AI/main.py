from plot_manager import create_plot_interactively
from plot_manager import show_plots

from reading_manager import (
    add_soil_reading_interactively
)

from plot_ai import select_plot

from trend_visualizer import (
    select_plot_for_trends
)


# =====================================
# MAIN MENU
# =====================================

def main():

    while True:

        print("\n")

        print(
            "=========================================="
        )

        print(
            "       🌱 AGRICULTURE AI SYSTEM"
        )

        print(
            "=========================================="
        )

        print(
            "\n1. Create New Plot"
        )

        print(
            "2. Add Soil Reading"
        )

        print(
            "3. Analyze Plot"
        )

        print(
            "4. View My Plots"
        )

        print(
            "5. View Soil Trends"
        )

        print(
            "6. Exit"
        )

        # ---------------------------------
        # USER CHOICE
        # ---------------------------------

        choice = input(
            "\nChoose an option: "
        ).strip()

        # ---------------------------------
        # CREATE PLOT
        # ---------------------------------

        if choice == "1":

            create_plot_interactively()

        # ---------------------------------
        # ADD SOIL READING
        # ---------------------------------

        elif choice == "2":

            add_soil_reading_interactively()

        # ---------------------------------
        # ANALYZE PLOT
        # ---------------------------------

        elif choice == "3":

            select_plot()

        # ---------------------------------
        # VIEW PLOTS
        # ---------------------------------

        elif choice == "4":

            show_plots()

        # ---------------------------------
        # SOIL TRENDS
        # ---------------------------------

        elif choice == "5":

            select_plot_for_trends()

        # ---------------------------------
        # EXIT
        # ---------------------------------

        elif choice == "6":

            print(
                "\n🌱 Thank you for using "
                "Agriculture AI."
            )

            print(
                "Goodbye!"
            )

            break

        # ---------------------------------
        # INVALID
        # ---------------------------------

        else:

            print(
                "\n❌ Invalid option."
            )

            print(
                "Please choose 1, 2, 3, 4, 5 or 6."
            )


# =====================================
# START APPLICATION
# =====================================

if __name__ == "__main__":

    main()