# Değişiklik Günlüğü

## 3.0.1 — Beta: Bellek Güvenliği ve Editör Eklentisi İyileştirmeleri

### Derleyici ve Çalışma Zamanı

- **Dizi Sınır Denetimi:** Sabit dizi erişimlerinde sınır aşımları artık derleme anında tespit edilir. Değişken indeksli erişimler ise çalışma anında `upp_dizi_idx` ile denetlenir; belirsiz davranışlar engellenerek güvenli biçimde panik üretilir.
- **İş Parçacığı Güvenliği:** UTF-8 çözümleme ve metin birleştirme önbellekleri iş parçacığına özel (`thread-local`) hale getirildi. Yoğun `arkaplan` kullanımındaki veri yarışı (data race) riskleri ortadan kaldırıldı.
- **Kaynak Yönetimi:** İfade içinde oluşturulan `ArkaplanIs` iş parçacığı tutamaçları takibe alındı. `bekle()` çağrıldığında kayıt serbest bırakılır, süreç sonlandığında ise açıkta kalan tüm tutamaçlar otomatik temizlenir.
- **Dinamik Liste Güvenliği:** `liste` yapısında sınır dışı erişim, yetersiz bellek, metin taşması ve `_upp_liste_buyut` kapasite aşımları doğrudan `upp_panik` çağrısıyla kontrollü şekilde sonlandırılır.
- **Büyük Dosya Desteği:** `upp.dosya_oku` fonksiyonu 64 bit dosya boyutlarını destekleyecek şekilde güncellendi (`_ftelli64` / `ftello`).
- **Analiz Belleği:** Canlı düzenleyici analizinde (`--analiz`) her oturum başında dize havuzu (`intern`) `havuz_sifirla` ile sıfırlanarak bellek sızıntıları önlendi.
- **Güvenlik İhlali Tespiti:** Güvenli modda doğrudan bildirimlerin yanı sıra, çağrılar veya tür dönüşümleri üzerinden sızan tüm işaretçi kullanımları da bellek güvenliği ihlali kapsamına alındı.

### Editör Eklentisi (VS Code / Cursor)

- **Tanı Senkronizasyonu:** Eski `--analiz` çıktıları "Sorunlar" (Problems) panelinde birikmez; panel her zaman güncel dosya durumunu yansıtır.
- **Güvenli F5 Akışı:** Hata ayıklama ve çalıştırma akışı birbirinden bağımsız iki aşamaya ayrıldı; derleme başarıyla tamamlanmadan ikili dosya yürütülmez. Kararsız çalışan `$LASTEXITCODE` komut zinciri kaldırıldı.
- **Kayıt Güvenliği:** Dosya diske kaydedilemediği durumlarda (`save() === false`) derleme işlemi otomatik olarak iptal edilir.
- **Çakışma Önleme:** Farklı dizinlerdeki (örneğin `src/` ve `test/`) aynı ada sahip dosyaların çıktıları, `derleme/` klasöründe artık birbirinin üzerine yazılmaz.
- **Durum Çubuğu Filtresi:** Durum çubuğundaki hata sayacı yalnızca `u++` ve `u++ güvenlik` kaynaklı bildirimleri dikkate alır.
- **Eksik Hata Koruması:** Derleyicinin `ok: false` döndüğü ancak hata ayrıntısı iletmediği durumlar için sentetik tanı mekanizması eklendi.
- **Biçimlendirme Konumu:** Kod biçimlendirici (`Format Document`), çalışma dizini olarak doğrudan açık olan belgenin klasörünü temel alır.
- **Hata Bildirimleri:** Süreç yürütme hataları editör arayüzünde `showErrorMessage` bildirim penceresiyle gösterilir.
- **Oturum Temizliği:** Dil sunucusu kapatıldığında yalnızca u++ ile ilişkili belgelerin tanı kayıtları temizlenir.

### Test ve Sürekli Entegrasyon (CI)

- **Çalışma Zamanı Testleri:** `tests/runtime/` test paketi eklendi; kaynak dosyalar derlenip çalıştırılarak sıfır harici çıkış kodları ve `stderr` çıktıları otomatik doğrulanır.
- **Yerel Derleyici Esnekliği:** Yerel derleyici ikilileri (`derleyici/uppc*`) bulunamadığında ilgili test adımları sessizce atlanır.
- **İş Parçacığı Denetimi (TSAN):** Linux ortamında `UPP_SANITIZE` bayrağı ile ThreadSanitizer etkinleştirilerek çalışma zamanı ve yoğun `arkaplan` testleri güvenceye alındı.
