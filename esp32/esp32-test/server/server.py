import json
import os
import threading
import uuid
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import unquote, urlparse

ROOT = Path(__file__).resolve().parent
IMAGE_DIR = ROOT / "images"
INDEX_FILE = ROOT / "index.html"
MAX_IMAGE_BYTES = 500_000
MAX_LISTED_IMAGES = 60
IMAGE_DIR.mkdir(exist_ok=True)
MAX_STORED_IMAGES = 120
STORAGE_LOCK = threading.Lock()


def prune_images_locked():
    files = sorted(IMAGE_DIR.glob("*.jpg"))
    expired = files[:-MAX_STORED_IMAGES]
    for path in expired:
        path.unlink(missing_ok=True)
    return len(expired)


def log_event(event, **fields):
    record = {
        "time": datetime.now(timezone.utc).isoformat(),
        "event": event,
        **fields,
    }
    print(json.dumps(record, ensure_ascii=False, separators=(",", ":")), flush=True)


class Handler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        log_event(
            "http_access",
            client=self.client_address[0],
            message=format % args,
        )

    def send_bytes(self, status, content_type, body, cache_control="no-store"):
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(body)))
        self.send_header("Cache-Control", cache_control)
        self.end_headers()
        self.wfile.write(body)

    def send_json(self, status, payload):
        body = (json.dumps(payload) + "\n").encode("utf-8")
        self.send_bytes(status, "application/json", body)

    def do_POST(self):
        if self.path == "/events":
            length = int(self.headers.get("Content-Length", "0"))
            body = self.rfile.read(length)
            log_event("event_received", body=body.decode("utf-8"))
            self.send_json(201, {"accepted": True})
            return

        if self.path != "/images":
            self.send_error(404)
            return

        content_type = self.headers.get_content_type()
        length = int(self.headers.get("Content-Length", "0"))
        sequence = self.headers.get("X-Image-Sequence", "?")

        if content_type != "image/jpeg":
            self.send_json(415, {"error": "expected image/jpeg"})
            return
        if length < 4 or length > MAX_IMAGE_BYTES:
            self.send_json(413, {"error": "invalid image size"})
            return

        image = self.rfile.read(length)
        complete = len(image) == length
        jpeg = image.startswith(b"\xff\xd8") and image.endswith(b"\xff\xd9")
        if not complete or not jpeg:
            self.send_json(422, {"error": "invalid jpeg"})
            return

        received_at = datetime.now(timezone.utc)
        timestamp = received_at.strftime("%Y%m%dT%H%M%S.%fZ")
        filename = f"{timestamp}-{uuid.uuid4().hex[:6]}.jpg"
        final_path = IMAGE_DIR / filename
        temporary_path = IMAGE_DIR / f".{filename}.tmp"

        try:
            with temporary_path.open("xb") as output:
                output.write(image)
                output.flush()
                os.fsync(output.fileno())
            with STORAGE_LOCK:
                os.replace(temporary_path, final_path)
        except OSError as error:
            temporary_path.unlink(missing_ok=True)
            log_event("image_save_failed", error=str(error))
            self.send_json(500, {"error": "save failed"})
            return

        pruned = 0
        try:
            with STORAGE_LOCK:
                pruned = prune_images_locked()
        except OSError as error:
            log_event("image_prune_failed", error=str(error))

        log_event(
            "image_saved",
            sequence=sequence,
            filename=filename,
            bytes=len(image),
            pruned=pruned,
        )
        self.send_json(
            201,
            {
                "accepted": True,
                "sequence": sequence,
                "bytes": len(image),
                "received_at": received_at.isoformat(),
                "url": f"/images/{filename}",
            },
        )

    def do_DELETE(self):
        path = unquote(urlparse(self.path).path)
        if path != "/api/images":
            self.send_error(404)
            return

        deleted = 0
        try:
            with STORAGE_LOCK:
                for image_path in IMAGE_DIR.glob("*.jpg"):
                    image_path.unlink()
                    deleted += 1
        except OSError as error:
            log_event("images_delete_failed", deleted=deleted, error=str(error))
            self.send_json(500, {"error": "delete failed", "deleted": deleted})
            return

        log_event("images_deleted", deleted=deleted)
        self.send_json(200, {"deleted": deleted})

    def do_GET(self):
        path = unquote(urlparse(self.path).path)

        if path == "/":
            if not INDEX_FILE.is_file():
                self.send_error(500, "index.html missing")
                return
            self.send_bytes(200, "text/html; charset=utf-8", INDEX_FILE.read_bytes())
            return

        if path == "/api/status":
            with STORAGE_LOCK:
                files = list(IMAGE_DIR.glob("*.jpg"))
                stored_bytes = sum(file.stat().st_size for file in files)
            self.send_json(
                200,
                {
                    "stored_images": len(files),
                    "stored_bytes": stored_bytes,
                    "max_stored_images": MAX_STORED_IMAGES,
                },
            )
            return

        if path == "/api/images":
            with STORAGE_LOCK:
                files = sorted(IMAGE_DIR.glob("*.jpg"))[-MAX_LISTED_IMAGES:]
                images = [
                    {"name": file.name, "url": f"/images/{file.name}"} for file in files
                ]
            self.send_json(200, {"images": images})
            return

        if path.startswith("/images/"):
            filename = path.removeprefix("/images/")
            safe_name = Path(filename).name == filename and filename.endswith(".jpg")
            image_path = IMAGE_DIR / filename
            if not safe_name:
                self.send_error(404)
                return

            try:
                with STORAGE_LOCK:
                    body = image_path.read_bytes() if image_path.is_file() else None
            except OSError as error:
                log_event("image_read_failed", filename=filename, error=str(error))
                self.send_error(500)
                return

            if body is None:
                self.send_error(404)
                return

            self.send_bytes(
                200,
                "image/jpeg",
                body,
                "public, max-age=31536000, immutable",
            )
            return

        self.send_error(404)


server = ThreadingHTTPServer(("0.0.0.0", 8000), Handler)
try:
    print(f"server: http://127.0.0.1:8000 images={IMAGE_DIR}")
    server.serve_forever()
except KeyboardInterrupt:
    print("\nserver: stopped")
finally:
    server.server_close()
