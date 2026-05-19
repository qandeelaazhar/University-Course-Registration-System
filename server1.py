import http.server
import socketserver
import json
import subprocess
import traceback
PORT = 8001
class RegistrationHandler(http.server.SimpleHTTPRequestHandler):
    def do_POST(self):
        if self.path == '/api/run':
            try:
                content_length = int(self.headers['Content-Length'])
                data = json.loads(self.rfile.read(content_length))
                print("Writing input file (web_input.txt)...")
                with open('web_input.txt', 'w') as f:
                    f.write(f"{len(data['capacities'])}\n{len(data['students'])}\n")
                    for cap in data['capacities']: f.write(f"{cap}\n")
                    for s in data['students']:
                        f.write(f"{s['name']}\n{s['priority']} {len(s['requests'])} {' '.join(map(str, s['requests']))}\n")
                print("Executing C program (./OS)...")
                # Look specifically for the compiled "OS" file
                result = subprocess.run(["./OS", "web"], capture_output=True, text=True)
                if result.returncode != 0:
                    print(f"C Program Error: {result.stderr}")
                    raise Exception("C Program failed to execute.")

                self.send_response(200)
                self.send_header('Content-type', 'application/json')
                self.end_headers()
                self.wfile.write(b'{"status": "success"}')
                print("Simulation successful!")

            except Exception as e:
                print(f"Server Error: {e}")
                traceback.print_exc()
                self.send_response(500)
                self.end_headers()
        else:
            super().do_GET()
# This prevents the "Address already in use" error
class ReusableTCPServer(socketserver.TCPServer):
    allow_reuse_address = True
with ReusableTCPServer(("", PORT), RegistrationHandler) as httpd:
    print(f"Server running! Open this link in your browser: http://localhost:{PORT}/index3.html")
    httpd.serve_forever()
