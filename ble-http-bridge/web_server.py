from flask import Flask, jsonify
from queue import Empty
from multiprocessing import Queue

app = Flask(__name__)

server_data_queue = None
latest_data = None

class WebServer():
    """
    Usage:

    q = Queue()
    ws = WebServer(q)
    ws.start()

    ... (other thread)
    q.put(data: Dict[str, Any])

    ... (other application)
    response = requests.get("http://localhost:5000/get_data")
    """


    def __init__(self, q: Queue) -> None:
        global server_data_queue
        server_data_queue = q

    def start(self):
        app.run(host='0.0.0.0', port=5000)

@app.route('/get_data', methods=['GET'])
def get_latest_data():
    global server_data_queue
    global latest_data
    if server_data_queue is None:
        return jsonify({"error_internal": "queue not initialized"}), 404
    
    try:
        while True: # empty til last element in buffer. tmp fix if bandwidth is too low
            data = server_data_queue.get(block=False)
            latest_data = data
    except Empty:
        pass

    if latest_data is None:
        return jsonify({"error_internal": "No data available"}), 404
    elif len(latest_data) == 0:
        return jsonify({"error_internal": "Disconected"}), 404
    
    return jsonify(latest_data), 200

