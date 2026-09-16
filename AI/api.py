from fastapi import FastAPI, UploadFile, File
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import StreamingResponse
from fastapi.staticfiles import StaticFiles
from contextlib import asynccontextmanager

from ultralytics import YOLO
from PIL import Image

import cv2
import io
import os
import time
import threading


# ==========================================================
# CONFIGURATION
# ==========================================================

CAMERA_URL = "http://172.21.36.75:8080/video"
SERVER_IP = "172.21.36.75"

STATIC_DIR = "static"
os.makedirs(STATIC_DIR, exist_ok=True)


# ==========================================================
# GLOBAL VARIABLES
# ==========================================================

latest_frame = None

frame_lock = threading.Lock()

latest_prediction = {
    "crop": "",
    "disease": "Waiting...",
    "confidence": 0,
    "severity": "--",
    "recommendation": "",
    "recommendations": [],
    "image": ""
}


# ==========================================================
# LOAD YOLO MODEL
# ==========================================================

print("Loading YOLO model...")

model = YOLO(
    "runs/classify/models/tomato_classifier/weights/best.pt"
)

print("YOLO Loaded Successfully")


# ==========================================================
# RECOMMENDATION DATABASE
# ==========================================================

recommendations = {

    "Tomato_Bacterial_spot": {
        "severity": "Medium",
        "treatment": [
            "Remove infected leaves",
            "Avoid overhead watering",
            "Apply copper bactericide"
        ]
    },

    "Tomato_Early_blight": {
        "severity": "High",
        "treatment": [
            "Apply Mancozeb",
            "Remove infected leaves",
            "Improve air circulation"
        ]
    },

    "Tomato_Late_blight": {
        "severity": "Very High",
        "treatment": [
            "Remove infected plants",
            "Apply fungicide",
            "Reduce humidity"
        ]
    },

    "Tomato_healthy": {
        "severity": "Healthy",
        "treatment": [
            "No disease detected",
            "Continue monitoring",
            "Maintain irrigation"
        ]
    }

}
# ==========================================================
# CAMERA WORKER
# ==========================================================

def camera_worker():

    global latest_frame
    global latest_prediction

    print("Starting camera thread...")

    cap = None

    while True:

        print("Connecting to IP Webcam...")

        cap = cv2.VideoCapture(CAMERA_URL)

        if cap.isOpened():
            print("✅ Camera Connected")
            break

        print("❌ Camera connection failed. Retrying in 5 seconds...")
        time.sleep(5)

    last_prediction_time = 0

    while True:

        ret, frame = cap.read()

        if not ret:

            print("⚠ Camera disconnected")

            cap.release()

            while True:

                print("Reconnecting...")

                cap = cv2.VideoCapture(CAMERA_URL)

                if cap.isOpened():
                    print("✅ Camera Reconnected")
                    break

                time.sleep(5)

            continue

        # Store latest frame safely
        with frame_lock:
            latest_frame = frame.copy()

        # Save latest image
        image_path = os.path.join(STATIC_DIR, "latest.jpg")
        cv2.imwrite(image_path, frame)

        # Run YOLO once every second
        if time.time() - last_prediction_time < 1:
            continue

        last_prediction_time = time.time()

        try:

            rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
            image = Image.fromarray(rgb)

            results = model.predict(
                image,
                imgsz=224,
                verbose=False
            )

            result = results[0]
            probs = result.probs

            disease = result.names[probs.top1]
            confidence = float(probs.top1conf * 100)

            info = recommendations.get(
                disease,
                {
                    "severity": "Unknown",
                    "treatment": ["No recommendation available"]
                }
            )

            latest_prediction = {
                "crop": "Tomato",
                "disease": disease,
                "confidence": round(confidence, 2),
                "severity": info["severity"],
                "recommendation": info["treatment"][0],
                "recommendations": info["treatment"],
                "image": f"http://{SERVER_IP}:8000/static/latest.jpg?t={int(time.time())}"
            }

            print(f"Prediction: {disease} ({confidence:.2f}%)")

        except Exception as e:
            print("Prediction Error:", e)


# ==========================================================
# FASTAPI STARTUP
# ==========================================================

@asynccontextmanager
async def lifespan(app: FastAPI):

    threading.Thread(
        target=camera_worker,
        daemon=True
    ).start()

    yield


app = FastAPI(
    title="AGRI-X AI Backend",
    lifespan=lifespan
)


# ==========================================================
# CORS
# ==========================================================

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# ==========================================================
# STATIC FILES
# ==========================================================

app.mount(
    "/static",
    StaticFiles(directory=STATIC_DIR),
    name="static"
)
# ==========================================================
# HOME
# ==========================================================

@app.get("/")
def home():
    return {
        "status": "AGRI-X AI Backend Running"
    }


# ==========================================================
# LATEST PREDICTION
# ==========================================================

@app.get("/latest")
def latest():
    return latest_prediction


# ==========================================================
# MANUAL IMAGE PREDICTION
# ==========================================================

@app.post("/predict")
async def predict(file: UploadFile = File(...)):

    try:

        image_bytes = await file.read()

        image = Image.open(io.BytesIO(image_bytes)).convert("RGB")

        results = model.predict(
            image,
            imgsz=224,
            verbose=False
        )

        result = results[0]

        probs = result.probs

        disease = result.names[probs.top1]

        confidence = float(probs.top1conf * 100)

        info = recommendations.get(
            disease,
            {
                "severity": "Unknown",
                "treatment": ["No recommendation available"]
            }
        )

        return {
            "crop": "Tomato",
            "disease": disease,
            "confidence": round(confidence, 2),
            "severity": info["severity"],
            "recommendation": info["treatment"][0],
            "recommendations": info["treatment"]
        }

    except Exception as e:

        return {
            "error": str(e)
        }


# ==========================================================
# LIVE VIDEO STREAM
# ==========================================================

def generate_frames():

    global latest_frame

    print("Video client connected")

    while True:

        if latest_frame is None:
            time.sleep(0.05)
            continue

        with frame_lock:
            frame = latest_frame.copy()

        print("Sending frame")

        success, buffer = cv2.imencode(".jpg", frame)

        if not success:
            continue

        yield (
            b"--frame\r\n"
            b"Content-Type: image/jpeg\r\n\r\n"
            + buffer.tobytes()
            + b"\r\n"
        )

        time.sleep(0.03)   # ~30 FPS


@app.get("/video")
def video():

    return StreamingResponse(
        generate_frames(),
        media_type="multipart/x-mixed-replace; boundary=frame"
    )