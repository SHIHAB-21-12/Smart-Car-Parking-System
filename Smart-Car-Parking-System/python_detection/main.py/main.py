import cv2
from ultralytics import YOLO
import requests
import time

model = YOLO("my_model.pt")

# 🔥 FORCE CAMERA
cap = cv2.VideoCapture(0, cv2.CAP_DSHOW)

# 🔥 REMOVE CAMERA BUFFER LAG
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)

# 🔥 OPTIONAL: SET RESOLUTION (STABLE FPS)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

ESP_IP = "192.168.0.100"
URL = f"http://{ESP_IP}/car_logic"

CONF_THRESHOLD = 0.5

car_counter = 0
state = 0
last_sent = -1

# 🔥 FRAME SKIP FOR SPEED
frame_count = 0

# 🔥 Detect class index dynamically
CAR_CLASS_ID = None
for k, v in model.names.items():
    if v.lower() == "car":
        CAR_CLASS_ID = k
        break

if CAR_CLASS_ID is None:
    raise ValueError("❌ 'car' class not found in model")

print(f"Using CAR class ID: {CAR_CLASS_ID}")

def send_with_retry(url, retries=3):
    for attempt in range(retries):
        try:
            response = requests.get(url, timeout=1)
            return response.text
        except Exception as e:
            print(f"Retry {attempt+1} failed:", e)
            time.sleep(0.1)
    return None

while True:
    ret, frame = cap.read()
    if not ret:
        print("Camera error")
        break

    # 🔥 FRAME SKIP
    frame_count += 1
    if frame_count % 2 != 0:
        continue

    # 🔥 FASTER INFERENCE
    results = model(frame, imgsz=320)

    car_detected = False

    for r in results:
        if r.boxes is not None:
            for box in r.boxes:
                cls = int(box.cls[0])
                conf = float(box.conf[0])

                if cls == CAR_CLASS_ID and conf > CONF_THRESHOLD:
                    car_detected = True
                    break

    # --- STABILITY FILTER ---
    if car_detected:
        car_counter = min(car_counter + 1, 10)
    else:
        car_counter = max(car_counter - 1, 0)

    if car_counter >= 4:
        state = 1
    elif car_counter <= 1:
        state = 0

    # --- SEND ONLY ON CHANGE ---
    if state != last_sent:
        response = send_with_retry(f"{URL}?state={state}")

        if response is not None:
            print(f"Sent: {state}, Response: {response}")
            last_sent = state
        else:
            print("❌ Failed to send after retries")

    # --- DISPLAY ---
    frame = results[0].plot()
    cv2.imshow("Detection", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()