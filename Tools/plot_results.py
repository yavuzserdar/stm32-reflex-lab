"""Plot a monitor CSV. Usage: python Tools/plot_results.py results/session-....csv"""
import argparse
import csv
from pathlib import Path


def load_results(path):
    with Path(path).open(encoding="utf-8-sig", newline="") as stream:
        rows = list(csv.DictReader(stream))
    for row in rows:
        for key in ("session", "round", "reaction_us", "best_us", "average_us", "early", "timeouts"):
            row[key] = int(row[key])
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--save", type=Path, help="Save PNG instead of opening a window")
    args = parser.parse_args()
    rows = load_results(args.csv)
    if not rows:
        parser.error("No completed rounds in this CSV yet.")
    try:
        import matplotlib.pyplot as plt
    except ImportError:
        parser.error("Install plotting support: python -m pip install -r Tools/requirements.txt")
    fig, ax = plt.subplots(figsize=(10, 5))
    # A reset restarts the firmware session counter, so split on round 1 too.
    groups = []
    for row in rows:
        if not groups or row["round"] == 1 or row["session"] != groups[-1][-1]["session"]:
            groups.append([])
        groups[-1].append(row)
    for index, group in enumerate(groups, 1):
        ax.plot([r["round"] for r in group], [r["reaction_us"] / 1000 for r in group],
                marker="o", label=f"Session {index}")
    ax.set(title="STM32 Reflex Lab", xlabel="Valid round", ylabel="Reaction time (ms)", xticks=range(1, 11))
    ax.grid(alpha=0.25)
    ax.legend()
    fig.tight_layout()
    if args.save:
        fig.savefig(args.save, dpi=150)
        print(args.save.resolve())
    else:
        plt.show()


if __name__ == "__main__":
    main()
