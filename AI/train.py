from ultralytics import YOLO

def main():
    # Load pretrained classification model
    model = YOLO("yolov8n-cls.pt")

    # Train
    model.train(
        data="dataset/TomatoDataset",
        epochs=30,
        imgsz=224,
        batch=32,
        device=0,
        project="models",
        name="tomato_classifier"
    )

    print("Training Completed!")

if __name__ == "__main__":
    main()