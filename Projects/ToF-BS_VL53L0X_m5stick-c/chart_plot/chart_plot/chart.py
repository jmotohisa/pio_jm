#!/usr/bin/env python

# このコードは、M5StickCに接続されたVL53L0Xセンサーから距離データをリアルタイムで取得し、グラフに表示するためのものです。

import serial
import time
import collections
import csv
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from datetime import datetime

# --- 設定 ---
SERIAL_PORT = '/dev/cu.usbserial-7552623392'  # お使いのポート名に変更してください
BAUD_RATE = 115200
COMMAND = b'M\n'
MAX_POINTS = 50       
# 保存するファイル名の生成
FILE_NAME = datetime.now().strftime("data_%Y%m%d_%H%M%S.csv")

# データを保持するキュー
x_data = collections.deque(maxlen=MAX_POINTS)
y_data = collections.deque(maxlen=MAX_POINTS)

# シリアル接続
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    time.sleep(2)
except Exception as e:
    print(f"接続エラー: {e}")
    exit()

# CSVファイルの初期化（ヘッダー書き込み）
with open(FILE_NAME, mode='w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["Timestamp", "Elapsed_Time_sec", "Distance_mm"])

# グラフの設定
fig, ax = plt.subplots()
line, = ax.plot([], [], 'cyan', linewidth=2)
fig.patch.set_facecolor('#222222')
ax.set_facecolor('#222222')
ax.tick_params(colors='white')
ax.set_title(f'VL53L0X Log: {FILE_NAME}', color='white')
ax.set_xlabel('Elapsed Time [s]', color='white')
ax.set_ylabel('Distance [mm]', color='white')
ax.grid(True, color='gray', linestyle='--', alpha=0.5)
ax.set_ylim(0, 2000) # 範囲は必要に応じて調整

start_time = time.time()

def update(frame):
    # 1. "M"を送信
    ser.write(COMMAND)
    
    # 2. 受信
    line_in = ser.readline().decode('utf-8').strip()
    
    if line_in:
        try:
            distance = float(line_in)
            now_abs = datetime.now().strftime("%Y-%m-%d %H:%M:%S.%f")[:-3]
            elapsed = time.time() - start_time
            
            # --- データの保存 (CSV追記) ---
            with open(FILE_NAME, mode='a', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([now_abs, f"{elapsed:.3f}", distance])
            
            # --- グラフ表示用の更新 ---
            x_data.append(elapsed)
            y_data.append(distance)
            line.set_data(x_data, y_data)
            
            if len(x_data) > 1:
                ax.set_xlim(x_data[0], x_data[-1])
            
            print(f"[{now_abs}] {distance} mm")
            
        except ValueError:
            pass

    return line,

# 1秒(1000ms)おきに実行
ani = FuncAnimation(fig, update, interval=1000, blit=False, cache_frame_data=False)

plt.tight_layout()
plt.show()

ser.close()
print(f"測定終了。データは '{FILE_NAME}' に保存されました。")
