from flask import Flask, render_template, request

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
        # Input format: { 'DeviceID': '', 'Timestamp': '', 'WaterLevel': ''}
        if new_data:
            DeviceID = int(new_data.get('DeviceID'))
            Timestamp = new_data.get('Timestamp')
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
    app.run()
