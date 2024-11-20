#include <WiFi.h>
#include <esp_now.h>
#include <Adafruit_BMP085.h>

Adafruit_BMP085 bmp;  // BMP085 sensörü

// Gönderilecek veri yapısı
typedef struct struct_message {
   float yükseklik1;  // Görev yükünden alınacak yükseklik1
  float yükseklik2;  // BMP085 sensöründen gelen yükseklik
  float basınç1;     // Görev yükünden alınacak basınç1
  float basınç2;     // BMP085 sensöründen gelen basınç
  int paketno;       // Paket numarası
  float sıcaklık;    // Görev yükünden alınacak sıcaklık
  float irtifafarkı; 
  float inişhızı;
  float pilgerilimi;
  float pitch;
  float roll;
  float yaw;
  char rhrh[10];     // Karakter dizisi
  float iotdata;     // Yer istasyonundan alınacak sıcaklık (DHT11)
  int takımno;
  int uydustatüsü;
  int hatakodu;
  char göndermesaati[25];
} struct_message;

struct_message myData;

// Alıcı MAC adresi (Görev yükünün MAC adresi)
uint8_t broadcastAddress[] = {0xA0, 0xA3, 0xB3, 0x8A, 0x71, 0xE0};  // Örnek MAC adresi, görev yükü MAC adresiyle değiştirin

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Veri gönderimi: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Başarılı" : "Başarısız");
}

void setup() {
  // Seri haberleşmeyi başlat
  Serial.begin(115200);

  // Wi-Fi modülünü başlat
  WiFi.mode(WIFI_STA);

  // BMP085 sensörünü başlat
  if (!bmp.begin()) {
    Serial.println("BMP085 sensörü bulunamadı, bağlantıları kontrol edin!");
    while (1) {}
  }

  // ESP-NOW'u başlat
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı");
    return;
  }

  // Veri gönderimi için callback fonksiyonunu ayarla
  esp_now_register_send_cb(OnDataSent);

  // Peer ekle
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo)); // Yapıyı sıfırla
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0; // Varsayılan kanal
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Peer eklenemedi");
    return;
  }

  Serial.println("Taşıyıcı hazır.");
}

void loop() {
  // BMP085 sensöründen verileri oku
  myData.basınç2 = bmp.readSealevelPressure();
  myData.yükseklik2 = bmp.readAltitude(101500);

  // Paket numarasını ayarla
  myData.paketno++;

  // Veriyi gönder
  esp_now_send(broadcastAddress, (uint8_t *)&myData, sizeof(myData));

  // 2 saniye bekle
  delay(1000);
}
