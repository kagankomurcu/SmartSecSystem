#include <arduino.h>
int Sonar(int TRG, int ECH)
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

bool PIROku(int PIR1, int PIR2, int PIR3)
{
  delay(1000);
  bool durum1 = digitalRead(PIR1);
  bool durum2 = digitalRead(PIR2);
  bool durum3 = digitalRead(PIR3);

  if(durum1 || durum2 || durum3)
  {
    return true;
  }

  else
  {
    return false;
  }
}
