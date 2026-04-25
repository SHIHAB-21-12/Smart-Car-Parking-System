# 🚗 Smart Car Parking System (AI + IoT)

An intelligent parking system using Deep Learning (YOLOv8) and ESP32 to automate vehicle entry, slot management, and billing.

---

## 🔥 Features

- 🚘 Real-time car detection using YOLOv8
- 🚪 Automatic gate control using ESP32 + Servo
- 📡 IR sensor-based slot detection
- 💰 Automatic billing system (time-based)
- 🌐 Web API for monitoring parking status
- ⚡ Real-time communication between AI and ESP32

---

## 🧠 System Architecture

![System](docs/system_architecture.png)

---

## ⚙️ Technologies Used

- Python (OpenCV, Ultralytics YOLO)
- ESP32 (IoT + WebServer)
- Deep Learning (YOLOv8)
- Embedded Systems
- HTTP Communication

---

## 🔄 Workflow

1. Camera captures video
2. YOLO model detects car
3. Python sends signal to ESP32
4. ESP32 opens gate
5. Car parks in slot
6. Billing starts automatically
7. On exit → bill is calculated

---

## 📦 Model

Download trained model here:  
👉 (Add your Google Drive link)

---

## 🚀 How to Run

```bash
pip install -r requirements.txt
python main.py
