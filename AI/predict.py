from ultralytics import YOLO

# Load your trained model
model = YOLO("runs/classify/models/tomato_classifier/weights/best.pt")

# Predict on a test image
results = model.predict(
    source="test_images/leaf.jpg",
    imgsz=224
)

for result in results:
    probs = result.probs

    print("\n====== AI Prediction ======")
    print("Disease   :", result.names[probs.top1])
    print(f"Confidence: {probs.top1conf*100:.2f}%")