# 1. IMPORTS
from flask import Flask, request
from twilio.rest import Client
import sqlite3
from datetime import datetime

app = Flask(__name__)

# 2. TWILIO CREDENTIALS
account_sid = 'ACa176210631ca26f21ed1791bb903536d'
auth_token  = 'f8fb18f361e21c8a7477917d345b93dd'
client = Client(account_sid, auth_token)

TWILIO_FROM = '+13187064281'   # Your Twilio number
ALERT_TO    = '+17785817518'   # Community contact number

# 3. DATABASE SETUP — runs once on startup
def init_db():
    conn = sqlite3.connect('slope_sentinel.db')
    conn.execute('''CREATE TABLE IF NOT EXISTS readings (
        id        INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp TEXT,
        gyro      REAL,
        moisture  INTEGER,
        risk      TEXT
    )''')
    conn.commit()
    conn.close()

# 4. TWILIO ALERT FUNCTION
def send_alert():
    client.messages.create(
        body='DANGER: Slope movement detected!',
        from_=TWILIO_FROM,
        to=ALERT_TO
    )
    client.calls.create(
        twiml='<Response><Say>Warning: landslide risk detected.</Say></Response>',
        from_=TWILIO_FROM,
        to=ALERT_TO
    )

# 5. FLASK ROUTE — receives data from Heltec
@app.route('/data', methods=['POST'])
def receive_data():
    raw = request.data.decode('utf-8')  # e.g. "GYRO:2.3,MOISTURE:342,RISK:SAFE"

    # Parse the string
    parts  = raw.split(',')
    gyro     = float(parts[0].split(':')[1])
    moisture = int(parts[1].split(':')[1])
    risk     = parts[2].split(':')[1]

    # Save to database
    conn = sqlite3.connect('slope_sentinel.db')
    conn.execute('INSERT INTO readings VALUES (?,?,?,?,?)',
                 (None, datetime.now().isoformat(), gyro, moisture, risk))
    conn.commit()
    conn.close()

    # Fire alert if DANGER
    if risk == 'DANGER':
        send_alert()

    return 'OK', 200

# 6. RUN THE SERVER
if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000, debug=True)