from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer


class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path == "/events":
            length = int(self.headers.get("Content-Length", "0"))
            body = self.rfile.read(length)
            print("event:", body.decode("utf-8"))

            response = b'{"accepted":true}\n'
            self.send_response(201)
            self.send_header("Content-Type", "application/json")
            self.send_header("Content-Length", str(len(response)))
            self.end_headers()
            self.wfile.write(response)
        else:
            self.send_error(404)


ThreadingHTTPServer(("0.0.0.0", 8000), Handler).serve_forever()
