from flask import Flask, render_template, request

app = Flask(__name__)

data = 'initial data'  # Initial data for display

@app.route('/', methods=['GET', 'POST'])
def index():
    global data  # Use the global data variable

    if request.method == 'POST':
        new_data = request.form['data']
        data = new_data  # Update the data with the new value

    return render_template('index.html', data=data)

if __name__ == '__main__':
    app.run()
