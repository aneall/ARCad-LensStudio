import requests
from time import sleep
import json

GET_IP = "localhost"

while True:
    response = requests.get(f"http://{GET_IP}:5000/get_data")
    if response.status_code == 200:
        data = response.json()
        print(json.dumps(data, indent=4))
        sleep(1)
    else:
        print("Failed to fetch data.")
        break
