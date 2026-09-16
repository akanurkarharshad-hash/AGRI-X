import cv2

url = "http://10.251.9.203:8080/video"

cap = cv2.VideoCapture(url)

print("Opened:", cap.isOpened())

while True:
    ret, frame = cap.read()

    print("ret =", ret)

    if ret:
        cv2.imshow("Camera", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()