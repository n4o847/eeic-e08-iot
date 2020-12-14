// ブザーを on の状態にする．
// (注意) 事前に pinMode(14,OUTPUT) の処理が必要．
void setBZ(boolean on) {
  digitalWrite(14, on ? HIGH : LOW);
}

// プッシュ SW の現在の状態を取得する．
// (注意) 事前に pinMode(2, INPUT) の処理が必要．
boolean getPushSWStatus() {
  return digitalRead(2) == HIGH;
}

// プッシュ SW が押されたことを判定する．前回の呼び出し時に
// プッシュ SW が押されていない，かつ，今回の呼び出し時に押
// されていた場合にのみ true となる．それ以外は false．最初の
// 呼び出しの場合は，必ず false となる．
// (注意) 事前に pinMode(2, INPUT) の処理が必要．
boolean detectPushSWON() {
  static int prev_stat = LOW;
  int stat = digitalRead(2);
  boolean result = false;
  if (prev_stat == LOW && stat == HIGH) {
    result = true;
  }
  prev_stat = stat;
  return result;
}

// DIP SW の現在の設定値を取得する．
// (注意) 事前に pinMode(12, INPUT); pinMode(13, INPUT); の
// 処理が必要．
int getDIPSWStatus() {
  int stat = 0;
  int bit1 = digitalRead(12);
  int bit0 = digitalRead(13);
  if (bit0 == LOW) {
    stat |= 0x01;
  }
  if (bit1 == LOW) {
    stat |= 0x02;
  }
  return stat;
}

// 人感センサ (MD: Motion Detector) の ON/OFF 状態を取得する
// (注意) 事前に pinMode(16, INPUT) の処理が必要．
boolean getMDStatus() {
  return digitalRead(16) == HIGH;
}

// http://www.musashinodenpa.com/arduino/ref/index.php?f=0&pos=2743
double translate(double value, double in_min, double in_max, double out_min, double out_max) {
  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// 現在の照度 (lux) を取得する．
int getIlluminance() {
  int stat = analogRead(A0);
  double v_tout = (double)(stat) / 1024.0;
  double v_a0 = v_tout * ((220e3 + 100e3) / 100e3);
  double i_s9618 = v_a0 / 1.0e3 + v_a0 / (220e3 + 100e3);
  double y = log10(i_s9618);
  double x = translate(y, log10(300e-9), log10(5e-3), -1.0, 4.0);
  double lx = pow(10, x);
  return (int)(lx);
}

// 指定した NTP サーバから現在時刻を取得し，日本標準時の
// Unixtime を提供する．
// (注意) 事前に WiFi に関する接続処理が行われている必要があ
// る．
unsigned long getNTPtime(const char* ntp_server) {
  WiFiUDP udp;
  udp.begin(8888);
  unsigned long unix_time = 0UL;
  byte packet[48];
  memset(packet, 0, 48);
  packet[0] = 0b11100011;
  packet[1] = 0;
  packet[2] = 6;
  packet[3] = 0xEC;
  packet[12] = 49;
  packet[13] = 0x4E;
  packet[14] = 49;
  packet[15] = 52;
  udp.beginPacket(ntp_server, 123);
  udp.write(packet, 48);
  udp.endPacket();
  for (int i = 0; i < 10; i++) {
    delay(500);
    if (udp.parsePacket()) {
      udp.read(packet, 48);
      unsigned long highWord = word(packet[40], packet[41]);
      unsigned long lowWord = word(packet[42], packet[43]);
      unsigned long secsSince1900 = highWord << 16 | lowWord;
      const unsigned long seventyYears = 2208988800UL;
      unix_time = secsSince1900 - seventyYears + 32400UL; // 32400 = 9 hours (JST)
      break;
    }
  }
  udp.stop();
  return unix_time;
}

// 指定した NTP サーバと時刻同期を行う．
// (注意) 上述の getNTPTime 関数を利用できる必要がある．
boolean syncNTPtime(const char* ntp_server) {
  unsigned long unix_time = getNTPtime(ntp_server);
  if (unix_time > 0) {
    setTime(unix_time);
    return true;
  }
  return false;
}
