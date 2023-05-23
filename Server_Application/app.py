from flask import Flask, render_template, request,url_for, redirect
import serial, os,pytz
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime, timedelta
from sqlalchemy import Sequence



utc_plus_eight = pytz.timezone('Asia/Singapore')  # Replace 'Asia/Singapore' with the appropriate timezone identifier

basedir = os.path.abspath(os.path.dirname(__file__))
app = Flask(__name__)
# ser = serial.Serial('COM7', 9600)

# create the extension
db = SQLAlchemy()
# db.dropall()
# create the app
app = Flask(__name__)
# configure the SQLite database, relative to the app instance folder
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///project.db"
# initialize the app with the extension
db.init_app(app)

class Records(db.Model):
    #device id
    readingid = db.Column(db.Integer, Sequence('reading_id_seq'), primary_key=True)
    deviceid = db.Column(db.Integer)
    timestamp = db.Column(db.DateTime(timezone=True), default=lambda: datetime.now(utc_plus_eight))
    reading = db.Column(db.String, nullable = False)

    def __repr__(self):
        return f'<Device {self.deviceid}: {self.reading} at {self.timestamp}, readingID = {self.readingid} >'

with app.app_context():
    db.create_all()


# sample initial data; can be removed later but keep array
devices = [
    {
        'Timestamps': ["0", "1"], 
        'WaterLevels': ["Low", "Moderate"],
    }
]

@app.route('/arduino', methods=['POST'])
def arduino():
    data = request.form.get('data')
    ser.write(data.encode())
    response = ser.readline().decode().strip()
    return response

@app.route('/', methods=['GET', 'POST'])
def index():
    global data  # Use the global data variable

    if request.method == 'POST':
        new_data = request.get_json()
        # Input format: { 'DeviceID': '', 'WaterLevel': ''}
        if new_data:
            # raw_date =datetime.utcnow() # Obtaining datetime
            # rounded_date = raw_date.replace(microsecond=0) + timedelta(seconds=round(raw_date.microsecond / 1000000))

            DeviceID = int(new_data.get('DeviceID'))
            # Timestamp = str(rounded_date)
            WaterLevel = new_data.get('WaterLevel')

            record = Records(deviceid = DeviceID,reading = WaterLevel)

            db.session.add(record)
            db.session.commit()
            # update(DeviceID, Timestamp, WaterLevel)
        

    return render_template('index.html', devices=devices)

def update(DeviceID, Timestamp, WaterLevel):
    if DeviceID < len(devices):
        devices[DeviceID]['Timestamps'].append(Timestamp)
        devices[DeviceID]['WaterLevels'].append(WaterLevel)
        return
    else:
        new_device = {
            'Timestamps': [Timestamp],
            'WaterLevels': [WaterLevel],
        }
        devices.append(new_device)

if __name__ == '__main__':
    app.run(host='192.168.1.3', port=5050, debug=True, threaded=False)
