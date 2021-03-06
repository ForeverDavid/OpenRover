# OpenRover
# Remote.py

# Allows you to remote control the vehicle from a websocket, from the self served website.

import sys
import os
import time
import asyncio
try:
    import websockets
except:
    print("No websockets. Try 'sudo pip install websockets'.")
from threading import Thread

# Export mouse x and y, and keyboard button press status
left_mouse_down = False
right_mouse_down = False
x = 0
y = 0
left = False
right = False
up = False
down = False

# Finished?
done = False

# Start thread and create new event loop
loop = asyncio.new_event_loop()
def thread_function(loop):
    asyncio.set_event_loop(loop)
    loop.run_forever()
thread = Thread(target=thread_function, args=(loop,))
thread.start()
def process(text):
    global x, y, left_mouse_down, right_mouse_down, left, right, up, down
    #print(text)
    if text.startswith( 'left' ):
        left = (len(text.split()) > 1 and text.split()[1] == "down")
    elif text.startswith( 'right' ):
        right = (len(text.split()) > 1 and text.split()[1] == "down")
    elif text.startswith( 'up' ):
        up = (len(text.split()) > 1 and text.split()[1] == "down")
    elif text.startswith( 'down' ):
        down = (len(text.split()) > 1 and text.split()[1] == "down")
    elif text.startswith( 'x' ):
        x = int(text.split()[1])
    elif text.startswith( 'y' ):
        y = int(text.split()[1])
    elif text.startswith( 'left_mouse' ):
        left_mouse_down = (len(text.split()) > 1 and text.split()[1] == "down")
    elif text.startswith( 'right_mouse' ):
        right_mouse_down = (len(test.split()) > 1 and text.split()[1] == "down")

# Send to websocket
frame = None
def send_frame(frame_in):
    global frame
    frame = frame_in

# Process incoming websocket connections
@asyncio.coroutine
def new_websocket_connection(websocket, path):
    global frame
    print("Received websocket connection.")
    while True:
#        text = yield from websocket.recv()
#        process(text)

        # Send if frame available to send
        if frame is not None:
            yield from websocket.send(bytes(frame))
            frame = None
        time.sleep(0.01)


# Get IP address
import socket
s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect(("8.8.8.8", 80)) # Google DNS
myIP = s.getsockname()[0]
s.close()

# Start websocket server to listen for keyboard/mouse remote commands via websocket
start_server = websockets.serve(new_websocket_connection, myIP, 8081)
loop.call_soon_threadsafe(asyncio.async, start_server)
