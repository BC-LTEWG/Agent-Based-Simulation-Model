# Producer-Distributor Transaction Graphs

This file now focuses on **transaction data** between distributors and producers.

## Data Source

- Parsed from `trace.txt` events:
  - `Firm_<id>: reorder - Product <p> <q>` (distributor-side orders)
  - `Producer_<id>: pursued_plan - Product <p> <q>` (accepted producer plans)
- Distributor IDs are filtered to the configured distributor range (100-199).

## Generated Outputs

- `docs/graphs/producer_distributor_transactions.csv`
- `docs/graphs/producer_distributor_transactions_time_series.png`
- `docs/graphs/producer_distributor_transactions_heatmap.png`
- `docs/graphs/producer_distributor_transactions_summary.txt`

## Regenerate

Run:

`python docs/plot_producer_distributor_transactions.py`

Optional (relation diagrams, not transaction data):

`python docs/plot_producer_distributor_relations.py`
