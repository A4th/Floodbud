from flask import Flask, render_template, request
import os
import pytz
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime, timedelta
from sqlalchemy import Sequence
# from sqlalchemy import func

# Replace 'Asia/Singapore' with the appropriate timezone identifier
utc_plus_eight = pytz.timezone('Asia/Singapore')

basedir = os.path.abspath(os.path.dirname(__file__))
app = Flask(__name__)

# create the extension
db = SQLAlchemy()

# create the app
app = Flask(__name__)
# configure the SQLite database, relative to the app instance folder
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///project.db"
# initialize the app with the extension
db.init_app(app)

class DeviceLocation(db.Model):
    __tablename__ = 'device_location'
    deviceid = db.Column(db.Integer, primary_key=True)
    location = db.Column(db.String(100), nullable=False)

    def __repr__(self):
        return self.location

class Records(db.Model):
    # device id
    __tablename__ = 'water_level'
    readingid = db.Column(db.Integer, Sequence(
        'reading_id_seq'), primary_key=True)
    deviceid = db.Column(db.Integer, db.ForeignKey('device_location.deviceid'))
    timestamp = db.Column(db.DateTime(timezone=True),
                          default=lambda: datetime.now(utc_plus_eight))
    reading = db.Column(db.String, nullable=False)
    location = db.relationship(
        'DeviceLocation', backref='water_level', lazy=True)

    def __repr__(self):
        return f'<Device {self.deviceid}: {self.reading} at {self.timestamp}, readingID = {self.readingid}, location = {self.location}>'


with app.app_context():
    db.create_all()

devices = []

@app.route('/location', methods=['GET', 'POST'])
def location():
    location = request.args.get('location')
    print(location)
    id = DeviceLocation.query.filter_by(location=location).first().deviceid

    selected_date = request.args.get('day')
    if selected_date != None:
        selected_date = request.args.get('day')
        print(selected_date)
        selected_date = datetime.strptime(selected_date, '%Y-%m-%d')

        day_start = selected_date.replace(hour=0, minute=0, second=0)
        day_end = day_start + timedelta(days=1)

        records = Records.query.filter(Records.deviceid == id, Records.timestamp >= day_start, Records.timestamp < day_end)

        timestamps = []
        readings = []
        for entry in records:
            timestamps.insert(0, entry.timestamp)
            readings.insert(0, entry.reading.title())
        device_day = {
            'DeviceID': id,
            'Location': location,
            'Timestamps': timestamps,
            'WaterLevels': readings,
        }
        return render_template('location.html', location=location, readings=device_day)

    return render_template('location.html', location=location, readings=devices[id])

@app.route('/', methods=['GET', 'POST'])
def index():
    global data  # Use the global data variable

    if request.method == 'POST':
        new_data = request.get_json()
        # Input format: { 'DeviceID': '', 'WaterLevel': ''}
        if new_data:

            DeviceID = int(new_data.get('DeviceID'))
            WaterLevel = new_data.get('WaterLevel')

            record = Records(deviceid=DeviceID, reading=WaterLevel)

            db.session.add(record)
            db.session.commit()
    update_db()

    return render_template('index.html', devices=devices)


def update_db():
    devices.clear()
    records = Records.query.all()
    if records:
        for device in range(max(list({record.deviceid for record in records}))):
            timestamps = []
            readings = []
            for entry in Records.query.filter_by(deviceid=device):
                timestamps.insert(0, entry.timestamp)
                readings.insert(0, entry.reading.title())

            if len(devices) > device:
                devices[device]['Timestamps'].extend(timestamps)
                devices[device]['WaterLevels'].extend(readings)
            else:
                new_device = {
                    'DeviceID': device,
                    'Location': DeviceLocation.query.filter_by(deviceid=device).first(),
                    'Timestamps': timestamps,
                    'WaterLevels': readings,
                }
                devices.insert(device, new_device)


if __name__ == '__main__':
    app.run(host='192.168.1.3', port=5050, debug=True, threaded=False)
