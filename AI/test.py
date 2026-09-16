from ultralytics import YOLO

print("Step 1: Script Started")

model = YOLO("yolov8n.pt")
print("Step 2: Model Loaded")

results = model.predict(
    source="test_images/leaf.jpg",
    save=True,
    device=0,
    verbose=True
)

print("Step 3: Prediction Complete")

for result in results:
    print(result)