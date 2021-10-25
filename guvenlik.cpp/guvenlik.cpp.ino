/* wiFiEmail - Ev Guvenlik Programi
*/

#include <arduino.h>
int Sonar(int TRG, int ECH, bool bayrak)
{
  if(!bayrak)
  {
    digitalWrite(TRG, LOW);  // once sinyal icin elektrigi kapat
    delayMicroseconds(2);   // milyonda iki saniye bekle
    digitalWrite(TRG, HIGH);  // Sinyal gonder
    delayMicroseconds(5);   // milyonda bes saniye surdur
    digitalWrite(TRG, LOW); // Sinyali kapat
    
    int sure = pulseIn(ECH, HIGH);  // yanki gelis suresini olc
    
    int ara = sure / 29 / 2;  // sureye gore arayi hesapla
    
    Serial.print(ara);  // hesaplanan rakami monitore yaz
    Serial.println(" cm");  // ara cinsini yaz alt satra gec
    
    delay(100); // islemi saniyede 10 kere tekrarla
    return ara;    
  }
  
}

bool PIROku(int PIR1, int PIR2, int PIR3, bool bayrak)
{
  if(!bayrak)
  {
    bool durum1, durum2, durum3;
    durum1 = 0;
    durum2 = 0;
    durum3 = 0;
    delay(1000);
    durum1 = digitalRead(PIR1);
    durum2 = digitalRead(PIR2);
    durum3 = digitalRead(PIR3);
    Serial.print("durum1: ");  // durum1
    Serial.println(durum1);  // 1 ve 2 Sensör hareket algıladı
    Serial.print("durum2: ");  // durum1
    Serial.println(durum2);  // 1 ve 3 sensör hareket algıladı
    Serial.print("durum3: ");  // durum1
    Serial.println(durum3);  // 2 ve 3 Sensör Hareket Algıladı
    
    if(durum1 || durum2 || durum3)
    {
      return true;
    }
    
    else
    {
      return false;
    }    
  }
}
