from __future__ import annotations

import csv
import re
from collections import defaultdict, deque
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


STARTING_NUM_PRODUCERS = 100
STARTING_NUM_DISTRIBUTORS = 100
DIST_ID_MIN = STARTING_NUM_PRODUCERS
DIST_ID_MAX = STARTING_NUM_PRODUCERS + STARTING_NUM_DISTRIBUTORS - 1


RE_REORDER = re.compile(
    r"^\[(?P<t>\d+)\]\s+Firm_(?P<firm>\d+):\s+reorder - Product (?P<product>\d+) (?P<qty>\d+)\s*$"
)
RE_PURSUED = re.compile(
    r"^\[(?P<t>\d+)\]\s+Producer_(?P<producer>\d+):\s+pursued_plan - Product (?P<product>\d+) (?P<qty>\d+)\s*$"
)


def parse_trace_transactions(trace_path: Path):
    distributor_reorders = defaultdict(deque)
    producer_pursued = defaultdict(deque)

    with trace_path.open("r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()

            reorder_match = RE_REORDER.match(line)
            if reorder_match:
                firm_id = int(reorder_match.group("firm"))
                if DIST_ID_MIN <= firm_id <= DIST_ID_MAX:
                    key = (
                        int(reorder_match.group("t")),
                        int(reorder_match.group("product")),
                        int(reorder_match.group("qty")),
                    )
                    distributor_reorders[key].append(firm_id)
                continue

            pursued_match = RE_PURSUED.match(line)
            if pursued_match:
                key = (
                    int(pursued_match.group("t")),
                    int(pursued_match.group("product")),
                    int(pursued_match.group("qty")),
                )
                producer_pursued[key].append(int(pursued_match.group("producer")))

    matched = []
    unmatched_reorders = 0
    unmatched_pursued = 0

    all_keys = set(distributor_reorders.keys()) | set(producer_pursued.keys())
    for key in all_keys:
        rq = distributor_reorders.get(key, deque())
        pq = producer_pursued.get(key, deque())
        n = min(len(rq), len(pq))
        t, product, qty = key
        for _ in range(n):
            matched.append(
                {
                    "time_step": t,
                    "product_id": product,
                    "quantity": qty,
                    "distributor_id": rq.popleft(),
                    "producer_id": pq.popleft(),
                }
            )
        unmatched_reorders += len(rq)
        unmatched_pursued += len(pq)

    matched.sort(key=lambda x: (x["time_step"], x["distributor_id"], x["producer_id"], x["product_id"]))
    return matched, unmatched_reorders, unmatched_pursued


def write_transactions_csv(records, out_csv: Path):
    out_csv.parent.mkdir(parents=True, exist_ok=True)
    with out_csv.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=["time_step", "distributor_id", "producer_id", "product_id", "quantity"],
        )
        writer.writeheader()
        for r in records:
            writer.writerow(r)


def plot_time_series(records, out_png: Path):
    qty_by_time = defaultdict(int)
    tx_count_by_time = defaultdict(int)
    for r in records:
        t = r["time_step"]
        qty_by_time[t] += r["quantity"]
        tx_count_by_time[t] += 1

    if not qty_by_time:
        return

    times = sorted(qty_by_time.keys())
    quantities = [qty_by_time[t] for t in times]
    tx_counts = [tx_count_by_time[t] for t in times]

    fig, ax1 = plt.subplots(figsize=(12, 6))
    ax2 = ax1.twinx()

    ax1.plot(times, quantities, color="#2563eb", linewidth=1.8, label="Total quantity")
    ax2.plot(times, tx_counts, color="#dc2626", linewidth=1.4, label="Transaction count")

    ax1.set_title("Distributor-Producer Transactions Over Time (from trace.txt)")
    ax1.set_xlabel("Time step")
    ax1.set_ylabel("Quantity traded", color="#2563eb")
    ax2.set_ylabel("Number of transactions", color="#dc2626")
    ax1.grid(alpha=0.25)

    lines1, labels1 = ax1.get_legend_handles_labels()
    lines2, labels2 = ax2.get_legend_handles_labels()
    ax1.legend(lines1 + lines2, labels1 + labels2, loc="upper right")

    fig.tight_layout()
    fig.savefig(out_png, dpi=220)
    plt.close(fig)


def plot_pair_heatmap(records, out_png: Path, top_n=20):
    qty_by_pair = defaultdict(int)
    dist_totals = defaultdict(int)
    prod_totals = defaultdict(int)

    for r in records:
        d = r["distributor_id"]
        p = r["producer_id"]
        q = r["quantity"]
        qty_by_pair[(d, p)] += q
        dist_totals[d] += q
        prod_totals[p] += q

    if not qty_by_pair:
        return

    top_distributors = [d for d, _ in sorted(dist_totals.items(), key=lambda x: x[1], reverse=True)[:top_n]]
    top_producers = [p for p, _ in sorted(prod_totals.items(), key=lambda x: x[1], reverse=True)[:top_n]]

    d_index = {d: i for i, d in enumerate(top_distributors)}
    p_index = {p: i for i, p in enumerate(top_producers)}
    matrix = np.zeros((len(top_distributors), len(top_producers)))

    for (d, p), q in qty_by_pair.items():
        if d in d_index and p in p_index:
            matrix[d_index[d], p_index[p]] = q

    fig, ax = plt.subplots(figsize=(11, 8))
    im = ax.imshow(matrix, cmap="viridis", aspect="auto")
    fig.colorbar(im, ax=ax, label="Total quantity")

    ax.set_title("Top Distributor-Producer Transaction Quantities")
    ax.set_xlabel("Producer ID (top by quantity)")
    ax.set_ylabel("Distributor ID (top by quantity)")
    ax.set_xticks(np.arange(len(top_producers)))
    ax.set_xticklabels(top_producers, rotation=45, ha="right")
    ax.set_yticks(np.arange(len(top_distributors)))
    ax.set_yticklabels(top_distributors)

    fig.tight_layout()
    fig.savefig(out_png, dpi=220)
    plt.close(fig)


def main():
    root = Path(__file__).resolve().parent.parent
    trace_file = root / "trace.txt"
    out_dir = root / "docs" / "graphs"
    out_dir.mkdir(parents=True, exist_ok=True)

    if not trace_file.exists():
        raise FileNotFoundError(f"Trace file not found: {trace_file}")

    records, unmatched_reorders, unmatched_pursued = parse_trace_transactions(trace_file)

    write_transactions_csv(records, out_dir / "producer_distributor_transactions.csv")
    plot_time_series(records, out_dir / "producer_distributor_transactions_time_series.png")
    plot_pair_heatmap(records, out_dir / "producer_distributor_transactions_heatmap.png")

    summary_file = out_dir / "producer_distributor_transactions_summary.txt"
    with summary_file.open("w", encoding="utf-8") as f:
        f.write(f"matched_transactions={len(records)}\n")
        f.write(f"unmatched_distributor_reorders={unmatched_reorders}\n")
        f.write(f"unmatched_producer_pursued_plans={unmatched_pursued}\n")


if __name__ == "__main__":
    main()
