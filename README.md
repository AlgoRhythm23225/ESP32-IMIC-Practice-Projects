Dùng BLE gửi Wifi để ESP32 kết nối

Cách dùng: 
Bước 1 mở idf-esp:
- idf.py build
- idf.py -p COMx flash monitor

Bước 2 mở folder BLE GUI > app.py > Scan BLE > Chọn ESP32-WifiConfig > Connect > Đợi nó gửi thông báo advertising stopped > điền ID và mật khẩu Wifi của nhà vào > Send Wifi > Nó hiện BLE_WIFI_CFG: Got IP: xxx.xxx.xxx.xxx là thành công