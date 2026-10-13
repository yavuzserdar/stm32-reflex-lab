# STM32 Reflex Lab — İlk çalıştırma

Kart ve USB veri kablosu yeterli. Ek sensör veya bağlantı gerekmez.

1. Kartın **ST-LINK USB girişini** bilgisayara bağla.
2. VS Code'da **File → Open Folder** seçeneğiyle
   `C:\CSTM32workspace\stm32-reflex-lab` klasörünü aç.
3. **Ctrl+Shift+B** ile derle. Terminalde `Build OK` yazısını görmelisin.
4. **Terminal → Run Task → Reflex: Flash** görevini çalıştır. Programı karta
   yükler, doğrular ve kartı yeniden başlatır.
5. **Terminal → Run Task → Reflex: Monitor** görevini çalıştır. COM portunu gir;
   tek port varsa boş bırakabilirsin. Portu öğrenmek için `Reflex: List Ports`
   görevini kullan. Port adı bilgisayarına göre değişebilir; örneğin `COM4`.
6. Monitör açıldıktan sonra kartın siyah **RESET** düğmesine bir kere bas.
   Terminalde `STM32 Reflex Lab` ve `READY` yazılarını görmelisin.
7. Mavi **B1 USER** düğmesine basıp bırak. Yeşil **LD2** yanınca yeniden bas.

Her turdan sonra düğmeyi bırak. Sonraki tur otomatik başlar. Erken basarsan veya
LED yandıktan sonra 5 saniye içinde basmazsan aynı tur tekrarlanır. 10 geçerli
turun sonunda terminalde en iyi ve ortalama süren görünür. Yeniden oynamak için
mavi düğmeye bas. Monitörü durdurmak için **Ctrl+C** kullan.

Sonuçlar `results/` klasörüne CSV olarak kaydedilir. Oynamak ve sonuçları görmek
için Python gerekmez. İsteğe bağlı grafik için README'deki Python adımlarını izle.

## Bir şey çalışmazsa

- **COM portu yok:** USB kablosunun veri ilettiğini ve ST-LINK girişine takılı
  olduğunu kontrol et. Gerekirse ST-LINK sürücüsünü kur.
- **Port açılamıyor:** Aynı portu kullanan başka seri monitörü kapat.
- **Karta yüklenmiyor:** Diğer hata ayıklama oturumlarını kapat; birden fazla
  ST-LINK bağlıysa yalnızca bu kartı bağlı bırak.
- **LED yanmıyor:** Önce mavi B1'e basıp bırak; rastgele bekleme 2–5 saniyedir.
  Terminalde `WAIT` durumunu görmelisin.
- **Derleyici bulunamadı:** Bu bilgisayarda STM32CubeIDE `C:\ST` altında bulundu.
  Başka bir bilgisayarda kurulum yolu farklıysa README'deki ortam değişkenlerini ayarla.
