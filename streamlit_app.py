import streamlit as st
import sqlite3
import pandas as pd
from datetime import datetime

# ── PAGE CONFIG ───────────────────────────────────────────────────
st.set_page_config(
    page_title="Slope Sentinel Dashboard",
    page_icon="🏔️",
    layout="wide"
)

# ── TITLE ─────────────────────────────────────────────────────────
st.title("🏔️ Slope Sentinel — Live Monitoring Dashboard")
st.caption("APSC 171 — UBCO 2026")

# ── LOAD DATA FROM DATABASE ───────────────────────────────────────
def load_data():
    try:
        conn = sqlite3.connect(
            'C:/Users/Michelle/Downloads/APSC 171 Final Design/slope-sentinel/slope_sentinel.db'
        )
        df = pd.read_sql_query(
            'SELECT * FROM readings ORDER BY timestamp DESC LIMIT 50',
            conn
        )
        conn.close()
        return df
    except Exception as e:
        st.error(f"Database error: {e}")
        return pd.DataFrame()

df = load_data()

# ── STATUS BADGE ──────────────────────────────────────────────────
if not df.empty:
    latest_risk = df.iloc[0]['risk']
    if latest_risk == 'DANGER':
        st.error("🚨 STATUS: DANGER — Slope movement detected!")
    else:
        st.success("✅ STATUS: SAFE — All readings normal")
else:
    st.warning("⚠️ No data yet — waiting for first reading")

st.divider()

# ── METRICS ROW ───────────────────────────────────────────────────
if not df.empty:
    col1, col2, col3 = st.columns(3)
    with col1:
        st.metric("Latest Gyro (deg/sec)", f"{df.iloc[0]['gyro']:.2f}", 
                  help="DANGER if > 15.0")
    with col2:
        st.metric("Latest Moisture", int(df.iloc[0]['moisture']),
                  help="DANGER if > 800")
    with col3:
        st.metric("Last Updated", df.iloc[0]['timestamp'][:19])

st.divider()

# ── CHARTS ────────────────────────────────────────────────────────
if not df.empty:
    chart_df = df[['timestamp', 'gyro', 'moisture']].iloc[::-1].reset_index(drop=True)

    st.subheader("📈 Gyroscope Reading (deg/sec)")
    st.line_chart(chart_df.set_index('timestamp')['gyro'])

    st.subheader("💧 Moisture Reading (0–1023)")
    st.line_chart(chart_df.set_index('timestamp')['moisture'])

    st.divider()

    # ── RAW DATA TABLE ────────────────────────────────────────────
    st.subheader("📋 Recent Readings")
    st.dataframe(df, use_container_width=True)

# ── REFRESH BUTTON ────────────────────────────────────────────────
st.divider()
if st.button("🔄 Refresh Data"):
    st.rerun()