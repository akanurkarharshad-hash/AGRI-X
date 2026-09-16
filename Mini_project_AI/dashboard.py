import json
import os
import tkinter as tk
from tkinter import ttk, messagebox

from matplotlib.figure import Figure
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg

from crop_profiles import CROP_PROFILES
from history_analysis import analyze_plot_history
from nutrient_status import determine_status
from decision_engine import generate_crop_decision
from recommendation_engine import generate_recommendations


PLOT_FILE = "plots.json"


# ==========================================
# LOAD PLOTS
# ==========================================

def load_plots():

    if not os.path.exists(PLOT_FILE):
        return {}

    with open(PLOT_FILE, "r") as file:
        return json.load(file)


# ==========================================
# FORMAT TIMESTAMP
# ==========================================

def format_timestamp(timestamp):

    if not timestamp:
        return "Not recorded"

    try:
        from datetime import datetime

        dt = datetime.strptime(
            timestamp,
            "%Y-%m-%d %H:%M:%S"
        )

        return dt.strftime(
            "%d-%m-%Y %H:%M"
        )

    except (ValueError, TypeError):

        return "Not recorded"


# ==========================================
# PERCENTAGE CHANGE
# ==========================================

def percentage_change(old, new):

    if old == 0:
        return None

    return (
        (new - old) / abs(old)
    ) * 100


# ==========================================
# TREND CLASSIFICATION
# ==========================================

def classify_trend(values):

    if len(values) < 2:
        return "insufficient_data"

    changes = []

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
        return "insufficient_data"

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

    if increasing / total >= 0.70:
        return "increasing"

    if decreasing / total >= 0.70:
        return "decreasing"

    if stable / total >= 0.70:
        return "stable"

    return "fluctuating"


# ==========================================
# DASHBOARD
# ==========================================

class AgricultureDashboard:

    def __init__(self, root):

        self.root = root

        self.root.title(
            "🌱 Agriculture AI Dashboard"
        )

        self.root.geometry(
            "1250x800"
        )

        self.root.minsize(
            1050,
            700
        )

        self.plots = load_plots()

        self.current_plot = None

        self.setup_style()

        self.create_interface()

        self.load_plot_list()


    # ======================================
    # STYLE
    # ======================================

    def setup_style(self):

        style = ttk.Style()

        try:
            style.theme_use(
                "clam"
            )
        except tk.TclError:
            pass

        style.configure(
            "Title.TLabel",
            font=(
                "Segoe UI",
                22,
                "bold"
            )
        )

        style.configure(
            "Subtitle.TLabel",
            font=(
                "Segoe UI",
                11
            )
        )

        style.configure(
            "CardTitle.TLabel",
            font=(
                "Segoe UI",
                13,
                "bold"
            )
        )

        style.configure(
            "Value.TLabel",
            font=(
                "Segoe UI",
                16,
                "bold"
            )
        )


    # ======================================
    # MAIN INTERFACE
    # ======================================

    def create_interface(self):

        # ----------------------------------
        # Header
        # ----------------------------------

        header = ttk.Frame(
            self.root,
            padding=15
        )

        header.pack(
            fill="x"
        )

        ttk.Label(
            header,
            text="🌱 AGRICULTURE AI",
            style="Title.TLabel"
        ).pack(
            side="left"
        )

        ttk.Label(
            header,
            text="Soil Monitoring & Decision Support",
            style="Subtitle.TLabel"
        ).pack(
            side="left",
            padx=20
        )

        # ----------------------------------
        # Plot selection
        # ----------------------------------

        selection_frame = ttk.LabelFrame(
            self.root,
            text="Select Plot",
            padding=10
        )

        selection_frame.pack(
            fill="x",
            padx=15,
            pady=5
        )

        ttk.Label(
            selection_frame,
            text="Plot:"
        ).pack(
            side="left"
        )

        self.plot_combo = ttk.Combobox(
            selection_frame,
            state="readonly",
            width=45
        )

        self.plot_combo.pack(
            side="left",
            padx=10
        )

        self.plot_combo.bind(
            "<<ComboboxSelected>>",
            self.on_plot_selected
        )

        ttk.Button(
            selection_frame,
            text="🔄 Refresh",
            command=self.refresh_dashboard
        ).pack(
            side="left",
            padx=10
        )

        # ----------------------------------
        # Plot information
        # ----------------------------------

        info_frame = ttk.Frame(
            self.root
        )

        info_frame.pack(
            fill="x",
            padx=15,
            pady=5
        )

        self.plot_label = ttk.Label(
            info_frame,
            text="Plot: --",
            style="CardTitle.TLabel"
        )

        self.plot_label.pack(
            side="left",
            padx=10
        )

        self.crop_label = ttk.Label(
            info_frame,
            text="Crop: --",
            style="CardTitle.TLabel"
        )

        self.crop_label.pack(
            side="left",
            padx=30
        )

        self.status_label = ttk.Label(
            info_frame,
            text="Status: --",
            style="CardTitle.TLabel"
        )

        self.status_label.pack(
            side="right",
            padx=10
        )

        # ----------------------------------
        # Current values
        # ----------------------------------

        values_frame = ttk.Frame(
            self.root
        )

        values_frame.pack(
            fill="x",
            padx=15,
            pady=5
        )

        self.n_value = self.create_value_card(
            values_frame,
            "Nitrogen",
            "N"
        )

        self.p_value = self.create_value_card(
            values_frame,
            "Phosphorus",
            "P"
        )

        self.k_value = self.create_value_card(
            values_frame,
            "Potassium",
            "K"
        )

        self.ph_value = self.create_value_card(
            values_frame,
            "Soil pH",
            "pH"
        )

        # ----------------------------------
        # Graph area
        # ----------------------------------

        graph_frame = ttk.Frame(
            self.root
        )

        graph_frame.pack(
            fill="both",
            expand=True,
            padx=15,
            pady=5
        )

        # NPK graph

        npk_frame = ttk.LabelFrame(
            graph_frame,
            text="📈 N / P / K Trends",
            padding=5
        )

        npk_frame.pack(
            side="left",
            fill="both",
            expand=True,
            padx=(0, 5)
        )

        self.npk_figure = Figure(
            figsize=(5, 3),
            dpi=90
        )

        self.npk_ax = self.npk_figure.add_subplot(
            111
        )

        self.npk_canvas = FigureCanvasTkAgg(
            self.npk_figure,
            master=npk_frame
        )

        self.npk_canvas.get_tk_widget().pack(
            fill="both",
            expand=True
        )

        # pH graph

        ph_frame = ttk.LabelFrame(
            graph_frame,
            text="📊 pH Trend",
            padding=5
        )

        ph_frame.pack(
            side="right",
            fill="both",
            expand=True,
            padx=(5, 0)
        )

        self.ph_figure = Figure(
            figsize=(5, 3),
            dpi=90
        )

        self.ph_ax = self.ph_figure.add_subplot(
            111
        )

        self.ph_canvas = FigureCanvasTkAgg(
            self.ph_figure,
            master=ph_frame
        )

        self.ph_canvas.get_tk_widget().pack(
            fill="both",
            expand=True
        )

        # ----------------------------------
        # Bottom information
        # ----------------------------------

        bottom_frame = ttk.Frame(
            self.root
        )

        bottom_frame.pack(
            fill="both",
            expand=False,
            padx=15,
            pady=5
        )

        # AI summary

        ai_frame = ttk.LabelFrame(
            bottom_frame,
            text="🤖 AI ANALYSIS",
            padding=8
        )

        ai_frame.pack(
            side="left",
            fill="both",
            expand=True,
            padx=(0, 5)
        )

        self.ai_text = tk.Text(
            ai_frame,
            height=8,
            wrap="word",
            font=(
                "Segoe UI",
                10
            ),
            state="disabled"
        )

        self.ai_text.pack(
            fill="both",
            expand=True
        )

        # Recommendations

        recommendation_frame = ttk.LabelFrame(
            bottom_frame,
            text="💡 RECOMMENDATIONS",
            padding=8
        )

        recommendation_frame.pack(
            side="right",
            fill="both",
            expand=True,
            padx=(5, 0)
        )

        self.recommendation_text = tk.Text(
            recommendation_frame,
            height=8,
            wrap="word",
            font=(
                "Segoe UI",
                10
            ),
            state="disabled"
        )

        self.recommendation_text.pack(
            fill="both",
            expand=True
        )


    # ======================================
    # VALUE CARD
    # ======================================

    def create_value_card(
        self,
        parent,
        title,
        symbol
    ):

        frame = ttk.LabelFrame(
            parent,
            text=f"{symbol}  {title}",
            padding=10
        )

        frame.pack(
            side="left",
            fill="x",
            expand=True,
            padx=5
        )

        label = ttk.Label(
            frame,
            text="--",
            style="Value.TLabel"
        )

        label.pack()

        return label


    # ======================================
    # LOAD PLOT LIST
    # ======================================

    def load_plot_list(self):

        self.plot_combo["values"] = []

        values = []

        for plot_id, plot in self.plots.items():

            crop_name = CROP_PROFILES[
                plot["crop"]
            ]["name"]

            values.append(
                f"{plot_id} → "
                f"{plot['name']} → "
                f"{crop_name}"
            )

        self.plot_combo["values"] = values

        if values:

            self.plot_combo.current(0)

            self.on_plot_selected()


    # ======================================
    # SELECTED PLOT
    # ======================================

    def get_selected_plot_id(self):

        selected = self.plot_combo.get()

        if not selected:
            return None

        return selected.split(
            " → "
        )[0]


    # ======================================
    # PLOT SELECTED
    # ======================================

    def on_plot_selected(
        self,
        event=None
    ):

        plot_id = self.get_selected_plot_id()

        if plot_id:

            self.update_dashboard(
                plot_id
            )


    # ======================================
    # REFRESH
    # ======================================

    def refresh_dashboard(self):

        self.plots = load_plots()

        self.load_plot_list()

        plot_id = self.get_selected_plot_id()

        if plot_id:

            self.update_dashboard(
                plot_id
            )


    # ======================================
    # UPDATE DASHBOARD
    # ======================================

    def update_dashboard(
        self,
        plot_id
    ):

        if plot_id not in self.plots:

            messagebox.showerror(
                "Error",
                "Selected plot does not exist."
            )

            return

        plot = self.plots[
            plot_id
        ]

        readings = plot.get(
            "readings",
            []
        )

        self.current_plot = plot_id

        crop = plot[
            "crop"
        ]

        crop_name = CROP_PROFILES[
            crop
        ]["name"]

        # ----------------------------------
        # No readings
        # ----------------------------------

        if not readings:

            self.plot_label.config(
                text=f"Plot: {plot['name']}"
            )

            self.crop_label.config(
                text=f"Crop: {crop_name}"
            )

            self.status_label.config(
                text="Status: NO DATA"
            )

            self.n_value.config(
                text="--"
            )

            self.p_value.config(
                text="--"
            )

            self.k_value.config(
                text="--"
            )

            self.ph_value.config(
                text="--"
            )

            self.clear_graphs()

            self.set_text(
                self.ai_text,
                "No soil readings available.\n\n"
                "Take a sensor reading first."
            )

            self.set_text(
                self.recommendation_text,
                "Add soil readings to generate "
                "AI recommendations."
            )

            return

        # ----------------------------------
        # Current reading
        # ----------------------------------

        current = readings[-1]

        self.plot_label.config(
            text=f"Plot: {plot['name']}"
        )

        self.crop_label.config(
            text=f"Crop: {crop_name}"
        )

        # ----------------------------------
        # Current values
        # ----------------------------------

        self.n_value.config(
            text=f"{current['nitrogen']} mg/kg"
        )

        self.p_value.config(
            text=f"{current['phosphorus']} mg/kg"
        )

        self.k_value.config(
            text=f"{current['potassium']} mg/kg"
        )

        self.ph_value.config(
            text=f"{current['ph']}"
        )

        # ----------------------------------
        # History analysis
        # ----------------------------------

        history_result = analyze_plot_history(
            readings
        )

        # ----------------------------------
        # pH analysis
        # ----------------------------------

        from soil_ai import analyze_ph

        ph_result = analyze_ph(
            crop,
            current["ph"]
        )

        # ----------------------------------
        # Nutrient status
        # ----------------------------------

        nutrient_status = {}

        for parameter in [
            "nitrogen",
            "phosphorus",
            "potassium"
        ]:

            analysis = history_result[
                parameter
            ]

            nutrient_status[
                parameter
            ] = determine_status(
                parameter=parameter,
                current_value=current[
                    parameter
                ],
                trend=analysis[
                    "trend"
                ],
                number_of_readings=analysis[
                    "number_of_readings"
                ],
                percentage_change=analysis[
                    "percentage_change"
                ]
            )

        # ----------------------------------
        # Decision engine
        # ----------------------------------

        crop_decision = generate_crop_decision(
            crop=crop_name,
            current=current,
            history_analysis=history_result,
            nutrient_status=nutrient_status,
            ph_analysis=ph_result
        )

        # ----------------------------------
        # Recommendations
        # ----------------------------------

        recommendations = generate_recommendations(
            crop=crop_name,
            history_analysis=history_result,
            nutrient_status=nutrient_status,
            ph_analysis=ph_result
        )

        # ----------------------------------
        # Status
        # ----------------------------------

        overall = crop_decision[
            "overall"
        ]

        self.status_label.config(
            text=f"Status: {overall}"
        )

        # ----------------------------------
        # Graphs
        # ----------------------------------

        self.draw_npk_graph(
            readings,
            plot["name"],
            crop_name
        )

        self.draw_ph_graph(
            readings,
            plot["name"],
            crop_name
        )

        # ----------------------------------
        # AI analysis
        # ----------------------------------

        self.display_ai_analysis(
            history_result,
            nutrient_status,
            ph_result,
            crop_decision,
            readings
        )

        # ----------------------------------
        # Recommendations
        # ----------------------------------

        self.display_recommendations(
            recommendations
        )


    # ======================================
    # NPK GRAPH
    # ======================================

    def draw_npk_graph(
        self,
        readings,
        plot_name,
        crop_name
    ):

        self.npk_ax.clear()

        nitrogen = [
            reading["nitrogen"]
            for reading in readings
        ]

        phosphorus = [
            reading["phosphorus"]
            for reading in readings
        ]

        potassium = [
            reading["potassium"]
            for reading in readings
        ]

        x = list(
            range(1, len(readings) + 1)
        )

        labels = []

        for index, reading in enumerate(
            readings,
            start=1
        ):

            timestamp = reading.get(
                "timestamp"
            )

            formatted = format_timestamp(
                timestamp
            )

            if formatted != "Not recorded":

                labels.append(
                    formatted
                )

            else:

                labels.append(
                    f"R{index}"
                )

        self.npk_ax.plot(
            x,
            nitrogen,
            marker="o",
            linewidth=2,
            label="Nitrogen"
        )

        self.npk_ax.plot(
            x,
            phosphorus,
            marker="o",
            linewidth=2,
            label="Phosphorus"
        )

        self.npk_ax.plot(
            x,
            potassium,
            marker="o",
            linewidth=2,
            label="Potassium"
        )

        self.npk_ax.set_title(
            "N / P / K Soil Trends"
        )

        self.npk_ax.set_ylabel(
            "mg/kg"
        )

        self.npk_ax.set_xticks(
            x
        )

        self.npk_ax.set_xticklabels(
            labels,
            rotation=35,
            ha="right",
            fontsize=8
        )

        self.npk_ax.grid(
            True,
            alpha=0.3
        )

        self.npk_ax.legend(
            fontsize=8
        )

        self.npk_figure.tight_layout()

        self.npk_canvas.draw()


    # ======================================
    # pH GRAPH
    # ======================================

    def draw_ph_graph(
        self,
        readings,
        plot_name,
        crop_name
    ):

        self.ph_ax.clear()

        values = [
            reading["ph"]
            for reading in readings
        ]

        x = list(
            range(1, len(values) + 1)
        )

        labels = []

        for index, reading in enumerate(
            readings,
            start=1
        ):

            timestamp = reading.get(
                "timestamp"
            )

            formatted = format_timestamp(
                timestamp
            )

            if formatted != "Not recorded":

                labels.append(
                    formatted
                )

            else:

                labels.append(
                    f"R{index}"
                )

        self.ph_ax.plot(
            x,
            values,
            marker="o",
            linewidth=2,
            label="pH"
        )

        self.ph_ax.set_title(
            "Soil pH Trend"
        )

        self.ph_ax.set_ylabel(
            "pH"
        )

        self.ph_ax.set_ylim(
            0,
            14
        )

        self.ph_ax.set_xticks(
            x
        )

        self.ph_ax.set_xticklabels(
            labels,
            rotation=35,
            ha="right",
            fontsize=8
        )

        self.ph_ax.grid(
            True,
            alpha=0.3
        )

        self.ph_ax.legend(
            fontsize=8
        )

        self.ph_figure.tight_layout()

        self.ph_canvas.draw()


    # ======================================
    # CLEAR GRAPHS
    # ======================================

    def clear_graphs(self):

        self.npk_ax.clear()

        self.ph_ax.clear()

        self.npk_ax.set_title(
            "No N / P / K data"
        )

        self.ph_ax.set_title(
            "No pH data"
        )

        self.npk_canvas.draw()

        self.ph_canvas.draw()


    # ======================================
    # AI ANALYSIS DISPLAY
    # ======================================

    def display_ai_analysis(
        self,
        history_result,
        nutrient_status,
        ph_result,
        crop_decision,
        readings
    ):

        lines = []

        lines.append(
            f"Overall Status: "
            f"{crop_decision['overall']}"
        )

        lines.append("")

        lines.append(
            "NUTRIENT STATUS"
        )

        for parameter in [
            "nitrogen",
            "phosphorus",
            "potassium"
        ]:

            status = nutrient_status[
                parameter
            ]["status"].upper()

            lines.append(
                f"{parameter.capitalize()}: "
                f"{status}"
            )

        lines.append("")

        lines.append(
            "TRENDS"
        )

        for parameter in [
            "nitrogen",
            "phosphorus",
            "potassium",
            "ph"
        ]:

            analysis = history_result[
                parameter
            ]

            lines.append(
                f"{parameter.upper()}: "
                f"{analysis['trend'].upper()}"
            )

        lines.append("")

        lines.append(
            "pH INTERPRETATION"
        )

        lines.append(
            ph_result["message"]
        )

        lines.append("")

        lines.append(
            "DECISION SUPPORT"
        )

        for decision in crop_decision[
            "decisions"
        ]:

            lines.append(
                f"• {decision}"
            )

        self.set_text(
            self.ai_text,
            "\n".join(lines)
        )


    # ======================================
    # RECOMMENDATIONS DISPLAY
    # ======================================

    def display_recommendations(
        self,
        recommendations
    ):

        if not recommendations:

            text = (
                "No specific recommendations "
                "generated."
            )

        else:

            text = "\n".join(
                f"• {recommendation}"
                for recommendation
                in recommendations
            )

        self.set_text(
            self.recommendation_text,
            text
        )


    # ======================================
    # TEXT HELPER
    # ======================================

    def set_text(
        self,
        widget,
        text
    ):

        widget.config(
            state="normal"
        )

        widget.delete(
            "1.0",
            tk.END
        )

        widget.insert(
            tk.END,
            text
        )

        widget.config(
            state="disabled"
        )


# ==========================================
# MAIN
# ==========================================

if __name__ == "__main__":

    root = tk.Tk()

    app = AgricultureDashboard(
        root
    )

    root.mainloop()