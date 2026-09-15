# Değişiklik Günlüğü

## 3.0.1 — Beta: Bellek Güvenliği ve Editör Eklentisi İyileştirmeleri

### Derleyici ve Çalışma Zamanı

- **Dizi Sınır Denetimi:** Sabit dizi erişimlerinde sınır aşımları artık derleme anında tespit edilir. Değişken indeksli erişimler ise çalışma anında `upp_dizi_idx` ile denetlenir; belirsiz davranışlar engellenerek güvenli biçimde panik üretilir.
- **İş Parçacığı Güvenliği:** UTF-8 çözümleme ve metin birleştirme önbellekleri iş parçacığına özel (`thread-local`) hale getirildi. Yoğun `arkaplan` kullanımındaki veri yarışı (data race) riskleri ortadan kaldırıldı.
- **Kaynak Yönetimi:** İfade içinde oluşturulan `ArkaplanIs` iş parçacığı tutamaçları takibe alındı. `bekle()` çağrıldığında kayıt serbest bırakılır, süreç sonlandığında ise açıkta kalan tüm tutamaçlar otomatik temizlenir.
- **Dinamik Liste / Harita:** `liste` ve `harita` büyütmede `nc *= 2` taşması panik üretir (`_upp_liste_buyut`, `_upp_harita_buyut`). Sınır dışı erişim ve yetersiz bellek `upp_panik` ile biter. Eski `havuz_impl.h` (Python bootstrap) SoA geçişinde yok; bu korumalar C çalışma zamanındadır.
- **Intern sıfırlama:** `--analiz` oturumunda `havuz_sifirla` intern haritasını ve dizge listesini `bosalt()` ile serbest bırakır.
- **c_kod önişlemci:** `c_on_islemci` gömülü C'yi satır satır `upp.metin.kes` ile tarar; 4095 baytlık kesme (eski `havuz_impl.h`) yoktur.
- **Büyük Dosya Desteği:** `upp.dosya_oku` 64 bit (`_ftelli64` / `ftello`).
- **Güvenlik İhlali Tespiti:** Güvenli modda işaretçi bildirimi (çağrı sızıntısı dahil) bellek güvenliği ihlalidir.

### Editör Eklentisi (VS Code / Cursor)

- **Tanı Senkronizasyonu:** Eski `--analiz` çıktıları Problems paneline yazılmaz.
- **Güvenli F5 Akışı:** Derle ve çalıştır ayrı görevler; `$LASTEXITCODE` birleşik satırı yok.
- **Kayıt Güvenliği:** `save() === false` derlemeyi iptal eder.
- **Çakışma Önleme:** `src/main.upp` ile `test/main.upp` çıktıları `derleme/` altında ayrı gövde alır.
- **Durum Çubuğu Filtresi:** Yalnızca `u++` / `u++ güvenlik`.
- **Eksik Hata Koruması:** `ok: false` ve `hatalar` yoksa sentetik tanı.
- **Biçimlendirme Konumu:** `cwd` açık belgenin klasörü.
- **Hata Bildirimleri:** `showErrorMessage`.
- **Oturum Temizliği:** Motor düşünce yalnızca açık u++ belgelerinin tanısı temizlenir.

### Bootstrap ve CI

- **Aşama-0 C:** `bootstrap/uppc.c` repoda. Temiz checkout: `gcc bootstrap/uppc.c` → `derleyici/uppc`. Python bootstrap yok.
- **Native test CI:** Bootstrap'ı gcc ile derler, sonra `python tests/run_tests.py` çalıştırır. İkili yok diye yeşil atlama yoktur.
- **TSAN:** Linux'ta aynı bootstrap ikilisiyle `UPP_SANITIZE=-fsanitize=thread` altında runtime ve yoğun arkaplan.
- **Çalışma Zamanı Testleri:** `tests/runtime/` derle + sıfır olmayan çıkış + stderr.
