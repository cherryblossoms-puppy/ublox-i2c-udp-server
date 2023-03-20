# ublox-i2c-udp-server
Daemon that sends NMEA obtained from i2c via udp  


## ビルド  
cd ublox-i2c-udp-server  
gcc -o ublox_i2c_udp_server ublox-i2c-udp-server.c  

## 実行
./ublox_i2c_udp_server　127.0.0.1　12345 &  
or  
./ublox_i2c_udp_server &  

デフォルトは，127.0.0.1:12345に送信


GPSDを使用して受信する場合
gpsd udp://127.0.0.1:12345 -n -N -D 8
