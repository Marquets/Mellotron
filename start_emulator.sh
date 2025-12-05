#!/bin/bash
# Start a simple Python HTTP server to serve the web emulator
# This avoids CORS issues when loading local WAV files

PORT=8080
echo "Starting Mellotron Emulator Server..."
echo "Open your browser at: http://localhost:$PORT/web_emulator.html"
echo "Press Ctrl+C to stop the server."

python3 -m http.server $PORT
