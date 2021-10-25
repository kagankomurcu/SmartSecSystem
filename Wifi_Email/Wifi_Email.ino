#include <Adafruit_ESP8266.h>
#include <SoftwareSerial.h>
#include "tan_tma.h"

// ESP8266 pinlerini tanimla
#define RX 3  // okuma baglanti pini
#define TX 4  // yazma baglanti pini
#define RST 8 // reset baglanti pini

// PIR pinlerini tanimla
#define PIR1 10
#define PIR2 11
#define PIR3 12

// PIR degiskeni tanimla
bool durumPir = 0;

// Buzzer pinlerini tanimla
#define BUZ 13

// HC-SR04 pinlerini tanimla
#define TRG 6
#define ECH 7

// Sonar sensor degiskenleri tanimla
int kapi;  // kapi degiskeni olustur
int mesafe = 5; // mesafe değişkeni oluştur ve cm cinsinden guvenli deger olustur


// yazilim seri baglantisi
SoftwareSerial yazilim(RX, TX); // once rx sonra tx pini

// aygit seri baglantisiS
Adafruit_ESP8266 wifi(&yazilim, &Serial, RST);

#define AgAdi "wifi ag adi" // wifi ag adi
#define AgPar "wifi sifresi" // wifi ag parolasi

char GONDEREN[] = "kaanguvenlikprojesi@gmail.com"; // gonderen eposta "kaanguvenlikprojesi@gmail.com"
char PAROLA[] = "cfkbp2021*"; // eposta parola "cfkbp2021*"
char GIDEN[] = "hedef@hotmail.com"; // hedef eposta
char KONU[] = "EGS"; // konu = Ev Guvenlik Sistemi
char ICERIK[] = "Durum:\r\nEvinizde birisi var."; // mesaj

// GONDEREN ve PAROLA base64 ile sifrelenmeli,  https://www.base64encode.org/ linkini kullan
#define GONDEREN_64 "a2Fhbmd1dmVubGlrcHJvamVzaUBnbWFpbC5jb20="
#define PAROLA_64 "Y2ZrYnAyMDIx"

#define HOST "smtp.gmail.com" // SMTP giden server adi


#define PORT 587 // SMTP giden server port numarasi

int sayim = 0; // gonderilen kod sirasi takibi icin
bool bayrak; // eposta komutu gidecek mi - bool ilk deger 0'dir


void setup()
{
  pinMode(PIR1, INPUT); // pir1 pini veri girisi
  pinMode(PIR2, INPUT); // pir2 pini veri girisi
  pinMode(PIR3, INPUT); // pir3 pini veri girisi
  pinMode(ECH, INPUT);  // Sonar Echo pini veri girisi
  pinMode(TRG, OUTPUT); // Sonar Trigger pini veri cikisi
  pinMode(BUZ, OUTPUT); // Buzzer pini veri cikisi
  
  char buffer[50]; // kullanilacak karakter sayisi

  // esp versiyon -- v 0.9.2.4 ya da v 0.9.2.2
  wifi.setBootMarker(F("Version:0.9.2.4]\r\n\r\nready"));

  yazilim.begin(74880); // ESP8266'a yazilim baglantisi Normali 9600 olmali
  Serial.begin(57600); while(!Serial);  // seri baglantisi 57600 baslat

  Serial.println(F("ESP8266 - Eposta Atma"));

  // Donanimi kontrol et
  Serial.print(F("Donanim yeniden baslatiliyor..."));
  if(!wifi.hardReset())
  {
    Serial.println(F("Modulden yanit yok"));
    for(;;);
  }
  Serial.println(F("OK."));

  Serial.print(F("Yazilim yeniden baslatiliyor..."));
  if(!wifi.softReset())
  {
    Serial.println(F("Modulden yanit yok"));
    for(;;);
  }
  Serial.println(F("OK."));
 
  Serial.print(F("Firmware versiyonu kontrol ediliyor..."));
  wifi.println(F("AT+GMR"));

  if(wifi.readLine(buffer, sizeof(buffer)))
  {
    Serial.println(buffer);
    wifi.find(); // ikinci OK mesajini gozardi et - belirli surede hedef arama komutu
  }
  else
  {
    Serial.println(F("hata"));
  }

  Serial.print(F("WiFi'a baglaniyor..."));
  if(wifi.connectToAP(F(AgAdi),F(AgPar)))
  {
    // IP adres kontolu kutuphanede yok, manuel arayip string'e baglariz
    Serial.print(F("OK\nIP adresi kontrol ediliyor..."));
    wifi.println(F("AT+CIFSR"));
    if(wifi.readLine(buffer, sizeof(buffer)))
    {
      Serial.println(buffer);
      wifi.find();  // takip eden 'OK' i gozardi et - belirli sure hedef arama komutu

      Serial.print(F("Sunucuya baglaniyor..."));

      Serial.print("Baglandi...");
      wifi.println("AT+CIPMUX=0");  // tek baglanti ayarla - SMTP server icin tek baglanti olur

      wifi.find();  // belirli sure hedef arama komutu
      wifi.closeTCP(); // diger acik TCP baglantilarini kapat
      wifi.find();  // belirli sure hedef arama komutu
      Serial.println("Eposta atmak icin \"gonder\" yaz ");
    }
    else
    {
      // IP adres kontrolu basarisiz
      Serial.println(F("BASARISIZ"));
    }
  }
}

void loop()
{
  kapi = Sonar(TRG, ECH);
  durumPir = PIROku(PIR1, PIR2, PIR3);
  
  if(!bayrak)
  {
    // eposta gidecek mi kontrol et
    if(kapi > mesafe || durumPir )
    {
      // eger evde birisi var ya da kapi acildiysa bayrak ayarla
      bayrak = true;
      Serial.print("bayrak = ");
      Serial.println(bayrak);
      tone(BUZ, 196);   // 196 frekans ile Buzzer calmaya baasla
    }

    if(bayrak)
    {
      // eger gonderilecekse, SMTP komutlarini baslatiriz.
      if(siradan())
      {
        // siradaki komutu yap
        sayim++;  // siradaki komut icin sayimi artir
      }
    }
  }
}



// siradan() SMTP komutlarini sirasiyla gerçektestirip bool donutu verir.
boolean siradan()
{
  switch(sayim)
  {
    // sayim sirasina gore siradaki durum komutlarini calistir
    case 0:
    {
      // baslangic komutlari
      Serial.println("Baglaniyor...");
      // dogru sunucu ve porta bagli mi donut ver
      return wifi.connectTCP(F(HOST), PORT);
      break;
    }

    case 1:
    {
      // "HELO ip_adresi" komutu gonder. Sunucu "250" yaniti ile karsilik verecek
      // kaan.guvenlik.com yazdim ancak ip adresi ya da domain ismi de yazilabilir
      return wifi.cipSend("HELO kaan.guvenlik.com", F("250"));
      break;
    }
    
    case 2:
    {
      // "AUTH LOGIN" komutu gonder, sunucu "334 username"i base64 gonderecek
      // username = "VXNlcm5hbWU6" base64
      return wifi.cipSend("AUTH LOGIN", F("334 VXNlcm5hbWU6"));
      break;
    }

    case 3:
    {
      // kullanici adi/epostasini base64 gonder, sunucu base64 "334 password" isteyecek
      return wifi.cipSend(GONDEREN_64, F("334 UGFzc3dvcmQ6"));
      break;
    }
    
    case 4:
    {
      // parolayi base64 gonder, giris basarili olursa sunucu 235 diyecek
      return wifi.cipSend(PAROLA_64, F("235"));
      break;
    }

    case 5:
    {
      // "MAIL FROM:<GONDEREN>" komutu gonder
      char gonderen[50] = "MAIL FROM:<"; // 50 karakter yetmezse degistir
      strcat(gonderen, GONDEREN);
      strcat(gonderen, ">");
      
      // girdi kabul edilirse sunucu "250" yanitini verir.
      return wifi.cipSend(gonderen, F("250"));
      break;
    }

    case 6:
    {
      // "RCPT TO:<GIDEN>" komutu gonder
      char giden[50] = "RCPT TP:>"; // 50 karakter yetmezse degistir
      strcat(giden, GIDEN);
      strcat(giden, ">");

      // girdi kabul edilirse sunucu "250" yanitini verir.
      return wifi.cipSend(giden, F("250"));
      break;
    }

    case 7:
    {
      // "DATA" komutu gonder, sunucu "334 end message with \r\n.\r\n." diyecek.
      // mesajini "\r\n.\r\n." ile bitir diyor

      // tum veri aktarildiginda sunucu "354" der
      return wifi.cipSend("DATA", F("354"));
      break;
    }

    case 8:
    {
      // basliga "FROM: gonderen <GONDEREN>" koy
      char gonderen[160] = "FROM: ";
      strcat(gonderen, GONDEREN);
      strcat(gonderen, " ");
      strcat(gonderen, "<");
      strcat(gonderen, GONDEREN);
      strcat(gonderen, ">");

      return wifi.cipSend(gonderen);
      break;
    }

    case 9:
    {
      // basliga "TO: " (kime) koy
      char to[100] = "TO: ";
      strcat(to, GIDEN);
      strcat(to, "<");
      strcat(to, GIDEN);
      strcat(to, ">");

      return wifi.cipSend(to);
      break;
    }

    case 10:
    {
      // SUBJECT (konu) basligi ekleme
      char konu[80] = "SUBJECT: ";
      strcat(konu, KONU);

      return wifi.cipSend(konu);
      break;
    }

    case 11:
    {
      // baslik girizgahinin bitimini gosterir. (Konu, Gonderen, Kime, vb)
      return wifi.cipSend("\r\n");
      break;
    }

    case 12:
    {
      // icerik
      return wifi.cipSend(ICERIK);
      break;
    }

    case 13:
    {
      // veri bitimini belirtir
      return wifi.cipSend("\r\n.");
      break;
    }

    case 14:
    {
      // Cikis yap
      return wifi. cipSend("QUIT");
      break;
    }

    case 15:
    {
      // baglantiyi kapat
      wifi.closeTCP();
      return true;
      break;
    }

    case 16:
    {
      Serial.println("Islem Tamamlandi");
      bayrak = false; // gonderim yok ayarla
      sayim = 0; // islem sirasini yeniden baslat
      noTone(BUZ);  // Buzzer'i kapatiyoruz
      return false; // sayimi artirmaya devam etmemek icin
      break;
    }

    default:
    {
      // bos yap
      break;
    }
  }
}
