from flask import Flask, render_template, request
from datetime import datetime, timedelta

app = Flask(__name__)

# sample initial data; can be removed later
devices = [
    {
        'Timestamps': ["0", "1"], 
        'WaterLevels': ["Low", "Moderate"],
    }
]

@app.route('/', methods=['GET', 'POST'])
def index():
    global data  # Use the global data variable

    if request.method == 'POST':
        new_data = request.get_json()
        # Input format: { 'DeviceID': '', 'WaterLevel': ''}
        if new_data:
            raw_date = datetime.now()
            rounded_date = raw_date.replace(microsecond=0) + timedelta(seconds=round(raw_date.microsecond / 1000000))

            DeviceID = int(new_data.get('DeviceID'))
            Timestamp = str(rounded_date)
            WaterLevel = new_data.get('WaterLevel')
            update(DeviceID, Timestamp, WaterLevel)
        

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
    app.run(host='192.168.1.1', port=5050, debug=True, threaded=False)
