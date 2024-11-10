import asyncio
import multiprocessing as mp
import time
from typing import Dict, Any

from bleak import BleakScanner
from bleak import BleakClient

def parse_nus_data(data: bytearray) -> Dict[str, Any]:
    """
    Data should follow this format:
    [0, 3]: ID -- increments with each message
    [4, 7]: Value -- 32-bit signed int
    [8, 11]: Units (unused)

    Example output:
    latest_data = {
        "id": 0,
        "value": 0,
        "timestamp": 0.0,
        "opts": ""
    }
    """
    # if len(data) != 12:
    #     print(f"received invalid data: {data}")
    #     return {}

    id = int.from_bytes(data[0:4], byteorder='little')
    value = int.from_bytes(data[4:8], byteorder='little', signed=True)

    ret_dict = {}
    ret_dict["id"] = id
    ret_dict["value"] = value / 50  # mm only
    ret_dict["timestamp"] = time.time()

    print("received data: ", ret_dict)

    return ret_dict

class BluetoothClient:
    """
    Class containing a bluetooth client's connection details
    Runs entirely asynchronously, await the run method to start

    TODO: reconnect on disconnect
    """

    def __init__(self, queue : mp.Queue, device_name : str, args) -> None:
        self.queue = queue
        self.name = device_name
        self.device = None
        self.advertising_data = None
        self.args = args

    async def run(self):
        await self._find()
        await self._connect()

    async def _find(self):
        stop_event = asyncio.Event()

        def detection_callback(dev, advert):
            if dev.name == self.name:
                stop_event.set()
                self.device = dev
                self.advertising_data = advert

        async with BleakScanner(detection_callback) as scanner:
            # TODO: add timeout
            await stop_event.wait()

        print(f'Found device: {self.device}')
        print(f'found dev: {time.time()}')
        print(self.advertising_data)


    async def _connect(self):
        disconnect_event = asyncio.Event()

        def disconnect_callback(dev):
            disconnect_event.set()
            self.queue.put(bytearray())
            print(f'disconnected: {dev}')

        def recieve_callback(gatt_char, data):
            self.queue.put(parse_nus_data(data))

        try:
            async with BleakClient(self.device, disconnect_callback, timeout=self.args.timeout) as client:
                characteristics = client.services.characteristics.copy()
                if any((gatt_char := gatt).description == self.args.gatt_descriptor 
                    for gatt in characteristics.values()):
                    print(gatt_char)
                    print(f'started notif: {time.time()}')

                    await client.start_notify(gatt_char, recieve_callback)
                    await disconnect_event.wait()

        except asyncio.exceptions.CancelledError:
            print('connection cancelled')
        except asyncio.exceptions.TimeoutError:
            print('connection timeout')




        
