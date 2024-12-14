from flask import Flask, jsonify, request
from flask_cors import CORS
import random
import firebase_admin
from firebase_admin import credentials, firestore

app = Flask(__name__)
CORS(app)

cred = credentials.Certificate("/root/food-supply-tracking/backend/awom-c91d6-firebase-adminsdk-y6311-cb6ef01f23.json")
firebase_admin.initialize_app(cred)
db = firestore.client()

alerts = [
    {"timestamp": "2024-12-13 10:00:00", "type": "Temperature Spike", "value": "23°C"},
    {"timestamp": "2024-12-13 11:30:00", "type": "Humidity Drop", "value": "20%"},
    {"timestamp": "2024-12-13 12:45:00", "type": "Pressure Spike", "value": "1050 hPa"}
]

# Default Inventory
inventory = [
    {"name": "Milk Carton", "rack": "A1", "delivery_location": "Store A"},
    {"name": "Bread Loaf", "rack": "B2", "delivery_location": "Store B"},
    {"name": "Cheese Box", "rack": "C3", "delivery_location": "Store C"},
    {"name": "Canned Beans", "rack": "D4", "delivery_location": "Store D"},
    {"name": "Juice Bottle", "rack": "E5", "delivery_location": "Store E"}
]

# Random Sensor Data
sensor_data = {pkg["name"]: {} for pkg in inventory}
stages = ["Producer", "Manufacturing", "Processing", "Packaging", "Warehouse"]

def generate_random_sensor_data(product_id):
    return {
    #     "timestamp": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "product_id": product_id,
        "temperature": round(random.uniform(15, 25), 2),
        "humidity": round(random.uniform(30, 80), 2),
        "pressure": round(random.uniform(950, 1050), 2)
    }

@app.route("/inventory", methods=["GET"])
def get_inventory():
    return jsonify(inventory)

#Endpoint to Fetch Firebase Data
# @app.route("/sensor-data/<product_id>", methods=["GET"])
# def get_sensor_data(product_id):
#     docs = db.collection("location_data").order_by("timestamp", direction=firestore.Query.DESCENDING).limit(1).stream()
#     latest_data = [doc.to_dict() for doc in docs]
#     if not latest_data:
#         return jsonify({"error": "No data available"}), 404
#     return jsonify(latest_data[0])

@app.route("/sensor-data/<product_id>", methods=["GET"])
def get_sensor_data(product_id):
    if not product_id:
        return jsonify({"error": "Product ID is required"}), 400

    # Generate random sensor data
    sensor_data = generate_random_sensor_data(product_id)
    return jsonify(sensor_data)

# Endpoint to Get Alerts
@app.route("/alerts", methods=["GET"])
def get_alerts():
    return jsonify(alerts)

# Endpoint to Simulate Alert
@app.route("/generate-alert", methods=["POST"])
def generate_alert():
    data = request.json
    alerts.append(data)
    return jsonify({"message": "Alert added successfully"}), 201

# @app.route("/sensor-data", methods=["GET"])
# def get_sensor_data():
#     generate_sensor_data()
#     return jsonify(sensor_data)

@app.route("/dispatch", methods=["POST"])
def dispatch_product():
    data = request.json
    name = data.get("name")
    return jsonify({"message": f"Product '{name}' is being dispatched", "eta": "2 hours"})

if __name__ == "__main__":
    app.run(debug=True)
