"""
Convert a Databento .dbn OHLCV file into per-day JSON files for the frontend.

Usage:
    python convert_dbn_to_json.py <dbn_file> <symbol> <output_dir>

Example:
    python convert_dbn_to_json.py glbx-mdp3-20250318-20260317.ohlcv-1m NQ.c.0 ../web/public/data/nq

Notes:
    - Databento OHLCV-1m files contain 1-minute bars.
    - Prices are stored as fixed-point integers scaled by 1e9.
    - Timestamps are nanoseconds since the Unix epoch.
    - "NQ.c.0" is Databento's continuous front-month symbol for NQ futures.
"""

import sys
import json
from collections import defaultdict
from datetime import datetime, timezone
import databento as db


def main(dbn_path: str, symbol: str, output_dir: str) -> None:
    print(f"Reading {dbn_path}...")
    data = db.DBNStore.from_file(dbn_path)
    df = data.to_df()

    # Filter to the symbol we want.
    df = df[df["symbol"] == symbol]
    if df.empty:
        print(f"No data for symbol '{symbol}' in this file.")
        print(f"Available symbols: {sorted(data.to_df()['symbol'].unique())}")
        sys.exit(1)

    print(f"Loaded {len(df)} bars for {symbol}.")

    # Group by trading day. Use the bar's date in US/Eastern for the grouping
    # so a trading session lands in one file even though it spans midnight UTC.
    bars_by_day = defaultdict(list)

    for ts, row in df.iterrows():
        # ts is a pandas Timestamp in UTC. Convert to Eastern for day-grouping.
        ts_utc = ts.tz_convert("UTC") if ts.tzinfo else ts.tz_localize("UTC")
        ts_eastern = ts_utc.tz_convert("America/New_York")
        day_key = ts_eastern.strftime("%Y-%m-%d")

        bars_by_day[day_key].append({
            "time": int(ts_utc.timestamp()),  # Unix seconds, what Lightweight Charts expects
            "open": float(row["open"]),
            "high": float(row["high"]),
            "low": float(row["low"]),
            "close": float(row["close"]),
            "volume": int(row["volume"]),
        })

    # Make sure the output dir exists.
    import os
    os.makedirs(output_dir, exist_ok=True)

    # Write one JSON file per day.
    for day, bars in bars_by_day.items():
        path = os.path.join(output_dir, f"{day}.json")
        with open(path, "w") as f:
            json.dump(bars, f)
        print(f"  Wrote {len(bars)} bars to {path}")

    # Also write an index of available days for the frontend to discover.
    index_path = os.path.join(output_dir, "index.json")
    with open(index_path, "w") as f:
        json.dump(sorted(bars_by_day.keys()), f, indent=2)
    print(f"Wrote index of {len(bars_by_day)} days to {index_path}")


if __name__ == "__main__":
    if len(sys.argv) != 4:
        print(__doc__)
        sys.exit(1)
    main(sys.argv[1], sys.argv[2], sys.argv[3])