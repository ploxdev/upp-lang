/* u++ bootstrap derleyici — kalıcı C.
 * gcc bootstrap/uppc.c → derleyici/uppc[.exe]
 * Çalışma zamanı src/runtime/upp_runtime.c (tek çeviri birimi, gömülü kopya yok).
 * Derleyici gövdesi src/uppc/*.upp karşılığıdır.
 */
#if defined(_WIN32)
#define UPP_HEDEF_WINDOWS 1
#else
#ifndef UPP_HEDEF_LINUX
#define UPP_HEDEF_LINUX 1
#endif
#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include "../src/runtime/upp_runtime.c"

typedef enum JetonTur {
    JetonTur_J_EOF = 0,
    JetonTur_J_ANAHTAR = 1,
    JetonTur_J_KIMLIK = 2,
    JetonTur_J_METIN = 3,
    JetonTur_J_SAYI = 4,
    JetonTur_J_ONDALIK = 5,
    JetonTur_J_C_GOMMU = 6,
    JetonTur_J_ASM_GOMMU = 7,
    JetonTur_J_YAG_OK = 8,
    JetonTur_J_OK = 9,
    JetonTur_J_ESIT_ESIT = 10,
    JetonTur_J_ESIT_DEGIL = 11,
    JetonTur_J_KUCUK_ESIT = 12,
    JetonTur_J_BUYUK_ESIT = 13,
    JetonTur_J_VE = 14,
    JetonTur_J_VEYA = 15,
    JetonTur_J_ARTI = 16,
    JetonTur_J_EKSI = 17,
    JetonTur_J_YILDIZ = 18,
    JetonTur_J_BOLU = 19,
    JetonTur_J_MOD = 20,
    JetonTur_J_ATAMA = 21,
    JetonTur_J_KUCUK = 22,
    JetonTur_J_BUYUK = 23,
    JetonTur_J_UNLEM = 24,
    JetonTur_J_VE_ISARET = 25,
    JetonTur_J_VAY_ISARET = 26,
    JetonTur_J_XOR = 27,
    JetonTur_J_PARANTEZ_AC = 28,
    JetonTur_J_PARANTEZ_KAPA = 29,
    JetonTur_J_SLUS_AC = 30,
    JetonTur_J_SLUS_KAPA = 31,
    JetonTur_J_KOSE_AC = 32,
    JetonTur_J_KOSE_KAPA = 33,
    JetonTur_J_NOKTALI_VIRGUL = 34,
    JetonTur_J_VIRGUL = 35,
    JetonTur_J_NOKTA = 36,
    JetonTur_J_IKI_NOKTA = 37,
    JetonTur_J_ARTI_ATAMA = 38,
    JetonTur_J_EKSI_ATAMA = 39,
    JetonTur_J_YILDIZ_ATAMA = 40,
    JetonTur_J_BOLU_ATAMA = 41,
    JetonTur_J_MOD_ATAMA = 42,
    JetonTur_J_SORU = 43
} JetonTur;

typedef enum DugumTur {
    DugumTur_D_YOK = 0,
    DugumTur_D_PROGRAM = 1,
    DugumTur_D_CEKLE = 2,
    DugumTur_D_SECENEK = 3,
    DugumTur_D_SECENEK_UYE = 4,
    DugumTur_D_SINIF = 5,
    DugumTur_D_ALAN = 6,
    DugumTur_D_FONK = 7,
    DugumTur_D_PARAM = 8,
    DugumTur_D_TUR = 9,
    DugumTur_D_BLOK = 10,
    DugumTur_D_GUVENSIZ = 11,
    DugumTur_D_ARKAPLAN = 12,
    DugumTur_D_ARKAPLAN_IFADE = 13,
    DugumTur_D_EGER = 14,
    DugumTur_D_IKEN = 15,
    DugumTur_D_DONGU = 16,
    DugumTur_D_SECIM = 17,
    DugumTur_D_DURUM = 18,
    DugumTur_D_HER = 19,
    DugumTur_D_DON = 20,
    DugumTur_D_DUR = 21,
    DugumTur_D_DEVAM = 22,
    DugumTur_D_IFADE_CUMLE = 23,
    DugumTur_D_BOS_CUMLE = 24,
    DugumTur_D_C_GOMMU = 25,
    DugumTur_D_ASM_GOMMU = 26,
    DugumTur_D_DEGISKEN = 27,
    DugumTur_D_ATAMA = 28,
    DugumTur_D_IKIL = 29,
    DugumTur_D_TEKIL = 30,
    DugumTur_D_CAGRI = 31,
    DugumTur_D_INDIS = 32,
    DugumTur_D_UYE = 33,
    DugumTur_D_KIMLIK = 34,
    DugumTur_D_SABIT = 35,
    DugumTur_D_DIZI_SABIT = 36,
    DugumTur_D_KULLAN = 37,
    DugumTur_D_UCLU = 38,
    DugumTur_D_GUVENLI = 39
} DugumTur;


/* ---- prototipler ---- */
void intern_hazir(void);
char* metin_kopya_yeni(char* s);
void intern_sifirla(void);
char* intern(char* s);
char* derleme_kaynak_ad(void);
bool metin_esit(char* a, char* b);
long long metin_uzun(char* s);
long long metin_cp(char* s, long long i);
long long metin_adv(char* s, long long i);
char* intern_dilim(char* s, long long bas, long long son);
char* kaynak_lf(char* s);
char* sayi_metin(long long n);
char* kat(char* a, char* b);
char* kat3(char* a, char* b, char* c);
char* kat4(char* a, char* b, char* c, char* d);
char* kat5(char* a, char* b, char* c, char* d, char* e);
char* c_escape(char* s);
long long hex_hane_deger(long long c);
long long hex_deger(char* lex);
long long sayi_oku(char* lex);
char* satir_kirp_sol(char* s);
bool c_on_islemci(char* kod);
bool ident_bas(long long cp);
bool ident_devam(long long cp);
void dugum_hazir(void);
bool dugum_ok(long long id);
long long dugum_yeni(long long tur, long long satir, long long sutun);
long long dugum_tur(long long id);
long long dugum_satir(long long id);
long long dugum_sutun(long long id);
char* dugum_ad(long long id);
char* dugum_kaynak(long long id);
long long dugum_sayi(long long id);
long long dugum_a(long long id);
long long dugum_b(long long id);
long long dugum_c(long long id);
long long dugum_d(long long id);
long long dugum_liste(long long id);
long long dugum_liste2(long long id);
long long dugum_ptrd(long long id);
long long dugum_dims(long long id);
long long dugum_bayrak(long long id);
void dugum_yaz_ad(long long id, char* ad);
void dugum_yaz_sayi(long long id, long long v);
void dugum_yaz_a(long long id, long long v);
void dugum_yaz_b(long long id, long long v);
void dugum_yaz_c(long long id, long long v);
void dugum_yaz_d(long long id, long long v);
void dugum_yaz_liste(long long id, long long v);
void dugum_yaz_liste2(long long id, long long v);
void dugum_yaz_ptrd(long long id, long long v);
void dugum_yaz_dims(long long id, long long v);
void dugum_yaz_bayrak(long long id, long long v);
void dugum_yaz_tur(long long id, long long v);
void liste_hazir(void);
bool liste_ok(long long id);
void liste_buyut(long long id);
long long liste_yeni(void);
void liste_ekle(long long id, long long oge);
long long liste_uzunluk(long long id);
long long liste_al(long long id, long long i);
void liste_yaz(long long id, long long i, long long v);
void liste_kisalt(long long id);
void jeton_hazir(void);
bool jeton_ok(long long id);
long long jeton_yeni(long long tur, char* deger, long long satir, long long sutun);
long long jeton_tur(long long id);
char* jeton_deger(long long id);
long long jeton_satir(long long id);
long long jeton_sutun(long long id);
long long jeton_sayisi(void);
void jeton_sifirla(void);
void kaynak_ayarla(char* s, char* ad);
void lex_kaynak_ad_guncelle(char* ad);
void lex_yorum_govde(char* govde);
char* lex_kaynak_metin(void);
long long lex_cp(void);
bool lex_bitis(void);
long long lex_pos(void);
long long lex_satir(void);
long long lex_sutun(void);
long long lex_ilerle(void);
char* lex_dilim(long long bas, long long son);
long long lex_cp_ileri(long long kac);
void tampon_sifirla(void);
void tampon_yaz(char* s);
void tampon_yaz_sayi(long long n);
char* tampon_al(void);
void bg_yigin_sifirla(void);
void bg_yigin_ekle(char* s);
char* bg_yigin_al(void);
void emit(char* s);
void emit_hedef_yaz(long long v);
void os_hedef_yaz(long long v);
long long os_host_linux(void);
long long os_hedef_al(void);
char* os_tirnak(char* s);
void indent_ekle(long long d);
long long indent_al(void);
void indent_kur(long long v);
long long ayristir_i_al(void);
void ayristir_i_yaz(long long v);
long long loop_al(void);
void loop_yaz(long long v);
long long switch_al(void);
void switch_yaz(long long v);
long long bg_al(void);
void bg_yaz(long long v);
long long tmp_yeni(void);
void tmp_sifirla(void);
long long bgid_yeni(void);
void cur_fn_yaz(long long v);
long long cur_fn_al(void);
void cur_class_yaz(char* s);
char* cur_class_al(void);
void kapsam_sifirla(void);
void kapsam_ac(void);
void kapsam_kapat(void);
void kapsam_bagla(char* ad, long long ty);
long long kapsam_bul(char* ad);
bool kapsam_yerel_var(char* ad);
void tablo_sifirla(void);
long long tablo_yaz(long long kind, char* ad, long long id);
long long tablo_bul(long long kind, char* ad);
long long tablo_sinif_sayisi(void);
char* tablo_sinif_ad(long long i);
long long tablo_sinif_id(long long i);
bool os_dosya_yaz(char* yol, char* data);
bool os_dosya_var(char* yol);
char* os_yol_dizin(char* yol);
char* os_yol_birlestir(char* dir, char* rel);
char* os_yol_gercek(char* yol);
char* os_cwd(void);
bool os_yol_mutlak_mi(char* yol);
char* os_runtime_dene(char* kok);
char* os_runtime_bul(void);
void os_runtime_ozel_yaz(char* yol);
char* os_runtime_oku(void);
char* os_gcc_aday_dene(char* p);
char* os_gcc_bul(void);
bool os_yol_bosluk_var(char* s);
char* os_cmd_parca(char* s);
long long os_gcc_cagir(char* gcc, char* c_yol, char* exe_yol);
char* os_gcc_cikti(void);
long long os_program_calistir(char* yol);
bool bicim_icinde(char* s, char* ara);
char* bicim_kaynak(char* src);
char* bicim_norm(char* ham);
long long os_bicim_dosya(char* yol, long long kontrol);
void havuz_sifirla(void);
bool hata_var(void);
void hata_sifirla(void);
void hata_ver(long long satir, long long sutun, char* mesaj);
void hata_guvenlik(long long satir, long long sutun, char* mesaj);
void hata_bas(void);
void hata_fmt_basla(void);
void hata_fmt_yaz(char* s);
void hata_fmt_sayi(long long n);
char* hata_fmt_al(void);
void hata_fmt_ver(long long satir, long long sutun);
void hata_fmt_guvenlik(long long satir, long long sutun);
void hata_json_mod_ayarla(long long v);
bool hata_json_mod(void);
char* json_hex_hane(long long n);
char* json_kacis(char* s);
char* json_hata_nesne(long long satir, long long sutun, char* tur, char* mesaj);
void hata_json_yaz(long long satir, long long sutun, char* tur, char* mesaj);
void hata_json_ok(void);
void hata_json_bas(void);
void hata_analiz_topla_ayarla(long long v);
long long hata_analiz_say(void);
bool hata_analiz_mi(void);
bool dugum_tur_mi(long long id, long long tur);
long long tur_dugum(long long satir, long long sutun, char* ad, long long ptrd, long long dims);
long long tur_kopya(long long t);
char* tur_aciklama(long long t);
long long sabit_sayi(long long satir, long long sutun, long long v);
long long kimlik_dugum(long long satir, long long sutun, char* ad);
bool anahtar_mi(char* s);
bool tur_adi_mi(char* s);
bool koleksiyon_elem_mi(char* s);
bool liste_tur_mi(char* n);
char* liste_elem_ad(char* n);
bool harita_tur_mi(char* n);
char* harita_anahtar_ad(char* n);
char* harita_deger_ad(char* n);
long long koleksiyon_etiket(char* e);
bool interp_ad_mi(char* name);
bool yerlesik_sinif_mi(char* s);
char* c_sembol(char* ad, char* alias);
char* nokta_altcizgi(char* n);
char* dugum_alias_ad(long long n);
char* dugum_c_ad(long long n);
char* sinif_tur_ad(long long cls);
char* metot_sinif_tur_ad(long long fn);
char* metot_sinif_c_ad(long long fn);
void lex_yorum_satir(void);
void lex_blok_yorum(void);
void lex_bosluk_at(void);
bool lex_sonraki_bosluksuz_suslu(void);
long long lex_gomulu(long long tur, long long satir, long long sutun);
long long lex_metin(void);
long long lex_sayi(void);
void lex_tara(void);
long long jeton_id_at(long long off);
long long bak_tur(void);
char* bak_deger(void);
long long bak_satir(void);
long long bak_sutun(void);
long long bak_tur_at(long long off);
char* bak_deger_at(long long off);
long long j_ilerle(void);
bool esles_tur(long long t);
bool esles_kw(char* w);
bool kw_mi(char* w);
long long beklenen_tur(long long t, char* goster);
long long beklenen_kw(char* w);
long long beklenen_kimlik(bool tur_kw_izin);
char* metin_coz(char* raw);
bool tur_baslangici_mi(void);
char* ayristir_koleksiyon_elem(void);
long long ayristir_tur(void);
long long ayristir_dizi_sabit(void);
long long ayristir_baslatici(void);
long long ayristir_birincil(void);
long long ayristir_sonek(void);
long long ayristir_tekil(void);
long long ayristir_carpan(void);
long long ayristir_terim(void);
long long ayristir_karsilastirma(void);
long long ayristir_esitlik(void);
long long ayristir_bit_ve(void);
long long ayristir_xor(void);
long long ayristir_bit_veya(void);
long long ayristir_ve(void);
long long ayristir_veya(void);
long long ayristir_uclu(void);
long long ayristir_atama(void);
long long ayristir_ifade(void);
long long ayristir_param(void);
long long ayristir_alan(void);
long long ayristir_oto(void);
long long ayristir_degisken_bayrak(long long sabit_mi);
long long ayristir_degisken(void);
long long ayristir_sabit(void);
long long ayristir_blok(void);
long long ayristir_eger(void);
long long ayristir_iken(void);
long long ayristir_dongu(void);
long long ayristir_secim_kol(void);
long long ayristir_secim(void);
long long ayristir_her(void);
long long ayristir_cumle(void);
long long ayristir_fonksiyon(char* sinif_ad);
bool sinif_uye_var(long long fields, long long methods, char* ad);
long long ayristir_sinif(void);
long long ayristir_secenek(void);
long long ayristir_kullan(void);
long long ayristir_include(void);
long long ayristir_birim(void);
long long ayristir_program(void);
long long sabit_tur_kodu(long long n);
long long tip_bos(long long satir, long long sutun);
long long tip_sayi(long long satir, long long sutun);
long long tip_bayt(long long satir, long long sutun);
long long tip_ondalik(long long satir, long long sutun);
long long tip_metin(long long satir, long long sutun);
long long tip_mantik(long long satir, long long sutun);
void cift_tanim_ver(long long eski, long long yeni, char* ad);
bool tip_void_mi(long long t);
bool tip_yok_mi(long long t);
bool tip_metin_mi(long long t);
bool tip_tam_sayi_mi(long long t);
void arg_tam_sayi_olmali(long long e);
bool tip_sayisal_mi(long long t);
bool tip_sinif_deger_mi(long long t);
bool tip_enum_mi(long long t);
bool tip_num_genis(long long t);
bool tip_atanir(long long dst, long long src);
bool lvalue_mi(long long e);
long long tip_yerlesik_sinif(char* ad, char* a, char* ta, char* b, char* tb, char* c, char* tc);
void tip_uyelik_yolu(long long e, long long i, long long L);
long long upp_yol(long long e);
bool sayilabilir_dizi_mi(long long e);
bool secim_uygun_tur(long long t);
bool secim_sabit_mi(long long e);
long long bg_ad_listesi_kopya(long long L);
bool bg_liste_ad_var(long long L, char* ad);
void bg_ident_ekle(long long idn, long long declared, long long used, long long yer);
void bg_interp_topla(long long e, long long declared, long long used, long long yer);
void bg_ident_topla(long long n, long long declared, long long used, long long yer);
void bg_yakala(long long node);
bool sabit_dogru_mu(long long e);
bool her_zaman_doner(long long stmt);
void tip_interp(long long e);
long long tip_liste_uye(long long expr, char* on);
long long tip_harita_uye(long long expr, char* on);
long long tip_uye(long long expr, bool as_call);
long long upp_donus(char* name, long long satir, long long sutun);
bool json_ad_mi(char* name);
bool json_arity_ok(char* name, long long n);
long long json_donus(char* name, long long satir, long long sutun);
long long upp_ns_donus(char* ns, char* name, long long n, long long satir, long long sutun);
long long tip_liste_cagri(long long expr, char* on);
long long tip_harita_cagri(long long expr, char* on);
bool tip_args_ok(long long P, long long args, long long satir, long long sutun);
long long tip_kurucu(long long cls, long long expr);
long long tip_cagri(long long expr);
long long tip_ikil(long long expr);
bool sabit_tamsayi_mi(long long e);
long long sabit_tamsayi_deger(long long e);
long long tip_ifade(long long e);
bool sabit_ifade_mi(long long e);
void kapsam_bagla_yeni(char* ad, long long ty, long long satir, long long sutun);
void tip_yuru(long long stmt);
void tip_fn_doldur(long long fn);
void tip_fn_govde(long long fn);
void tip_calistir(long long program);
void guven_yuru(long long node, bool unsafe);
void guven_kontrol(long long program);
char* uret_ctur(long long t);
char* uret_args_fn(long long fn, long long args);
char* uret_kurucu(long long cls, long long args);
char* uret_args(long long L);
char* uret_sabit(long long e);
bool interp_var_mi(char* text);
char* yuzde_kacis(char* s);
long long interp_expr_kur(char* name, long long satir, long long sutun);
char* uret_printf_spec(long long t);
char* uret_interp_arg(long long expr);
char* uret_interp_printf(long long lit, bool newline, char* akis);
char* uret_interp_heap(long long lit);
char* uret_metin_sabit(long long e);
char* uret_kol_pack(char* elem, char* cval);
char* uret_liste_al(char* elem, char* recv, char* idx);
char* uret_harita_key(char* ktur, long long e);
char* uret_liste_cagri(long long expr, char* on);
char* uret_harita_al(char* vtur, char* recv, char* kpack);
char* uret_harita_cagri(long long expr, char* on);
char* uret_uye(long long e);
char* uret_upp_cagri(char* name, long long args, long long satir, long long sutun);
char* uret_dizi_ptr_say(long long e, char* elem_c);
char* uret_ns_cagri(char* ns, char* name, long long args);
char* uret_metot(long long expr);
char* uret_dizi_idx(long long arr_t, long long idx_e);
char* uret_ifade(long long e);
char* uret_bg_sifir(long long t);
char* uret_bg_ctx_ad(long long id);
char* uret_bg_fn_ad(long long id);
char* uret_bg_sp_ad(long long id);
void uret_bg_kopya_alan(char* hedef, long long cap);
void uret_bg_metin_ctx_birak(long long caps, char* p);
char* uret_bg_kopya_hata_kosul(char* hedef, long long caps);
char* uret_bg_param_listesi(long long caps);
char* uret_bg_arg_listesi(long long caps);
char* uret_bg_ifade(long long e);
bool linux_mi(void);
void uret_bg_isci(long long node);
void uret_bg_deyim(long long stmt);
void bg_yuru(long long n, long long kip);
void uret_govde(long long stmt);
bool kilit_hazir_gerekli(long long d);
void uret_var(long long d, bool kuresel);
void uret_cumle(long long stmt);
char* uret_prototip(long long decl);
void uret_fonksiyon(long long decl);
void uret_program(long long program);
void os_runtime_gomulu_yay(void);
bool arg_esit(char* a, char* b);
long long harf_kucuk(long long c);
bool kutup_uzanti_mi(char* s);
bool baslik_mi(char* s);
bool liste_ad_var(long long L, char* s);
void liste_ad_ekle(long long L, char* s);
char* kullan_coz(char* spec, char* dahil_yol, long long satir, long long sutun);
void yukle_birim(char* kaynak, char* yol, long long kutuphane, long long seen, long long stack, long long merged, char* alias);
long long yukle_birlesik(char* kaynak, char* ad, long long ana_zorunlu);
long long analiz_program(char* kaynak, char* ad, long long cok_hata);
bool derle_kaynak(char* kaynak, char* ad);
void ast_yaz(long long node, long long derinlik);
long long havuz_sinir_test(void);
long long harf_kucuk_ascii(long long c);
bool yol_son_esit(char* s, char* uz);
char* yol_uzanti_kes(char* s);
char* dil_surum(void);
long long dil_protokol(void);
void surum_yazdir(bool json);
int main(int argc, char **argv);

/* ---- u++ üst düzey değişkenler ---- */
static
long long g_os_hedef = (-1LL);
static
char* g_gcc_log = NULL;
static
UppKolHarita intern_map = {0};
static
UppKolListe intern_metin = {0};
static
long long intern_n = 0LL;
static
UppKolListe d_tur = {0};
static
UppKolListe d_satir = {0};
static
UppKolListe d_sutun = {0};
static
UppKolListe d_ad = {0};
static
UppKolListe d_kaynak = {0};
static
UppKolListe d_sayi = {0};
static
UppKolListe d_a = {0};
static
UppKolListe d_b = {0};
static
UppKolListe d_c = {0};
static
UppKolListe d_d = {0};
static
UppKolListe d_liste = {0};
static
UppKolListe d_liste2 = {0};
static
UppKolListe d_ptrd = {0};
static
UppKolListe d_dims = {0};
static
UppKolListe d_bayrak = {0};
static
long long d_n = 0LL;
static
UppKolListe l_ofset = {0};
static
UppKolListe l_uzunluk = {0};
static
UppKolListe l_kap = {0};
static
UppKolListe l_veri = {0};
static
long long l_n = 0LL;
static
UppKolListe l_bos_of = {0};
static
UppKolListe l_bos_kap = {0};
static
UppKolListe j_tur = {0};
static
UppKolListe j_deger = {0};
static
UppKolListe j_satir = {0};
static
UppKolListe j_sutun = {0};
static
long long j_n = 0LL;
static
char* g_kaynak = NULL;
static
char* g_kaynak_ad = NULL;
static
long long g_cp_i = 0LL;
static
long long g_lex_satir = 1LL;
static
long long g_lex_sutun = 1LL;
static
char* g_tampon = NULL;
static
char* g_bg = NULL;
static
long long g_indent = 0LL;
static
long long g_emit_hedef = 0LL;
static
long long g_ayristir_i = 0LL;
static
long long g_loop = 0LL;
static
long long g_switch = 0LL;
static
long long g_bg_mod = 0LL;
static
long long g_tmp = 0LL;
static
long long g_bg_id = 0LL;
static
long long g_cur_fn = 0LL;
static
char* g_cur_class = NULL;
static
UppKolListe k_ad = {0};
static
UppKolListe k_tip = {0};
static
UppKolListe k_seviye = {0};
static
long long k_n = 0LL;
static
long long k_seviye_cur = 0LL;
static
UppKolHarita t_sinif = {0};
static
UppKolHarita t_fn = {0};
static
UppKolHarita t_enum = {0};
static
UppKolHarita t_global = {0};
static
UppKolListe t_sinif_adler = {0};
static
char* g_ozel_runtime_yol = NULL;
static
long long g_hata_bayrak = 0LL;
static
long long g_hata_guvenlik = 0LL;
static
long long g_json_mod = 0LL;
static
long long g_analiz_topla = 0LL;
static
long long g_hata_satir = 0LL;
static
long long g_hata_sutun = 1LL;
static
char* g_hata_mesaj = NULL;
static
char* g_htampon = NULL;
static
UppKolListe a_satir = {0};
static
UppKolListe a_sutun = {0};
static
UppKolListe a_gv = {0};
static
UppKolListe a_mesaj = {0};
static
long long a_n = 0LL;
static void _upp_kuresel_hazir(void) {
    g_gcc_log = upp_metin_kopya("");
    g_kaynak = upp_metin_kopya("");
    g_kaynak_ad = upp_metin_kopya("<girdi>");
    g_tampon = upp_metin_kopya("");
    g_bg = upp_metin_kopya("");
    g_cur_class = upp_metin_kopya("");
    g_ozel_runtime_yol = upp_metin_kopya("");
    g_hata_mesaj = upp_metin_kopya("");
    g_htampon = upp_metin_kopya("");
}

void intern_hazir(void) {
    {
        if ((intern_n > 0LL))
        {
            return;
        }
        upp_liste_ekle(&(intern_metin), 3, 0LL, 0.0, "");
        upp_harita_koy(&(intern_map), 3, 0, 0LL, "", (long long)(0LL), 0.0, NULL);
        (intern_n = 1LL);
    }
}

char* metin_kopya_yeni(char* s) {
    {
        char* o = upp_metin_kopya("");
        if ((s == NULL))
        {
            return o;
        }
        upp_metin_ekle(&(o), s);
        return o;
    }
}

void intern_sifirla(void) {
    {
        upp_harita_bosalt(&(intern_map));
        upp_liste_bosalt(&(intern_metin));
        (intern_n = 0LL);
    }
}

char* intern(char* s) {
    {
        intern_hazir();
        if ((s == NULL))
        {
            (s = "");
        }
        if ((bool)upp_harita_var_mi(intern_map, 3, 0LL, s))
        {
            return upp_liste_al_s(intern_metin, upp_harita_al_i(intern_map, 3, 0LL, s));
        }
        char* kopya = metin_kopya_yeni(s);
        upp_harita_koy(&(intern_map), 3, 0, 0LL, kopya, (long long)(intern_n), 0.0, NULL);
        upp_liste_ekle(&(intern_metin), 3, 0LL, 0.0, kopya);
        (intern_n = (intern_n + 1LL));
        return kopya;
    }
}

char* derleme_kaynak_ad(void) {
    {
        return g_kaynak_ad;
    }
}

bool metin_esit(char* a, char* b) {
    {
        if ((a == NULL))
        {
            (a = "");
        }
        if ((b == NULL))
        {
            (b = "");
        }
        return upp_metin_esit(a, b);
    }
}

long long metin_uzun(char* s) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        if ((s == NULL))
        {
            return 0LL;
        }
        return upp_uzunluk(s);
    }
}

long long metin_cp(char* s, long long i) {
    {
        if ((s == NULL))
        {
            return (-1LL);
        }
        if (((i < 0LL) || (i >= upp_uzunluk(s))))
        {
            return (-1LL);
        }
        return upp_metin_kod(s, i);
    }
}

long long metin_adv(char* s, long long i) {
    {
        if ((s == NULL))
        {
            return 1LL;
        }
        if (((i < 0LL) || (i >= upp_uzunluk(s))))
        {
            return 1LL;
        }
        return 1LL;
    }
}

char* intern_dilim(char* s, long long bas, long long son) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        long long n = upp_uzunluk(s);
        if ((bas < 0LL))
        {
            (bas = 0LL);
        }
        if ((son > n))
        {
            (son = n);
        }
        if ((son < bas))
        {
            (son = bas);
        }
        return intern(upp_metin_kes(s, bas, (son - bas)));
    }
}

char* kaynak_lf(char* s) {
    {
        if ((s == NULL))
        {
            return NULL;
        }
        long long n = upp_uzunluk(s);
        long long i = 0LL;
        while ((i < n))
        {
            if ((upp_metin_kod(s, i) == 13LL))
            {
                break;
            }
            (i = (i + 1LL));
        }
        if ((i >= n))
        {
            return intern(s);
        }
        char* o = upp_metin_kopya("");
        long long parca = 0LL;
        (i = 0LL);
        while ((i < n))
        {
            if ((upp_metin_kod(s, i) == 13LL))
            {
                if ((i > parca))
                {
                    upp_metin_ekle(&(o), upp_metin_kes(s, parca, (i - parca)));
                }
                (parca = (i + 1LL));
            }
            (i = (i + 1LL));
        }
        if ((parca < n))
        {
            upp_metin_ekle(&(o), upp_metin_kes(s, parca, (n - parca)));
        }
        return intern(o);
    }
}

char* sayi_metin(long long n) {
    {
        if ((n == 0LL))
        {
            return intern("0");
        }
        char* o = upp_metin_kopya("");
        if ((n < 0LL))
        {
            upp_metin_ekle(&(o), "-");
            (n = (0LL - n));
        }
        char* haneler = upp_metin_kopya("0123456789");
        char* ters = upp_metin_kopya("");
        while ((n > 0LL))
        {
            long long h = (n - ((n / 10LL) * 10LL));
            upp_metin_ekle(&(ters), upp_metin_kes(haneler, h, 1LL));
            (n = (n / 10LL));
        }
        long long i = (upp_uzunluk(ters) - 1LL);
        while ((i >= 0LL))
        {
            upp_metin_ekle(&(o), upp_metin_kes(ters, i, 1LL));
            (i = (i - 1LL));
        }
        return intern(o);
    }
}

char* kat(char* a, char* b) {
    {
        if ((a == NULL))
        {
            (a = "");
        }
        if ((b == NULL))
        {
            (b = "");
        }
        char* s = upp_metin_kopya("");
        upp_metin_ekle(&(s), a);
        upp_metin_ekle(&(s), b);
        return intern(s);
    }
}

char* kat3(char* a, char* b, char* c) {
    {
        return kat(kat(a, b), c);
    }
}

char* kat4(char* a, char* b, char* c, char* d) {
    {
        return kat(kat(a, b), kat(c, d));
    }
}

char* kat5(char* a, char* b, char* c, char* d, char* e) {
    {
        return kat(kat4(a, b, c, d), e);
    }
}

char* c_escape(char* s) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        char* o = upp_metin_kopya("\"");
        long long i = 0LL;
        long long n = upp_uzunluk(s);
        while ((i < n))
        {
            long long c = upp_metin_kod(s, i);
            if ((c == 92LL))
            {
                upp_metin_ekle(&(o), "\\\\");
            }
            else
                if ((c == 34LL))
                {
                    upp_metin_ekle(&(o), "\\\"");
                }
                else
                    if ((c == 10LL))
                    {
                        upp_metin_ekle(&(o), "\\n");
                    }
                    else
                        if ((c == 13LL))
                        {
                            upp_metin_ekle(&(o), "\\r");
                        }
                        else
                            if ((c == 9LL))
                            {
                                upp_metin_ekle(&(o), "\\t");
                            }
                            else
                            {
                                upp_metin_ekle(&(o), upp_metin_kes(s, i, 1LL));
                            }
            (i = (i + 1LL));
        }
        upp_metin_ekle(&(o), "\"");
        return intern(o);
    }
}

long long hex_hane_deger(long long c) {
    {
        if (((c >= 48LL) && (c <= 57LL)))
        {
            return (c - 48LL);
        }
        if (((c >= 97LL) && (c <= 102LL)))
        {
            return (c - 87LL);
        }
        if (((c >= 65LL) && (c <= 70LL)))
        {
            return (c - 55LL);
        }
        return (-1LL);
    }
}

long long hex_deger(char* lex) {
    {
        if ((lex == NULL))
        {
            return 0LL;
        }
        long long i = 0LL;
        long long n = upp_uzunluk(lex);
        long long isaret = 1LL;
        if (((i < n) && (upp_metin_kod(lex, i) == 45LL)))
        {
            (isaret = (0LL - 1LL));
            (i = (i + 1LL));
        }
        if (((((i + 1LL) < n) && (upp_metin_kod(lex, i) == 48LL)) && ((upp_metin_kod(lex, (i + 1LL)) == 120LL) || (upp_metin_kod(lex, (i + 1LL)) == 88LL))))
        {
            (i = (i + 2LL));
        }
        long long v = 0LL;
        while ((i < n))
        {
            long long h = hex_hane_deger(upp_metin_kod(lex, i));
            if ((h < 0LL))
            {
                break;
            }
            (v = ((v * 16LL) + h));
            (i = (i + 1LL));
        }
        return (v * isaret);
    }
}

long long sayi_oku(char* lex) {
    {
        if ((lex == NULL))
        {
            return 0LL;
        }
        long long i = 0LL;
        long long n = upp_uzunluk(lex);
        long long isaret = 1LL;
        if (((i < n) && (upp_metin_kod(lex, i) == 45LL)))
        {
            (isaret = (0LL - 1LL));
            (i = (i + 1LL));
        }
        long long v = 0LL;
        while ((i < n))
        {
            long long c = upp_metin_kod(lex, i);
            if (((c < 48LL) || (c > 57LL)))
            {
                break;
            }
            (v = ((v * 10LL) + (c - 48LL)));
            (i = (i + 1LL));
        }
        return (v * isaret);
    }
}

char* satir_kirp_sol(char* s) {
    {
        if ((s == NULL))
        {
            return "";
        }
        long long i = 0LL;
        long long n = upp_uzunluk(s);
        while (((i < n) && (((upp_metin_kod(s, i) == 32LL) || (upp_metin_kod(s, i) == 9LL)) || (upp_metin_kod(s, i) == 13LL))))
        {
            (i = (i + 1LL));
        }
        return intern(upp_metin_kes(s, i, (n - i)));
    }
}

bool c_on_islemci(char* kod) {
    {
        if ((kod == NULL))
        {
            return false;
        }
        long long i = 0LL;
        long long n = upp_uzunluk(kod);
        while ((i < n))
        {
            long long bas = i;
            while (((i < n) && (upp_metin_kod(kod, i) != 10LL)))
            {
                (i = (i + 1LL));
            }
            char* satir = satir_kirp_sol(upp_metin_kes(kod, bas, (i - bas)));
            if ((((upp_metin_baslar(satir, "#include") || upp_metin_baslar(satir, "#define")) || upp_metin_baslar(satir, "#pragma")) || upp_metin_baslar(satir, "#if")))
            {
                return true;
            }
            if (((i < n) && (upp_metin_kod(kod, i) == 10LL)))
            {
                (i = (i + 1LL));
            }
        }
        return false;
    }
}

bool ident_bas(long long cp) {
    {
        if (((cp >= 65LL) && (cp <= 90LL)))
        {
            return true;
        }
        if (((cp >= 97LL) && (cp <= 122LL)))
        {
            return true;
        }
        if ((cp == 95LL))
        {
            return true;
        }
        if (((((((cp == 231LL) || (cp == 287LL)) || (cp == 305LL)) || (cp == 246LL)) || (cp == 351LL)) || (cp == 252LL)))
        {
            return true;
        }
        if (((((((cp == 199LL) || (cp == 286LL)) || (cp == 304LL)) || (cp == 214LL)) || (cp == 350LL)) || (cp == 220LL)))
        {
            return true;
        }
        return false;
    }
}

bool ident_devam(long long cp) {
    {
        if (((cp >= 48LL) && (cp <= 57LL)))
        {
            return true;
        }
        return ident_bas(cp);
    }
}

void dugum_hazir(void) {
    {
        if ((d_n > 0LL))
        {
            return;
        }
        upp_liste_ekle(&(d_tur), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_satir), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_sutun), 0, (long long)(1LL), 0.0, NULL);
        upp_liste_ekle(&(d_ad), 3, 0LL, 0.0, "");
        upp_liste_ekle(&(d_kaynak), 3, 0LL, 0.0, "");
        upp_liste_ekle(&(d_sayi), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_a), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_b), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_c), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_d), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_liste), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_liste2), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_ptrd), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_dims), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_bayrak), 0, (long long)(0LL), 0.0, NULL);
        (d_n = 1LL);
    }
}

bool dugum_ok(long long id) {
    {
        if (((id <= 0LL) || (id >= d_n)))
        {
            hata_ver(1LL, 1LL, "geçersiz düğüm id");
            return false;
        }
        return true;
    }
}

long long dugum_yeni(long long tur, long long satir, long long sutun) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        dugum_hazir();
        upp_liste_ekle(&(d_tur), 0, (long long)(tur), 0.0, NULL);
        upp_liste_ekle(&(d_satir), 0, (long long)(satir), 0.0, NULL);
        upp_liste_ekle(&(d_sutun), 0, (long long)(sutun), 0.0, NULL);
        upp_liste_ekle(&(d_ad), 3, 0LL, 0.0, "");
        upp_liste_ekle(&(d_kaynak), 3, 0LL, 0.0, g_kaynak_ad);
        upp_liste_ekle(&(d_sayi), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_a), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_b), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_c), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_d), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_liste), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_liste2), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_ptrd), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_dims), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(d_bayrak), 0, (long long)(0LL), 0.0, NULL);
        long long id = d_n;
        (d_n = (d_n + 1LL));
        return id;
    }
}

long long dugum_tur(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_tur, id);
    }
}

long long dugum_satir(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_satir, id);
    }
}

long long dugum_sutun(long long id) {
    {
        if ((id == 0LL))
        {
            return 1LL;
        }
        if ((!dugum_ok(id)))
        {
            return 1LL;
        }
        return upp_liste_al_i(d_sutun, id);
    }
}

char* dugum_ad(long long id) {
    {
        if ((id == 0LL))
        {
            return "";
        }
        if ((!dugum_ok(id)))
        {
            return "";
        }
        return upp_liste_al_s(d_ad, id);
    }
}

char* dugum_kaynak(long long id) {
    {
        if ((id == 0LL))
        {
            return "";
        }
        if ((!dugum_ok(id)))
        {
            return "";
        }
        return upp_liste_al_s(d_kaynak, id);
    }
}

long long dugum_sayi(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_sayi, id);
    }
}

long long dugum_a(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_a, id);
    }
}

long long dugum_b(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_b, id);
    }
}

long long dugum_c(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_c, id);
    }
}

long long dugum_d(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_d, id);
    }
}

long long dugum_liste(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_liste, id);
    }
}

long long dugum_liste2(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_liste2, id);
    }
}

long long dugum_ptrd(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_ptrd, id);
    }
}

long long dugum_dims(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_dims, id);
    }
}

long long dugum_bayrak(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!dugum_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(d_bayrak, id);
    }
}

void dugum_yaz_ad(long long id, char* ad) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_ad), 3, id, 0LL, 0.0, intern(ad));
    }
}

void dugum_yaz_sayi(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_sayi), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_a(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_a), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_b(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_b), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_c(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_c), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_d(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_d), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_liste(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_liste), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_liste2(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_liste2), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_ptrd(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_ptrd), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_dims(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_dims), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_bayrak(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_bayrak), 0, id, (long long)(v), 0.0, NULL);
    }
}

void dugum_yaz_tur(long long id, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!dugum_ok(id)))
        {
            return;
        }
        upp_liste_yaz(&(d_tur), 0, id, (long long)(v), 0.0, NULL);
    }
}

void liste_hazir(void) {
    {
        if ((l_n > 0LL))
        {
            return;
        }
        upp_liste_ekle(&(l_ofset), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(l_uzunluk), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(l_kap), 0, (long long)(0LL), 0.0, NULL);
        (l_n = 1LL);
    }
}

bool liste_ok(long long id) {
    {
        if (((id <= 0LL) || (id >= l_n)))
        {
            hata_ver(1LL, 1LL, "geçersiz liste id");
            return false;
        }
        return true;
    }
}

void liste_buyut(long long id) {
    {
        long long eski_of = upp_liste_al_i(l_ofset, id);
        long long eski_n = upp_liste_al_i(l_uzunluk, id);
        long long eski_kap = upp_liste_al_i(l_kap, id);
        long long yeni_kap = eski_kap;
        if ((yeni_kap <= 0LL))
        {
            (yeni_kap = 4LL);
        }
        else
        {
            if ((yeni_kap > 4611686018427387903LL))
            {
                hata_ver(1LL, 1LL, "liste kapasitesi taştı");
                return;
            }
            (yeni_kap = (yeni_kap * 2LL));
        }
        if (((eski_kap > 0LL) && ((eski_of + eski_kap) == upp_liste_uzunluk(l_veri))))
        {
            long long i = eski_kap;
            while ((i < yeni_kap))
            {
                upp_liste_ekle(&(l_veri), 0, (long long)(0LL), 0.0, NULL);
                (i = (i + 1LL));
            }
            upp_liste_yaz(&(l_kap), 0, id, (long long)(yeni_kap), 0.0, NULL);
            return;
        }
        long long bi = 0LL;
        while ((bi < upp_liste_uzunluk(l_bos_of)))
        {
            if ((upp_liste_al_i(l_bos_kap, bi) >= yeni_kap))
            {
                long long yeni_of = upp_liste_al_i(l_bos_of, bi);
                long long i = 0LL;
                while ((i < eski_n))
                {
                    upp_liste_yaz(&(l_veri), 0, (yeni_of + i), (long long)(upp_liste_al_i(l_veri, (eski_of + i))), 0.0, NULL);
                    (i = (i + 1LL));
                }
                upp_liste_yaz(&(l_ofset), 0, id, (long long)(yeni_of), 0.0, NULL);
                upp_liste_yaz(&(l_kap), 0, id, (long long)(upp_liste_al_i(l_bos_kap, bi)), 0.0, NULL);
                upp_liste_sil(&(l_bos_of), bi);
                upp_liste_sil(&(l_bos_kap), bi);
                if ((eski_kap > 0LL))
                {
                    upp_liste_ekle(&(l_bos_of), 0, (long long)(eski_of), 0.0, NULL);
                    upp_liste_ekle(&(l_bos_kap), 0, (long long)(eski_kap), 0.0, NULL);
                }
                return;
            }
            (bi = (bi + 1LL));
        }
        long long yeni_of = upp_liste_uzunluk(l_veri);
        long long i = 0LL;
        while ((i < eski_n))
        {
            upp_liste_ekle(&(l_veri), 0, (long long)(upp_liste_al_i(l_veri, (eski_of + i))), 0.0, NULL);
            (i = (i + 1LL));
        }
        while ((i < yeni_kap))
        {
            upp_liste_ekle(&(l_veri), 0, (long long)(0LL), 0.0, NULL);
            (i = (i + 1LL));
        }
        upp_liste_yaz(&(l_ofset), 0, id, (long long)(yeni_of), 0.0, NULL);
        upp_liste_yaz(&(l_kap), 0, id, (long long)(yeni_kap), 0.0, NULL);
        if ((eski_kap > 0LL))
        {
            upp_liste_ekle(&(l_bos_of), 0, (long long)(eski_of), 0.0, NULL);
            upp_liste_ekle(&(l_bos_kap), 0, (long long)(eski_kap), 0.0, NULL);
        }
    }
}

long long liste_yeni(void) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        liste_hazir();
        upp_liste_ekle(&(l_ofset), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(l_uzunluk), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(l_kap), 0, (long long)(0LL), 0.0, NULL);
        long long id = l_n;
        (l_n = (l_n + 1LL));
        return id;
    }
}

void liste_ekle(long long id, long long oge) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!liste_ok(id)))
        {
            return;
        }
        if ((upp_liste_al_i(l_uzunluk, id) >= upp_liste_al_i(l_kap, id)))
        {
            liste_buyut(id);
        }
        upp_liste_yaz(&(l_veri), 0, (upp_liste_al_i(l_ofset, id) + upp_liste_al_i(l_uzunluk, id)), (long long)(oge), 0.0, NULL);
        upp_liste_yaz(&(l_uzunluk), 0, id, (long long)((upp_liste_al_i(l_uzunluk, id) + 1LL)), 0.0, NULL);
    }
}

long long liste_uzunluk(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!liste_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(l_uzunluk, id);
    }
}

long long liste_al(long long id, long long i) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!liste_ok(id)))
        {
            return 0LL;
        }
        if (((i < 0LL) || (i >= upp_liste_al_i(l_uzunluk, id))))
        {
            hata_ver(1LL, 1LL, "liste indeksi sınır dışında");
            return 0LL;
        }
        return upp_liste_al_i(l_veri, (upp_liste_al_i(l_ofset, id) + i));
    }
}

void liste_yaz(long long id, long long i, long long v) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!liste_ok(id)))
        {
            return;
        }
        if (((i < 0LL) || (i >= upp_liste_al_i(l_uzunluk, id))))
        {
            hata_ver(1LL, 1LL, "liste indeksi sınır dışında");
            return;
        }
        upp_liste_yaz(&(l_veri), 0, (upp_liste_al_i(l_ofset, id) + i), (long long)(v), 0.0, NULL);
    }
}

void liste_kisalt(long long id) {
    {
        if ((id == 0LL))
        {
            return;
        }
        if ((!liste_ok(id)))
        {
            return;
        }
        if ((upp_liste_al_i(l_uzunluk, id) > 0LL))
        {
            upp_liste_yaz(&(l_uzunluk), 0, id, (long long)((upp_liste_al_i(l_uzunluk, id) - 1LL)), 0.0, NULL);
        }
    }
}

void jeton_hazir(void) {
    {
        if ((j_n > 0LL))
        {
            return;
        }
        upp_liste_ekle(&(j_tur), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(j_deger), 3, 0LL, 0.0, "");
        upp_liste_ekle(&(j_satir), 0, (long long)(0LL), 0.0, NULL);
        upp_liste_ekle(&(j_sutun), 0, (long long)(1LL), 0.0, NULL);
        (j_n = 1LL);
    }
}

bool jeton_ok(long long id) {
    {
        if (((id <= 0LL) || (id >= j_n)))
        {
            hata_ver(1LL, 1LL, "geçersiz jeton id");
            return false;
        }
        return true;
    }
}

long long jeton_yeni(long long tur, char* deger, long long satir, long long sutun) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        jeton_hazir();
        if ((j_n == upp_liste_uzunluk(j_tur)))
        {
            upp_liste_ekle(&(j_tur), 0, (long long)(tur), 0.0, NULL);
            upp_liste_ekle(&(j_deger), 3, 0LL, 0.0, intern(deger));
            upp_liste_ekle(&(j_satir), 0, (long long)(satir), 0.0, NULL);
            upp_liste_ekle(&(j_sutun), 0, (long long)(sutun), 0.0, NULL);
        }
        else
        {
            upp_liste_yaz(&(j_tur), 0, j_n, (long long)(tur), 0.0, NULL);
            upp_liste_yaz(&(j_deger), 3, j_n, 0LL, 0.0, intern(deger));
            upp_liste_yaz(&(j_satir), 0, j_n, (long long)(satir), 0.0, NULL);
            upp_liste_yaz(&(j_sutun), 0, j_n, (long long)(sutun), 0.0, NULL);
        }
        long long id = j_n;
        (j_n = (j_n + 1LL));
        return id;
    }
}

long long jeton_tur(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!jeton_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(j_tur, id);
    }
}

char* jeton_deger(long long id) {
    {
        if ((id == 0LL))
        {
            return "";
        }
        if ((!jeton_ok(id)))
        {
            return "";
        }
        return upp_liste_al_s(j_deger, id);
    }
}

long long jeton_satir(long long id) {
    {
        if ((id == 0LL))
        {
            return 0LL;
        }
        if ((!jeton_ok(id)))
        {
            return 0LL;
        }
        return upp_liste_al_i(j_satir, id);
    }
}

long long jeton_sutun(long long id) {
    {
        if ((id == 0LL))
        {
            return 1LL;
        }
        if ((!jeton_ok(id)))
        {
            return 1LL;
        }
        return upp_liste_al_i(j_sutun, id);
    }
}

long long jeton_sayisi(void) {
    {
        return j_n;
    }
}

void jeton_sifirla(void) {
    {
        if ((upp_liste_uzunluk(j_tur) == 0LL))
        {
            (j_n = 0LL);
            return;
        }
        (j_n = 1LL);
    }
}

void kaynak_ayarla(char* s, char* ad) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        if (((upp_uzunluk(s) > 0LL) && (upp_metin_kod(s, 0LL) == 65279LL)))
        {
            (s = intern(upp_metin_kes(s, 1LL, (upp_uzunluk(s) - 1LL))));
        }
        (g_kaynak = s);
        (g_cp_i = 0LL);
        (g_lex_satir = 1LL);
        (g_lex_sutun = 1LL);
        if (((ad == NULL) || metin_esit(ad, "")))
        {
            (g_kaynak_ad = intern("<girdi>"));
        }
        else
        {
            (g_kaynak_ad = intern(ad));
        }
    }
}

void lex_kaynak_ad_guncelle(char* ad) {
    {
        if (((ad == NULL) || metin_esit(ad, "")))
        {
            return;
        }
        (g_kaynak_ad = intern(ad));
        (g_lex_satir = 0LL);
        (g_lex_sutun = 1LL);
    }
}

void lex_yorum_govde(char* govde) {
    {
        if ((govde == NULL))
        {
            return;
        }
        long long i = 0LL;
        long long n = upp_uzunluk(govde);
        while (((i < n) && ((upp_metin_kod(govde, i) == 32LL) || (upp_metin_kod(govde, i) == 9LL))))
        {
            (i = (i + 1LL));
        }
        char* isaret = upp_metin_kopya("@upp-kaynak");
        long long m = upp_uzunluk(isaret);
        if (((i + m) > n))
        {
            return;
        }
        long long j = 0LL;
        while ((j < m))
        {
            if ((upp_metin_kod(govde, (i + j)) != upp_metin_kod(isaret, j)))
            {
                return;
            }
            (j = (j + 1LL));
        }
        (i = (i + m));
        if ((((i < n) && (upp_metin_kod(govde, i) != 32LL)) && (upp_metin_kod(govde, i) != 9LL)))
        {
            return;
        }
        while (((i < n) && ((upp_metin_kod(govde, i) == 32LL) || (upp_metin_kod(govde, i) == 9LL))))
        {
            (i = (i + 1LL));
        }
        long long son = n;
        while ((son > i))
        {
            long long c = upp_metin_kod(govde, (son - 1LL));
            if ((((c != 32LL) && (c != 9LL)) && (c != 13LL)))
            {
                break;
            }
            (son = (son - 1LL));
        }
        if ((son <= i))
        {
            return;
        }
        lex_kaynak_ad_guncelle(intern(upp_metin_kes(govde, i, (son - i))));
    }
}

char* lex_kaynak_metin(void) {
    {
        return g_kaynak;
    }
}

long long lex_cp(void) {
    {
        if ((g_cp_i >= upp_uzunluk(g_kaynak)))
        {
            return (-1LL);
        }
        return upp_metin_kod(g_kaynak, g_cp_i);
    }
}

bool lex_bitis(void) {
    {
        return (g_cp_i >= upp_uzunluk(g_kaynak));
    }
}

long long lex_pos(void) {
    {
        return g_cp_i;
    }
}

long long lex_satir(void) {
    {
        return g_lex_satir;
    }
}

long long lex_sutun(void) {
    {
        return g_lex_sutun;
    }
}

long long lex_ilerle(void) {
    {
        if ((g_cp_i >= upp_uzunluk(g_kaynak)))
        {
            return (-1LL);
        }
        long long cp = upp_metin_kod(g_kaynak, g_cp_i);
        (g_cp_i = (g_cp_i + 1LL));
        if ((cp == 10LL))
        {
            (g_lex_satir = (g_lex_satir + 1LL));
            (g_lex_sutun = 1LL);
        }
        else
        {
            (g_lex_sutun = (g_lex_sutun + 1LL));
        }
        return cp;
    }
}

char* lex_dilim(long long bas, long long son) {
    {
        return intern_dilim(g_kaynak, bas, son);
    }
}

long long lex_cp_ileri(long long kac) {
    {
        long long p = (g_cp_i + kac);
        if (((p < 0LL) || (p >= upp_uzunluk(g_kaynak))))
        {
            return (-1LL);
        }
        return upp_metin_kod(g_kaynak, p);
    }
}

void tampon_sifirla(void) {
    {
        (g_tampon = metin_kopya_yeni(""));
        (g_indent = 0LL);
    }
}

void tampon_yaz(char* s) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        upp_metin_ekle(&(g_tampon), s);
    }
}

void tampon_yaz_sayi(long long n) {
    {
        upp_metin_ekle(&(g_tampon), sayi_metin(n));
    }
}

char* tampon_al(void) {
    {
        return g_tampon;
    }
}

void bg_yigin_sifirla(void) {
    {
        (g_bg = metin_kopya_yeni(""));
    }
}

void bg_yigin_ekle(char* s) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        upp_metin_ekle(&(g_bg), s);
    }
}

char* bg_yigin_al(void) {
    {
        return g_bg;
    }
}

void emit(char* s) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        if ((g_emit_hedef == 1LL))
        {
            if (metin_esit(s, ""))
            {
                upp_metin_ekle(&(g_bg), "\n");
                return;
            }
            long long i = 0LL;
            while ((i < g_indent))
            {
                upp_metin_ekle(&(g_bg), "    ");
                (i = (i + 1LL));
            }
            upp_metin_ekle(&(g_bg), s);
            upp_metin_ekle(&(g_bg), "\n");
            return;
        }
        if (metin_esit(s, ""))
        {
            upp_metin_ekle(&(g_tampon), "\n");
            return;
        }
        long long k = 0LL;
        while ((k < g_indent))
        {
            upp_metin_ekle(&(g_tampon), "    ");
            (k = (k + 1LL));
        }
        upp_metin_ekle(&(g_tampon), s);
        upp_metin_ekle(&(g_tampon), "\n");
    }
}

void emit_hedef_yaz(long long v) {
    {
        if ((v != 0LL))
        {
            (g_emit_hedef = 1LL);
        }
        else
        {
            (g_emit_hedef = 0LL);
        }
    }
}

void os_hedef_yaz(long long v) {
    {
        if ((v != 0LL))
        {
            (g_os_hedef = 1LL);
        }
        else
        {
            (g_os_hedef = 0LL);
        }
    }
}

long long os_host_linux(void) {
    {
        char* p = upp_yol_birlestir("a", "b");
        long long i = 0LL;
        long long n = upp_uzunluk(p);
        while ((i < n))
        {
            long long c = upp_metin_kod(p, i);
            if ((c == 47LL))
            {
                return 1LL;
            }
            if ((c == 92LL))
            {
                return 0LL;
            }
            (i = (i + 1LL));
        }
        return 0LL;
    }
}

long long os_hedef_al(void) {
    {
        if ((g_os_hedef < 0LL))
        {
            (g_os_hedef = os_host_linux());
        }
        return g_os_hedef;
    }
}

char* os_tirnak(char* s) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        char* o = upp_metin_kopya("\"");
        upp_metin_ekle(&(o), s);
        upp_metin_ekle(&(o), "\"");
        return o;
    }
}

void indent_ekle(long long d) {
    {
        (g_indent = (g_indent + d));
        if ((g_indent < 0LL))
        {
            (g_indent = 0LL);
        }
    }
}

long long indent_al(void) {
    {
        return g_indent;
    }
}

void indent_kur(long long v) {
    {
        if ((v < 0LL))
        {
            (g_indent = 0LL);
        }
        else
        {
            (g_indent = v);
        }
    }
}

long long ayristir_i_al(void) {
    {
        return g_ayristir_i;
    }
}

void ayristir_i_yaz(long long v) {
    {
        (g_ayristir_i = v);
    }
}

long long loop_al(void) {
    {
        return g_loop;
    }
}

void loop_yaz(long long v) {
    {
        (g_loop = v);
    }
}

long long switch_al(void) {
    {
        return g_switch;
    }
}

void switch_yaz(long long v) {
    {
        (g_switch = v);
    }
}

long long bg_al(void) {
    {
        return g_bg_mod;
    }
}

void bg_yaz(long long v) {
    {
        (g_bg_mod = v);
    }
}

long long tmp_yeni(void) {
    {
        (g_tmp = (g_tmp + 1LL));
        return g_tmp;
    }
}

void tmp_sifirla(void) {
    {
        (g_tmp = 0LL);
    }
}

long long bgid_yeni(void) {
    {
        (g_bg_id = (g_bg_id + 1LL));
        return g_bg_id;
    }
}

void cur_fn_yaz(long long v) {
    {
        (g_cur_fn = v);
    }
}

long long cur_fn_al(void) {
    {
        return g_cur_fn;
    }
}

void cur_class_yaz(char* s) {
    {
        (g_cur_class = intern(s));
    }
}

char* cur_class_al(void) {
    {
        return g_cur_class;
    }
}

void kapsam_sifirla(void) {
    {
        (k_n = 0LL);
        (k_seviye_cur = 0LL);
    }
}

void kapsam_ac(void) {
    {
        (k_seviye_cur = (k_seviye_cur + 1LL));
    }
}

void kapsam_kapat(void) {
    {
        while (((k_n > 0LL) && (upp_liste_al_i(k_seviye, (k_n - 1LL)) == k_seviye_cur)))
        {
            (k_n = (k_n - 1LL));
        }
        if ((k_seviye_cur > 0LL))
        {
            (k_seviye_cur = (k_seviye_cur - 1LL));
        }
    }
}

void kapsam_bagla(char* ad, long long ty) {
    {
        if ((k_seviye_cur <= 0LL))
        {
            kapsam_ac();
        }
        if ((k_n == upp_liste_uzunluk(k_ad)))
        {
            upp_liste_ekle(&(k_ad), 3, 0LL, 0.0, intern(ad));
            upp_liste_ekle(&(k_tip), 0, (long long)(ty), 0.0, NULL);
            upp_liste_ekle(&(k_seviye), 0, (long long)(k_seviye_cur), 0.0, NULL);
        }
        else
        {
            upp_liste_yaz(&(k_ad), 3, k_n, 0LL, 0.0, intern(ad));
            upp_liste_yaz(&(k_tip), 0, k_n, (long long)(ty), 0.0, NULL);
            upp_liste_yaz(&(k_seviye), 0, k_n, (long long)(k_seviye_cur), 0.0, NULL);
        }
        (k_n = (k_n + 1LL));
    }
}

long long kapsam_bul(char* ad) {
    {
        if ((ad == NULL))
        {
            (ad = "");
        }
        long long i = (k_n - 1LL);
        while ((i >= 0LL))
        {
            if (metin_esit(upp_liste_al_s(k_ad, i), ad))
            {
                return upp_liste_al_i(k_tip, i);
            }
            (i = (i - 1LL));
        }
        return 0LL;
    }
}

bool kapsam_yerel_var(char* ad) {
    {
        if ((ad == NULL))
        {
            (ad = "");
        }
        if ((k_n <= 0LL))
        {
            return false;
        }
        long long sev = k_seviye_cur;
        long long i = (k_n - 1LL);
        while (((i >= 0LL) && (upp_liste_al_i(k_seviye, i) == sev)))
        {
            if (metin_esit(upp_liste_al_s(k_ad, i), ad))
            {
                return true;
            }
            (i = (i - 1LL));
        }
        return false;
    }
}

void tablo_sifirla(void) {
    {
        upp_harita_bosalt(&(t_sinif));
        upp_harita_bosalt(&(t_fn));
        upp_harita_bosalt(&(t_enum));
        upp_harita_bosalt(&(t_global));
        upp_liste_bosalt(&(t_sinif_adler));
    }
}

long long tablo_yaz(long long kind, char* ad, long long id) {
    {
        char* k = intern(ad);
        if ((kind == 0LL))
        {
            if ((bool)upp_harita_var_mi(t_sinif, 3, 0LL, k))
            {
                upp_harita_koy(&(t_sinif), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
                return 1LL;
            }
            upp_harita_koy(&(t_sinif), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
            upp_liste_ekle(&(t_sinif_adler), 3, 0LL, 0.0, k);
            return 0LL;
        }
        if ((kind == 1LL))
        {
            if ((bool)upp_harita_var_mi(t_fn, 3, 0LL, k))
            {
                upp_harita_koy(&(t_fn), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
                return 1LL;
            }
            upp_harita_koy(&(t_fn), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
            return 0LL;
        }
        if ((kind == 2LL))
        {
            if ((bool)upp_harita_var_mi(t_enum, 3, 0LL, k))
            {
                upp_harita_koy(&(t_enum), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
                return 1LL;
            }
            upp_harita_koy(&(t_enum), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
            return 0LL;
        }
        if ((bool)upp_harita_var_mi(t_global, 3, 0LL, k))
        {
            upp_harita_koy(&(t_global), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
            return 1LL;
        }
        upp_harita_koy(&(t_global), 3, 0, 0LL, k, (long long)(id), 0.0, NULL);
        return 0LL;
    }
}

long long tablo_bul(long long kind, char* ad) {
    {
        if ((ad == NULL))
        {
            (ad = "");
        }
        if ((kind == 0LL))
        {
            if ((bool)upp_harita_var_mi(t_sinif, 3, 0LL, ad))
            {
                return upp_harita_al_i(t_sinif, 3, 0LL, ad);
            }
            return 0LL;
        }
        if ((kind == 1LL))
        {
            if ((bool)upp_harita_var_mi(t_fn, 3, 0LL, ad))
            {
                return upp_harita_al_i(t_fn, 3, 0LL, ad);
            }
            return 0LL;
        }
        if ((kind == 2LL))
        {
            if ((bool)upp_harita_var_mi(t_enum, 3, 0LL, ad))
            {
                return upp_harita_al_i(t_enum, 3, 0LL, ad);
            }
            return 0LL;
        }
        if ((bool)upp_harita_var_mi(t_global, 3, 0LL, ad))
        {
            return upp_harita_al_i(t_global, 3, 0LL, ad);
        }
        return 0LL;
    }
}

long long tablo_sinif_sayisi(void) {
    {
        return upp_liste_uzunluk(t_sinif_adler);
    }
}

char* tablo_sinif_ad(long long i) {
    {
        if (((i < 0LL) || (i >= upp_liste_uzunluk(t_sinif_adler))))
        {
            hata_ver(1LL, 1LL, "sınıf tablosu indeksi sınır dışında");
            return "";
        }
        return upp_liste_al_s(t_sinif_adler, i);
    }
}

long long tablo_sinif_id(long long i) {
    {
        if (((i < 0LL) || (i >= upp_liste_uzunluk(t_sinif_adler))))
        {
            hata_ver(1LL, 1LL, "sınıf tablosu indeksi sınır dışında");
            return 0LL;
        }
        return upp_harita_al_i(t_sinif, 3, 0LL, upp_liste_al_s(t_sinif_adler, i));
    }
}

bool os_dosya_yaz(char* yol, char* data) {
    {
        if (((yol == NULL) || metin_esit(yol, "")))
        {
            return false;
        }
        if ((data == NULL))
        {
            (data = "");
        }
        upp_yol_klasor_olustur(upp_yol_dizin(yol));
        return (upp_dosya_yaz(yol, data) != 0LL);
    }
}

bool os_dosya_var(char* yol) {
    {
        if (((yol == NULL) || metin_esit(yol, "")))
        {
            return false;
        }
        return upp_dosya_var_mi(yol);
    }
}

char* os_yol_dizin(char* yol) {
    {
        return intern(upp_yol_dizin(yol));
    }
}

char* os_yol_birlestir(char* dir, char* rel) {
    {
        return intern(upp_yol_birlestir(dir, rel));
    }
}

char* os_yol_gercek(char* yol) {
    {
        return intern(upp_yol_gercek(yol));
    }
}

char* os_cwd(void) {
    {
        return intern(upp_yol_calisma());
    }
}

bool os_yol_mutlak_mi(char* yol) {
    {
        return upp_yol_mutlak_mi(yol);
    }
}

char* os_runtime_dene(char* kok) {
    {
        if (((kok == NULL) || metin_esit(kok, "")))
        {
            return "";
        }
        char* a = upp_yol_birlestir(kok, "src/runtime/upp_runtime.c");
        if (upp_dosya_var_mi(a))
        {
            return intern(a);
        }
        char* b = upp_yol_birlestir(kok, "runtime/upp_runtime.c");
        if (upp_dosya_var_mi(b))
        {
            return intern(b);
        }
        return "";
    }
}

char* os_runtime_bul(void) {
    {
        char* hit = os_runtime_dene(upp_yol_calisma());
        if ((!metin_esit(hit, "")))
        {
            return hit;
        }
        char* d = upp_sistem_exe_dizin();
        long long i = 0LL;
        while ((i < 6LL))
        {
            (hit = os_runtime_dene(d));
            if ((!metin_esit(hit, "")))
            {
                return hit;
            }
            char* ust = upp_yol_dizin(d);
            if ((metin_esit(ust, d) || metin_esit(ust, "")))
            {
                break;
            }
            (d = ust);
            (i = (i + 1LL));
        }
        return "";
    }
}

void os_runtime_ozel_yaz(char* yol) {
    {
        (g_ozel_runtime_yol = yol);
    }
}

char* os_runtime_oku(void) {
    {
        if ((!metin_esit(g_ozel_runtime_yol, "")))
        {
            if (upp_dosya_var_mi(g_ozel_runtime_yol))
            {
                char* s = upp_dosya_oku(g_ozel_runtime_yol);
                if ((s != NULL))
                {
                    return s;
                }
            }
        }
        char* p = os_runtime_bul();
        if (metin_esit(p, ""))
        {
            return intern("");
        }
        char* s = upp_dosya_oku(p);
        if ((s == NULL))
        {
            return intern("");
        }
        return s;
    }
}

char* os_gcc_aday_dene(char* p) {
    {
        if (upp_dosya_var_mi(p))
        {
            return intern(p);
        }
        return "";
    }
}

char* os_gcc_bul(void) {
    {
        char* p = upp_sistem_komut_bul("gcc");
        if (((p != NULL) && (!metin_esit(p, ""))))
        {
            return intern(p);
        }
        (p = os_gcc_aday_dene("C:/msys64/ucrt64/bin/gcc.exe"));
        if ((!metin_esit(p, "")))
        {
            return p;
        }
        (p = os_gcc_aday_dene("C:/msys64/mingw64/bin/gcc.exe"));
        if ((!metin_esit(p, "")))
        {
            return p;
        }
        (p = os_gcc_aday_dene("C:/msys64/clang64/bin/gcc.exe"));
        if ((!metin_esit(p, "")))
        {
            return p;
        }
        (p = os_gcc_aday_dene("C:/mingw64/bin/gcc.exe"));
        if ((!metin_esit(p, "")))
        {
            return p;
        }
        (p = os_gcc_aday_dene("C:/MinGW/bin/gcc.exe"));
        if ((!metin_esit(p, "")))
        {
            return p;
        }
        (p = os_gcc_aday_dene("/usr/bin/gcc"));
        if ((!metin_esit(p, "")))
        {
            return p;
        }
        (p = os_gcc_aday_dene("/usr/local/bin/gcc"));
        if ((!metin_esit(p, "")))
        {
            return p;
        }
        return intern("");
    }
}

bool os_yol_bosluk_var(char* s) {
    {
        if ((s == NULL))
        {
            return false;
        }
        long long i = 0LL;
        long long n = upp_uzunluk(s);
        while ((i < n))
        {
            if ((upp_metin_kod(s, i) == 32LL))
            {
                return true;
            }
            (i = (i + 1LL));
        }
        return false;
    }
}

char* os_cmd_parca(char* s) {
    {
        if (os_yol_bosluk_var(s))
        {
            return os_tirnak(s);
        }
        return s;
    }
}

long long os_gcc_cagir(char* gcc, char* c_yol, char* exe_yol) {
    {
        (g_gcc_log = metin_kopya_yeni(""));
        if (((((gcc == NULL) || metin_esit(gcc, "")) || (c_yol == NULL)) || (exe_yol == NULL)))
        {
            return 1LL;
        }
        char* cmd = metin_kopya_yeni(os_cmd_parca(gcc));
        upp_metin_ekle(&(cmd), " -std=gnu11 ");
        char* san = upp_ortam_al("UPP_SANITIZE");
        if (((san != NULL) && (!metin_esit(san, ""))))
        {
            upp_metin_ekle(&(cmd), san);
            upp_metin_ekle(&(cmd), " ");
        }
        if ((os_hedef_al() != 0LL))
        {
            upp_metin_ekle(&(cmd), "-pthread ");
            upp_metin_ekle(&(cmd), os_cmd_parca(c_yol));
            upp_metin_ekle(&(cmd), " -o ");
            upp_metin_ekle(&(cmd), os_cmd_parca(exe_yol));
            upp_metin_ekle(&(cmd), " -lm");
        }
        else
        {
            upp_metin_ekle(&(cmd), os_cmd_parca(c_yol));
            upp_metin_ekle(&(cmd), " -o ");
            upp_metin_ekle(&(cmd), os_cmd_parca(exe_yol));
            upp_metin_ekle(&(cmd), " -luser32 -lwinmm -lgdi32");
        }
        SurecCikti r = upp_sistem_calistir(cmd);
        if ((r.cikti != NULL))
        {
            (g_gcc_log = r.cikti);
        }
        return r.kod;
    }
}

char* os_gcc_cikti(void) {
    {
        if ((g_gcc_log == NULL))
        {
            return intern("");
        }
        return intern(g_gcc_log);
    }
}

long long os_program_calistir(char* yol) {
    {
        if (((yol == NULL) || metin_esit(yol, "")))
        {
            return 1LL;
        }
        SurecCikti r = upp_sistem_calistir(os_cmd_parca(yol));
        return r.kod;
    }
}

bool bicim_icinde(char* s, char* ara) {
    {
        if (((s == NULL) || (ara == NULL)))
        {
            return false;
        }
        long long n = upp_uzunluk(s);
        long long m = upp_uzunluk(ara);
        if (((m <= 0LL) || (m > n)))
        {
            return false;
        }
        long long i = 0LL;
        while (((i + m) <= n))
        {
            long long j = 0LL;
            bool ok = true;
            while ((j < m))
            {
                if ((upp_metin_kod(s, (i + j)) != upp_metin_kod(ara, j)))
                {
                    (ok = false);
                    break;
                }
                (j = (j + 1LL));
            }
            if (ok)
            {
                return true;
            }
            (i = (i + 1LL));
        }
        return false;
    }
}

char* bicim_kaynak(char* src) {
    {
        if ((src == NULL))
        {
            (src = "");
        }
        if (((upp_uzunluk(src) > 0LL) && (upp_metin_kod(src, 0LL) == 65279LL)))
        {
            (src = intern(upp_metin_kes(src, 1LL, (upp_uzunluk(src) - 1LL))));
        }
        char* out = upp_metin_kopya("");
        long long depth = 0LL;
        bool in_block = false;
        bool in_triple = false;
        long long p = 0LL;
        long long n = upp_uzunluk(src);
        while ((p < n))
        {
            long long bas = p;
            while ((((p < n) && (upp_metin_kod(src, p) != 10LL)) && (upp_metin_kod(src, p) != 13LL)))
            {
                (p = (p + 1LL));
            }
            char* line = intern(upp_metin_kes(src, bas, (p - bas)));
            if ((in_block || in_triple))
            {
                upp_metin_ekle(&(out), line);
                upp_metin_ekle(&(out), "\n");
                if ((in_block && bicim_icinde(line, "*/")))
                {
                    (in_block = false);
                }
                if (in_triple)
                {
                    long long q = 0LL;
                    long long ln = upp_uzunluk(line);
                    while (((q + 2LL) < ln))
                    {
                        if ((((upp_metin_kod(line, q) == 34LL) && (upp_metin_kod(line, (q + 1LL)) == 34LL)) && (upp_metin_kod(line, (q + 2LL)) == 34LL)))
                        {
                            if (in_triple)
                            {
                                (in_triple = false);
                            }
                            else
                            {
                                (in_triple = true);
                            }
                            (q = (q + 3LL));
                            continue;
                        }
                        (q = (q + 1LL));
                    }
                }
            }
            else
            {
                long long t = 0LL;
                long long llen = upp_uzunluk(line);
                while (((t < llen) && ((upp_metin_kod(line, t) == 32LL) || (upp_metin_kod(line, t) == 9LL))))
                {
                    (t = (t + 1LL));
                }
                if ((t >= llen))
                {
                    upp_metin_ekle(&(out), "\n");
                }
                else
                {
                    long long slen = llen;
                    while (((slen > t) && ((upp_metin_kod(line, (slen - 1LL)) == 32LL) || (upp_metin_kod(line, (slen - 1LL)) == 9LL))))
                    {
                        (slen = (slen - 1LL));
                    }
                    char* stripped = intern(upp_metin_kes(line, t, (slen - t)));
                    long long lead = depth;
                    long long tmpi = 0LL;
                    long long sn = upp_uzunluk(stripped);
                    while (((tmpi < sn) && (upp_metin_kod(stripped, tmpi) == 125LL)))
                    {
                        if ((lead > 0LL))
                        {
                            (lead = (lead - 1LL));
                        }
                        (tmpi = (tmpi + 1LL));
                        while (((tmpi < sn) && ((upp_metin_kod(stripped, tmpi) == 32LL) || (upp_metin_kod(stripped, tmpi) == 9LL))))
                        {
                            (tmpi = (tmpi + 1LL));
                        }
                    }
                    long long k = 0LL;
                    while ((k < lead))
                    {
                        upp_metin_ekle(&(out), "    ");
                        (k = (k + 1LL));
                    }
                    upp_metin_ekle(&(out), stripped);
                    upp_metin_ekle(&(out), "\n");
                    long long i = 0LL;
                    bool in_str = false;
                    bool esc = false;
                    while ((i < llen))
                    {
                        long long ch = upp_metin_kod(line, i);
                        long long nxt = (-1LL);
                        if (((i + 1LL) < llen))
                        {
                            (nxt = upp_metin_kod(line, (i + 1LL)));
                        }
                        if (in_str)
                        {
                            if (esc)
                            {
                                (esc = false);
                            }
                            else
                                if ((ch == 92LL))
                                {
                                    (esc = true);
                                }
                                else
                                    if ((ch == 34LL))
                                    {
                                        (in_str = false);
                                    }
                            (i = (i + 1LL));
                            continue;
                        }
                        if (((((ch == 34LL) && (nxt == 34LL)) && ((i + 2LL) < llen)) && (upp_metin_kod(line, (i + 2LL)) == 34LL)))
                        {
                            (in_triple = true);
                            (i = (i + 3LL));
                            long long r = i;
                            while (((r + 2LL) < llen))
                            {
                                if ((((upp_metin_kod(line, r) == 34LL) && (upp_metin_kod(line, (r + 1LL)) == 34LL)) && (upp_metin_kod(line, (r + 2LL)) == 34LL)))
                                {
                                    (in_triple = false);
                                    (i = (r + 3LL));
                                    break;
                                }
                                (r = (r + 1LL));
                            }
                            if (in_triple)
                            {
                                break;
                            }
                            continue;
                        }
                        if ((ch == 34LL))
                        {
                            (in_str = true);
                            (i = (i + 1LL));
                            continue;
                        }
                        if (((ch == 47LL) && (nxt == 47LL)))
                        {
                            break;
                        }
                        if (((ch == 47LL) && (nxt == 42LL)))
                        {
                            (in_block = true);
                            (i = (i + 2LL));
                            char* kuyruk = intern(upp_metin_kes(line, i, (llen - i)));
                            if (bicim_icinde(kuyruk, "*/"))
                            {
                                (in_block = false);
                            }
                            break;
                        }
                        if ((ch == 123LL))
                        {
                            (depth = (depth + 1LL));
                        }
                        else
                            if ((ch == 125LL))
                            {
                                if ((depth > 0LL))
                                {
                                    (depth = (depth - 1LL));
                                }
                            }
                        (i = (i + 1LL));
                    }
                }
            }
            if (((p < n) && (upp_metin_kod(src, p) == 13LL)))
            {
                (p = (p + 1LL));
            }
            if (((p < n) && (upp_metin_kod(src, p) == 10LL)))
            {
                (p = (p + 1LL));
            }
        }
        return intern(out);
    }
}

char* bicim_norm(char* ham) {
    {
        if ((ham == NULL))
        {
            (ham = "");
        }
        char* o = upp_metin_kopya("");
        long long i = 0LL;
        long long n = upp_uzunluk(ham);
        if (((n > 0LL) && (upp_metin_kod(ham, 0LL) == 65279LL)))
        {
            (i = 1LL);
        }
        while ((i < n))
        {
            if ((upp_metin_kod(ham, i) != 13LL))
            {
                upp_metin_ekle(&(o), upp_metin_kes(ham, i, 1LL));
            }
            (i = (i + 1LL));
        }
        return intern(o);
    }
}

long long os_bicim_dosya(char* yol, long long kontrol) {
    {
        if (((yol == NULL) || metin_esit(yol, "")))
        {
            return 1LL;
        }
        char* ham = upp_dosya_oku(yol);
        if ((ham == NULL))
        {
            (ham = "");
        }
        char* yeni = bicim_kaynak(ham);
        if ((kontrol != 0LL))
        {
            if ((!metin_esit(bicim_norm(ham), yeni)))
            {
                return 1LL;
            }
            return 0LL;
        }
        if ((!os_dosya_yaz(yol, yeni)))
        {
            return 1LL;
        }
        return 0LL;
    }
}

void havuz_sifirla(void) {
    {
        upp_liste_bosalt(&(d_tur));
        upp_liste_bosalt(&(d_satir));
        upp_liste_bosalt(&(d_sutun));
        upp_liste_bosalt(&(d_ad));
        upp_liste_bosalt(&(d_kaynak));
        upp_liste_bosalt(&(d_sayi));
        upp_liste_bosalt(&(d_a));
        upp_liste_bosalt(&(d_b));
        upp_liste_bosalt(&(d_c));
        upp_liste_bosalt(&(d_d));
        upp_liste_bosalt(&(d_liste));
        upp_liste_bosalt(&(d_liste2));
        upp_liste_bosalt(&(d_ptrd));
        upp_liste_bosalt(&(d_dims));
        upp_liste_bosalt(&(d_bayrak));
        (d_n = 0LL);
        upp_liste_bosalt(&(l_ofset));
        upp_liste_bosalt(&(l_uzunluk));
        upp_liste_bosalt(&(l_kap));
        upp_liste_bosalt(&(l_veri));
        upp_liste_temizle(&(l_bos_of));
        upp_liste_temizle(&(l_bos_kap));
        (l_n = 0LL);
        upp_liste_bosalt(&(j_tur));
        upp_liste_bosalt(&(j_deger));
        upp_liste_bosalt(&(j_satir));
        upp_liste_bosalt(&(j_sutun));
        (j_n = 0LL);
        kapsam_sifirla();
        tablo_sifirla();
        tampon_sifirla();
        bg_yigin_sifirla();
        (g_emit_hedef = 0LL);
        (g_ayristir_i = 0LL);
        (g_loop = 0LL);
        (g_switch = 0LL);
        (g_bg_mod = 0LL);
        (g_tmp = 0LL);
        (g_bg_id = 0LL);
        hata_analiz_topla_ayarla(0LL);
        hata_sifirla();
        intern_sifirla();
    }
}

bool hata_var(void) {
    {
        return (g_hata_bayrak != 0LL);
    }
}

void hata_sifirla(void) {
    {
        (g_hata_bayrak = 0LL);
        (g_hata_guvenlik = 0LL);
        (g_hata_mesaj = metin_kopya_yeni(""));
        if ((g_analiz_topla == 0LL))
        {
            (a_n = 0LL);
        }
    }
}

void hata_ver(long long satir, long long sutun, char* mesaj) {
    {
        if ((mesaj == NULL))
        {
            (mesaj = "");
        }
        if ((g_analiz_topla != 0LL))
        {
            if ((a_n < 50LL))
            {
                if ((a_n == upp_liste_uzunluk(a_satir)))
                {
                    upp_liste_ekle(&(a_satir), 0, (long long)(satir), 0.0, NULL);
                    upp_liste_ekle(&(a_sutun), 0, (long long)(((sutun < 1LL) ? 1LL : sutun)), 0.0, NULL);
                    upp_liste_ekle(&(a_gv), 0, (long long)(0LL), 0.0, NULL);
                    upp_liste_ekle(&(a_mesaj), 3, 0LL, 0.0, mesaj);
                }
                else
                {
                    upp_liste_yaz(&(a_satir), 0, a_n, (long long)(satir), 0.0, NULL);
                    upp_liste_yaz(&(a_sutun), 0, a_n, (long long)(((sutun < 1LL) ? 1LL : sutun)), 0.0, NULL);
                    upp_liste_yaz(&(a_gv), 0, a_n, (long long)(0LL), 0.0, NULL);
                    upp_liste_yaz(&(a_mesaj), 3, a_n, 0LL, 0.0, mesaj);
                }
                (a_n = (a_n + 1LL));
            }
            return;
        }
        if ((g_hata_bayrak != 0LL))
        {
            return;
        }
        (g_hata_bayrak = 1LL);
        (g_hata_guvenlik = 0LL);
        (g_hata_satir = satir);
        (g_hata_sutun = ((sutun < 1LL) ? 1LL : sutun));
        (g_hata_mesaj = mesaj);
    }
}

void hata_guvenlik(long long satir, long long sutun, char* mesaj) {
    {
        if ((mesaj == NULL))
        {
            (mesaj = "");
        }
        if ((g_analiz_topla != 0LL))
        {
            if ((a_n < 50LL))
            {
                if ((a_n == upp_liste_uzunluk(a_satir)))
                {
                    upp_liste_ekle(&(a_satir), 0, (long long)(satir), 0.0, NULL);
                    upp_liste_ekle(&(a_sutun), 0, (long long)(((sutun < 1LL) ? 1LL : sutun)), 0.0, NULL);
                    upp_liste_ekle(&(a_gv), 0, (long long)(1LL), 0.0, NULL);
                    upp_liste_ekle(&(a_mesaj), 3, 0LL, 0.0, mesaj);
                }
                else
                {
                    upp_liste_yaz(&(a_satir), 0, a_n, (long long)(satir), 0.0, NULL);
                    upp_liste_yaz(&(a_sutun), 0, a_n, (long long)(((sutun < 1LL) ? 1LL : sutun)), 0.0, NULL);
                    upp_liste_yaz(&(a_gv), 0, a_n, (long long)(1LL), 0.0, NULL);
                    upp_liste_yaz(&(a_mesaj), 3, a_n, 0LL, 0.0, mesaj);
                }
                (a_n = (a_n + 1LL));
            }
            return;
        }
        if ((g_hata_bayrak != 0LL))
        {
            return;
        }
        (g_hata_bayrak = 1LL);
        (g_hata_guvenlik = 1LL);
        (g_hata_satir = satir);
        (g_hata_sutun = ((sutun < 1LL) ? 1LL : sutun));
        (g_hata_mesaj = mesaj);
    }
}

void hata_bas(void) {
    {
        char* pref = upp_metin_kopya("[u++ HATA]");
        if ((g_hata_guvenlik != 0LL))
        {
            (pref = "[u++ BELLEK GÜVENLİĞİ İHLALİ]");
        }
        char* ad = derleme_kaynak_ad();
        char* o = upp_metin_kopya("");
        upp_metin_ekle(&(o), pref);
        upp_metin_ekle(&(o), " Satır ");
        upp_metin_ekle(&(o), sayi_metin(g_hata_satir));
        upp_metin_ekle(&(o), ":");
        upp_metin_ekle(&(o), sayi_metin(g_hata_sutun));
        if (((ad != NULL) && (!metin_esit(ad, ""))))
        {
            upp_metin_ekle(&(o), " (");
            upp_metin_ekle(&(o), ad);
            upp_metin_ekle(&(o), ")");
        }
        upp_metin_ekle(&(o), ": ");
        upp_metin_ekle(&(o), g_hata_mesaj);
        fprintf(stderr, "%s\n", ((o) ? (o) : ""));
        if ((g_hata_guvenlik != 0LL))
        {
            fprintf(stderr, "%s\n", "  -> İpucu: Bu işlemi 'guvensiz { ... }' bloğu içerisine alın.");
        }
    }
}

void hata_fmt_basla(void) {
    {
        (g_htampon = metin_kopya_yeni(""));
    }
}

void hata_fmt_yaz(char* s) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        upp_metin_ekle(&(g_htampon), s);
    }
}

void hata_fmt_sayi(long long n) {
    {
        upp_metin_ekle(&(g_htampon), sayi_metin(n));
    }
}

char* hata_fmt_al(void) {
    {
        return g_htampon;
    }
}

void hata_fmt_ver(long long satir, long long sutun) {
    {
        hata_ver(satir, sutun, hata_fmt_al());
    }
}

void hata_fmt_guvenlik(long long satir, long long sutun) {
    {
        hata_guvenlik(satir, sutun, hata_fmt_al());
    }
}

void hata_json_mod_ayarla(long long v) {
    {
        if ((v != 0LL))
        {
            (g_json_mod = 1LL);
        }
        else
        {
            (g_json_mod = 0LL);
        }
    }
}

bool hata_json_mod(void) {
    {
        return (g_json_mod != 0LL);
    }
}

char* json_hex_hane(long long n) {
    {
        char* h = upp_metin_kopya("0123456789abcdef");
        return upp_metin_kes(h, n, 1LL);
    }
}

char* json_kacis(char* s) {
    {
        if ((s == NULL))
        {
            (s = "");
        }
        char* o = upp_metin_kopya("\"");
        long long i = 0LL;
        long long n = upp_uzunluk(s);
        while ((i < n))
        {
            long long c = upp_metin_kod(s, i);
            if (((c == 34LL) || (c == 92LL)))
            {
                upp_metin_ekle(&(o), "\\");
                upp_metin_ekle(&(o), upp_metin_kes(s, i, 1LL));
            }
            else
                if ((c == 10LL))
                {
                    upp_metin_ekle(&(o), "\\n");
                }
                else
                    if ((c == 13LL))
                    {
                        upp_metin_ekle(&(o), "\\r");
                    }
                    else
                        if ((c == 9LL))
                        {
                            upp_metin_ekle(&(o), "\\t");
                        }
                        else
                            if ((c < 32LL))
                            {
                                upp_metin_ekle(&(o), "\\u");
                                upp_metin_ekle(&(o), json_hex_hane(((c / 4096LL) % 16LL)));
                                upp_metin_ekle(&(o), json_hex_hane(((c / 256LL) % 16LL)));
                                upp_metin_ekle(&(o), json_hex_hane(((c / 16LL) % 16LL)));
                                upp_metin_ekle(&(o), json_hex_hane((c % 16LL)));
                            }
                            else
                            {
                                upp_metin_ekle(&(o), upp_metin_kes(s, i, 1LL));
                            }
            (i = (i + 1LL));
        }
        upp_metin_ekle(&(o), "\"");
        return o;
    }
}

char* json_hata_nesne(long long satir, long long sutun, char* tur, char* mesaj) {
    {
        if ((satir < 1LL))
        {
            (satir = 1LL);
        }
        if ((sutun < 1LL))
        {
            (sutun = 1LL);
        }
        if ((tur == NULL))
        {
            (tur = "hata");
        }
        if ((mesaj == NULL))
        {
            (mesaj = "");
        }
        char* o = upp_metin_kopya("{\"satir\":");
        upp_metin_ekle(&(o), sayi_metin(satir));
        upp_metin_ekle(&(o), ",\"sutun\":");
        upp_metin_ekle(&(o), sayi_metin(sutun));
        upp_metin_ekle(&(o), ",\"tur\":");
        upp_metin_ekle(&(o), json_kacis(tur));
        upp_metin_ekle(&(o), ",\"mesaj\":");
        upp_metin_ekle(&(o), json_kacis(mesaj));
        upp_metin_ekle(&(o), "}");
        return o;
    }
}

void hata_json_yaz(long long satir, long long sutun, char* tur, char* mesaj) {
    {
        char* o = upp_metin_kopya("{\"ok\":false,\"hatalar\":[");
        upp_metin_ekle(&(o), json_hata_nesne(satir, sutun, tur, mesaj));
        upp_metin_ekle(&(o), "]}\n");
        printf("%s", ((o) ? (o) : ""));
    }
}

void hata_json_ok(void) {
    {
        printf("%s", "{\"ok\":true,\"hatalar\":[]}\n");
    }
}

void hata_json_bas(void) {
    {
        if ((a_n > 0LL))
        {
            char* o = upp_metin_kopya("{\"ok\":false,\"hatalar\":[");
            long long i = 0LL;
            while ((i < a_n))
            {
                if ((i > 0LL))
                {
                    upp_metin_ekle(&(o), ",");
                }
                char* tur = upp_metin_kopya("hata");
                if ((upp_liste_al_i(a_gv, i) != 0LL))
                {
                    (tur = "guvenlik");
                }
                upp_metin_ekle(&(o), json_hata_nesne(upp_liste_al_i(a_satir, i), upp_liste_al_i(a_sutun, i), tur, upp_liste_al_s(a_mesaj, i)));
                (i = (i + 1LL));
            }
            upp_metin_ekle(&(o), "]}\n");
            printf("%s", ((o) ? (o) : ""));
            return;
        }
        char* tur = upp_metin_kopya("hata");
        if ((g_hata_guvenlik != 0LL))
        {
            (tur = "guvenlik");
        }
        hata_json_yaz(g_hata_satir, g_hata_sutun, tur, g_hata_mesaj);
    }
}

void hata_analiz_topla_ayarla(long long v) {
    {
        if ((v != 0LL))
        {
            (g_analiz_topla = 1LL);
            (a_n = 0LL);
            (g_hata_bayrak = 0LL);
        }
        else
        {
            (g_analiz_topla = 0LL);
        }
    }
}

long long hata_analiz_say(void) {
    {
        return a_n;
    }
}

bool hata_analiz_mi(void) {
    {
        return (g_analiz_topla != 0LL);
    }
}

bool dugum_tur_mi(long long id, long long tur) {
    {
        if ((id == 0LL))
        {
            return (tur == DugumTur_D_YOK);
        }
        return (dugum_tur(id) == tur);
    }
}

long long tur_dugum(long long satir, long long sutun, char* ad, long long ptrd, long long dims) {
    {
        long long n = dugum_yeni(DugumTur_D_TUR, satir, sutun);
        dugum_yaz_ad(n, ad);
        dugum_yaz_ptrd(n, ptrd);
        dugum_yaz_dims(n, dims);
        return n;
    }
}

long long tur_kopya(long long t) {
    {
        if ((t == 0LL))
        {
            return tur_dugum(0LL, 1LL, "bos", 0LL, 0LL);
        }
        long long n = tur_dugum(dugum_satir(t), dugum_sutun(t), dugum_ad(t), dugum_ptrd(t), dugum_dims(t));
        dugum_yaz_sayi(n, dugum_sayi(t));
        dugum_yaz_bayrak(n, dugum_bayrak(t));
        return n;
    }
}

char* tur_aciklama(long long t) {
    {
        long long i = 0LL;
        if ((t == 0LL))
        {
            return "bos";
        }
        hata_fmt_basla();
        hata_fmt_yaz(dugum_ad(t));
        for ((i = 0LL); (i < dugum_ptrd(t)); (i = (i + 1LL)))
        {
            hata_fmt_yaz("*");
        }
        for ((i = 0LL); (i < dugum_dims(t)); (i = (i + 1LL)))
        {
            hata_fmt_yaz("[]");
        }
        return intern(hata_fmt_al());
    }
}

long long sabit_sayi(long long satir, long long sutun, long long v) {
    {
        long long n = dugum_yeni(DugumTur_D_SABIT, satir, sutun);
        dugum_yaz_ad(n, "sayi");
        dugum_yaz_sayi(n, v);
        return n;
    }
}

long long kimlik_dugum(long long satir, long long sutun, char* ad) {
    {
        long long n = dugum_yeni(DugumTur_D_KIMLIK, satir, sutun);
        dugum_yaz_ad(n, ad);
        return n;
    }
}

bool anahtar_mi(char* s) {
    {
        if (metin_esit(s, "fonk"))
        {
            return true;
        }
        if (metin_esit(s, "don"))
        {
            return true;
        }
        if (metin_esit(s, "eger"))
        {
            return true;
        }
        if (metin_esit(s, "yoksa"))
        {
            return true;
        }
        if (metin_esit(s, "iken"))
        {
            return true;
        }
        if (metin_esit(s, "dongu"))
        {
            return true;
        }
        if (metin_esit(s, "dur"))
        {
            return true;
        }
        if (metin_esit(s, "devam"))
        {
            return true;
        }
        if (metin_esit(s, "sayi"))
        {
            return true;
        }
        if (metin_esit(s, "bayt"))
        {
            return true;
        }
        if (metin_esit(s, "ondalik"))
        {
            return true;
        }
        if (metin_esit(s, "metin"))
        {
            return true;
        }
        if (metin_esit(s, "mantik"))
        {
            return true;
        }
        if (metin_esit(s, "bos"))
        {
            return true;
        }
        if (metin_esit(s, "liste"))
        {
            return true;
        }
        if (metin_esit(s, "harita"))
        {
            return true;
        }
        if (metin_esit(s, "dogru"))
        {
            return true;
        }
        if (metin_esit(s, "yanlis"))
        {
            return true;
        }
        if (metin_esit(s, "yok"))
        {
            return true;
        }
        if (metin_esit(s, "guvensiz"))
        {
            return true;
        }
        if (metin_esit(s, "guvenli"))
        {
            return true;
        }
        if (metin_esit(s, "arkaplan"))
        {
            return true;
        }
        if (metin_esit(s, "sinif"))
        {
            return true;
        }
        if (metin_esit(s, "kendi"))
        {
            return true;
        }
        if (metin_esit(s, "oto"))
        {
            return true;
        }
        if (metin_esit(s, "c_ekle"))
        {
            return true;
        }
        if (metin_esit(s, "secim"))
        {
            return true;
        }
        if (metin_esit(s, "durum"))
        {
            return true;
        }
        if (metin_esit(s, "varsayilan"))
        {
            return true;
        }
        if (metin_esit(s, "her"))
        {
            return true;
        }
        if (metin_esit(s, "in"))
        {
            return true;
        }
        if (metin_esit(s, "secenek"))
        {
            return true;
        }
        if (metin_esit(s, "kullan"))
        {
            return true;
        }
        if (metin_esit(s, "olarak"))
        {
            return true;
        }
        if (metin_esit(s, "sabit"))
        {
            return true;
        }
        return false;
    }
}

bool tur_adi_mi(char* s) {
    {
        if (metin_esit(s, "sayi"))
        {
            return true;
        }
        if (metin_esit(s, "bayt"))
        {
            return true;
        }
        if (metin_esit(s, "ondalik"))
        {
            return true;
        }
        if (metin_esit(s, "metin"))
        {
            return true;
        }
        if (metin_esit(s, "mantik"))
        {
            return true;
        }
        if (metin_esit(s, "bos"))
        {
            return true;
        }
        if (metin_esit(s, "liste"))
        {
            return true;
        }
        if (metin_esit(s, "harita"))
        {
            return true;
        }
        return false;
    }
}

bool koleksiyon_elem_mi(char* s) {
    {
        if (metin_esit(s, "sayi"))
        {
            return true;
        }
        if (metin_esit(s, "bayt"))
        {
            return true;
        }
        if (metin_esit(s, "ondalik"))
        {
            return true;
        }
        if (metin_esit(s, "mantik"))
        {
            return true;
        }
        if (metin_esit(s, "metin"))
        {
            return true;
        }
        return false;
    }
}

bool liste_tur_mi(char* n) {
    {
        long long L = metin_uzun(n);
        if ((L < 8LL))
        {
            return false;
        }
        if ((!metin_esit(intern_dilim(n, 0LL, 6LL), "liste[")))
        {
            return false;
        }
        if ((metin_cp(n, (L - 1LL)) != 93LL))
        {
            return false;
        }
        return true;
    }
}

char* liste_elem_ad(char* n) {
    {
        return intern_dilim(n, 6LL, (metin_uzun(n) - 1LL));
    }
}

bool harita_tur_mi(char* n) {
    {
        long long L = metin_uzun(n);
        if ((L < 10LL))
        {
            return false;
        }
        if ((!metin_esit(intern_dilim(n, 0LL, 7LL), "harita[")))
        {
            return false;
        }
        if ((metin_cp(n, (L - 1LL)) != 93LL))
        {
            return false;
        }
        return true;
    }
}

char* harita_anahtar_ad(char* n) {
    {
        long long i = 7LL;
        long long L = metin_uzun(n);
        while ((i < (L - 1LL)))
        {
            if ((metin_cp(n, i) == 44LL))
            {
                return intern_dilim(n, 7LL, i);
            }
            (i = (i + 1LL));
        }
        return "";
    }
}

char* harita_deger_ad(char* n) {
    {
        long long i = 7LL;
        long long L = metin_uzun(n);
        while ((i < (L - 1LL)))
        {
            if ((metin_cp(n, i) == 44LL))
            {
                return intern_dilim(n, (i + 1LL), (L - 1LL));
            }
            (i = (i + 1LL));
        }
        return "";
    }
}

long long koleksiyon_etiket(char* e) {
    {
        if (metin_esit(e, "sayi"))
        {
            return 0LL;
        }
        if (metin_esit(e, "ondalik"))
        {
            return 1LL;
        }
        if (metin_esit(e, "mantik"))
        {
            return 2LL;
        }
        if (metin_esit(e, "metin"))
        {
            return 3LL;
        }
        if (metin_esit(e, "bayt"))
        {
            return 4LL;
        }
        return 0LL;
    }
}

bool interp_ad_mi(char* name) {
    {
        long long n = metin_uzun(name);
        long long i = 0LL;
        if ((n == 0LL))
        {
            return false;
        }
        if ((!ident_bas(metin_cp(name, 0LL))))
        {
            return false;
        }
        (i = metin_adv(name, 0LL));
        while ((i < n))
        {
            long long c = metin_cp(name, i);
            long long adv = metin_adv(name, i);
            if ((c == 46LL))
            {
                (i = (i + adv));
                if (((i >= n) || (!ident_bas(metin_cp(name, i)))))
                {
                    return false;
                }
                (i = (i + metin_adv(name, i)));
                continue;
            }
            if ((!ident_devam(c)))
            {
                return false;
            }
            (i = (i + adv));
        }
        return true;
    }
}

bool yerlesik_sinif_mi(char* s) {
    {
        if (metin_esit(s, "Yonelme"))
        {
            return true;
        }
        if (metin_esit(s, "EkranNokta"))
        {
            return true;
        }
        if (metin_esit(s, "Vektor3"))
        {
            return true;
        }
        if (metin_esit(s, "ArkaplanIs"))
        {
            return true;
        }
        if (metin_esit(s, "JSONDeger"))
        {
            return true;
        }
        if (metin_esit(s, "SurecCikti"))
        {
            return true;
        }
        if (metin_esit(s, "Kilit"))
        {
            return true;
        }
        return false;
    }
}

char* c_sembol(char* ad, char* alias) {
    {
        if ((metin_esit(alias, "") || (alias == NULL)))
        {
            return ad;
        }
        return kat3(alias, "_", ad);
    }
}

char* nokta_altcizgi(char* n) {
    {
        long long i = 0LL;
        long long L = metin_uzun(n);
        char* s = upp_metin_kopya("");
        while ((i < L))
        {
            long long c = metin_cp(n, i);
            long long adv = metin_adv(n, i);
            if ((c == 46LL))
            {
                (s = kat(s, "_"));
            }
            else
            {
                (s = kat(s, intern_dilim(n, i, (i + adv))));
            }
            (i = (i + adv));
        }
        return intern(s);
    }
}

char* dugum_alias_ad(long long n) {
    {
        if (((n == 0LL) || (dugum_d(n) == 0LL)))
        {
            return "";
        }
        if ((dugum_tur(dugum_d(n)) != DugumTur_D_KIMLIK))
        {
            return "";
        }
        return dugum_ad(dugum_d(n));
    }
}

char* dugum_c_ad(long long n) {
    {
        return c_sembol(dugum_ad(n), dugum_alias_ad(n));
    }
}

char* sinif_tur_ad(long long cls) {
    {
        char* alias = dugum_alias_ad(cls);
        if (metin_esit(alias, ""))
        {
            return dugum_ad(cls);
        }
        return intern(kat3(alias, ".", dugum_ad(cls)));
    }
}

char* metot_sinif_tur_ad(long long fn) {
    {
        if (((fn == 0LL) || (dugum_c(fn) == 0LL)))
        {
            return "";
        }
        char* ad = dugum_ad(dugum_c(fn));
        char* alias = dugum_alias_ad(fn);
        if (metin_esit(alias, ""))
        {
            return ad;
        }
        return intern(kat3(alias, ".", ad));
    }
}

char* metot_sinif_c_ad(long long fn) {
    {
        if (((fn == 0LL) || (dugum_c(fn) == 0LL)))
        {
            return "";
        }
        return c_sembol(dugum_ad(dugum_c(fn)), dugum_alias_ad(fn));
    }
}

void lex_yorum_satir(void) {
    {
        lex_ilerle();
        lex_ilerle();
        long long bas = lex_pos();
        while (((!lex_bitis()) && (lex_cp() != 10LL)))
        {
            lex_ilerle();
        }
        long long son = lex_pos();
        lex_yorum_govde(lex_dilim(bas, son));
    }
}

void lex_blok_yorum(void) {
    {
        long long satir = lex_satir();
        long long sutun = lex_sutun();
        lex_ilerle();
        lex_ilerle();
        while ((!lex_bitis()))
        {
            if (((lex_cp() == 42LL) && (lex_cp_ileri(1LL) == 47LL)))
            {
                lex_ilerle();
                lex_ilerle();
                return;
            }
            lex_ilerle();
        }
        hata_ver(satir, sutun, "Kapatılmamış blok yorumu (/* ... */)");
    }
}

void lex_bosluk_at(void) {
    {
        while ((!lex_bitis()))
        {
            long long ch = lex_cp();
            if (((((ch == 32LL) || (ch == 9LL)) || (ch == 13LL)) || (ch == 10LL)))
            {
                lex_ilerle();
                continue;
            }
            if (((ch == 47LL) && (lex_cp_ileri(1LL) == 47LL)))
            {
                lex_yorum_satir();
                continue;
            }
            if (((ch == 47LL) && (lex_cp_ileri(1LL) == 42LL)))
            {
                lex_blok_yorum();
                continue;
            }
            break;
        }
    }
}

bool lex_sonraki_bosluksuz_suslu(void) {
    {
        char* s = lex_kaynak_metin();
        long long i = lex_pos();
        long long n = upp_uzunluk(s);
        while ((i < n))
        {
            long long cp = upp_metin_kod(s, i);
            if (((((cp == 32LL) || (cp == 9LL)) || (cp == 13LL)) || (cp == 10LL)))
            {
                (i = (i + 1LL));
                continue;
            }
            if (((cp == 47LL) && ((i + 1LL) < n)))
            {
                long long n1 = upp_metin_kod(s, (i + 1LL));
                if ((n1 == 47LL))
                {
                    (i = (i + 2LL));
                    while (((i < n) && (upp_metin_kod(s, i) != 10LL)))
                    {
                        (i = (i + 1LL));
                    }
                    continue;
                }
                if ((n1 == 42LL))
                {
                    (i = (i + 2LL));
                    while ((((i + 1LL) < n) && (!((upp_metin_kod(s, i) == 42LL) && (upp_metin_kod(s, (i + 1LL)) == 47LL)))))
                    {
                        (i = (i + 1LL));
                    }
                    if (((i + 1LL) < n))
                    {
                        (i = (i + 2LL));
                    }
                    continue;
                }
            }
            return (cp == 123LL);
        }
        return false;
    }
}

long long lex_gomulu(long long tur, long long satir, long long sutun) {
    {
        lex_bosluk_at();
        if ((lex_cp() != 123LL))
        {
            hata_ver(satir, sutun, "Gömülü C/asm bloğu '{' ile başlamalı.");
            return 0LL;
        }
        lex_ilerle();
        long long inner_bas = lex_pos();
        long long derinlik = 1LL;
        bool in_str = false;
        bool in_chr = false;
        bool kacis = false;
        bool in_line = false;
        bool in_block = false;
        while (((!lex_bitis()) && (derinlik > 0LL)))
        {
            long long ch = lex_cp();
            long long nxt = lex_cp_ileri(1LL);
            if (in_line)
            {
                lex_ilerle();
                if ((ch == 10LL))
                {
                    (in_line = false);
                }
                continue;
            }
            if (in_block)
            {
                lex_ilerle();
                if (((ch == 42LL) && (nxt == 47LL)))
                {
                    if ((!lex_bitis()))
                    {
                        lex_ilerle();
                    }
                    (in_block = false);
                }
                continue;
            }
            if ((in_str || in_chr))
            {
                lex_ilerle();
                if (kacis)
                {
                    (kacis = false);
                    continue;
                }
                if ((ch == 92LL))
                {
                    (kacis = true);
                    continue;
                }
                if ((in_str && (ch == 34LL)))
                {
                    (in_str = false);
                }
                else
                    if ((in_chr && (ch == 39LL)))
                    {
                        (in_chr = false);
                    }
                continue;
            }
            if (((ch == 47LL) && (nxt == 47LL)))
            {
                (in_line = true);
                lex_ilerle();
                continue;
            }
            if (((ch == 47LL) && (nxt == 42LL)))
            {
                (in_block = true);
                lex_ilerle();
                continue;
            }
            if ((ch == 34LL))
            {
                (in_str = true);
                lex_ilerle();
                continue;
            }
            if ((ch == 39LL))
            {
                (in_chr = true);
                lex_ilerle();
                continue;
            }
            if ((ch == 123LL))
            {
                (derinlik = (derinlik + 1LL));
                lex_ilerle();
                continue;
            }
            if ((ch == 125LL))
            {
                (derinlik = (derinlik - 1LL));
                lex_ilerle();
                continue;
            }
            lex_ilerle();
        }
        if ((derinlik != 0LL))
        {
            hata_ver(satir, sutun, "Kapatılmamış gömülü C/asm bloğu.");
            return 0LL;
        }
        char* inner = lex_dilim(inner_bas, (lex_pos() - 1LL));
        return jeton_yeni(tur, inner, satir, sutun);
    }
}

long long lex_metin(void) {
    {
        long long satir = lex_satir();
        long long sutun = lex_sutun();
        long long bas = lex_pos();
        lex_ilerle();
        if ((((!lex_bitis()) && (lex_cp() == 34LL)) && (lex_cp_ileri(1LL) == 34LL)))
        {
            lex_ilerle();
            lex_ilerle();
            while ((!lex_bitis()))
            {
                if ((((lex_cp() == 34LL) && (lex_cp_ileri(1LL) == 34LL)) && (lex_cp_ileri(2LL) == 34LL)))
                {
                    lex_ilerle();
                    lex_ilerle();
                    lex_ilerle();
                    return jeton_yeni(JetonTur_J_METIN, lex_dilim(bas, lex_pos()), satir, sutun);
                }
                lex_ilerle();
            }
            hata_ver(satir, sutun, "Kapatılmamış çok satırlı metin sabiti (\"\"\"...\"\"\").");
            return 0LL;
        }
        while ((!lex_bitis()))
        {
            long long ch = lex_cp();
            if ((ch == 10LL))
            {
                hata_ver(satir, sutun, "Metin sabitinde satır sonu. Metni kapatın veya \\n kullanın.");
                return 0LL;
            }
            lex_ilerle();
            if ((ch == 92LL))
            {
                if (lex_bitis())
                {
                    hata_ver(satir, sutun, "Metin sabiti kaçış karakterinden sonra bitti.");
                    return 0LL;
                }
                lex_ilerle();
                continue;
            }
            if ((ch == 34LL))
            {
                return jeton_yeni(JetonTur_J_METIN, lex_dilim(bas, lex_pos()), satir, sutun);
            }
        }
        hata_ver(satir, sutun, "Kapatılmamış metin sabiti.");
        return 0LL;
    }
}

long long lex_sayi(void) {
    {
        long long satir = lex_satir();
        long long sutun = lex_sutun();
        long long bas = lex_pos();
        if (((lex_cp() == 48LL) && ((lex_cp_ileri(1LL) == 120LL) || (lex_cp_ileri(1LL) == 88LL))))
        {
            lex_ilerle();
            lex_ilerle();
            long long digits = 0LL;
            while ((!lex_bitis()))
            {
                long long ch = lex_cp();
                bool hex = ((((ch >= 48LL) && (ch <= 57LL)) || ((ch >= 97LL) && (ch <= 102LL))) || ((ch >= 65LL) && (ch <= 70LL)));
                if ((!hex))
                {
                    break;
                }
                lex_ilerle();
                (digits = (digits + 1LL));
            }
            if ((digits == 0LL))
            {
                hata_ver(satir, sutun, "'0x' sonrası onaltılık basamak bekleniyordu.");
                return 0LL;
            }
            char* lex = lex_dilim(bas, lex_pos());
            return jeton_yeni(JetonTur_J_SAYI, sayi_metin(hex_deger(lex)), satir, sutun);
        }
        while ((((!lex_bitis()) && (lex_cp() >= 48LL)) && (lex_cp() <= 57LL)))
        {
            lex_ilerle();
        }
        if (((!lex_bitis()) && (lex_cp() == 46LL)))
        {
            long long nxt = lex_cp_ileri(1LL);
            if (((nxt >= 48LL) && (nxt <= 57LL)))
            {
                lex_ilerle();
                while ((((!lex_bitis()) && (lex_cp() >= 48LL)) && (lex_cp() <= 57LL)))
                {
                    lex_ilerle();
                }
                return jeton_yeni(JetonTur_J_ONDALIK, lex_dilim(bas, lex_pos()), satir, sutun);
            }
        }
        return jeton_yeni(JetonTur_J_SAYI, lex_dilim(bas, lex_pos()), satir, sutun);
    }
}

void lex_tara(void) {
    {
        while (((!lex_bitis()) && (!hata_var())))
        {
            long long ch = lex_cp();
            if ((((ch == 32LL) || (ch == 9LL)) || (ch == 13LL)))
            {
                lex_ilerle();
                continue;
            }
            if ((ch == 10LL))
            {
                lex_ilerle();
                continue;
            }
            if (((ch == 47LL) && (lex_cp_ileri(1LL) == 47LL)))
            {
                lex_yorum_satir();
                continue;
            }
            if (((ch == 47LL) && (lex_cp_ileri(1LL) == 42LL)))
            {
                lex_blok_yorum();
                continue;
            }
            if ((ch == 34LL))
            {
                lex_metin();
                continue;
            }
            if (((ch >= 48LL) && (ch <= 57LL)))
            {
                lex_sayi();
                continue;
            }
            long long satir = lex_satir();
            long long sutun = lex_sutun();
            if (((ch == 61LL) && (lex_cp_ileri(1LL) == 62LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_YAG_OK, "=>", satir, sutun);
                continue;
            }
            if (((ch == 45LL) && (lex_cp_ileri(1LL) == 62LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_OK, "->", satir, sutun);
                continue;
            }
            if (((ch == 61LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_ESIT_ESIT, "==", satir, sutun);
                continue;
            }
            if (((ch == 33LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_ESIT_DEGIL, "!=", satir, sutun);
                continue;
            }
            if (((ch == 60LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_KUCUK_ESIT, "<=", satir, sutun);
                continue;
            }
            if (((ch == 62LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_BUYUK_ESIT, ">=", satir, sutun);
                continue;
            }
            if (((ch == 38LL) && (lex_cp_ileri(1LL) == 38LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_VE, "&&", satir, sutun);
                continue;
            }
            if (((ch == 124LL) && (lex_cp_ileri(1LL) == 124LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_VEYA, "||", satir, sutun);
                continue;
            }
            if (((ch == 43LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_ARTI_ATAMA, "+=", satir, sutun);
                continue;
            }
            if (((ch == 45LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_EKSI_ATAMA, "-=", satir, sutun);
                continue;
            }
            if (((ch == 42LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_YILDIZ_ATAMA, "*=", satir, sutun);
                continue;
            }
            if (((ch == 47LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_BOLU_ATAMA, "/=", satir, sutun);
                continue;
            }
            if (((ch == 37LL) && (lex_cp_ileri(1LL) == 61LL)))
            {
                lex_ilerle();
                lex_ilerle();
                jeton_yeni(JetonTur_J_MOD_ATAMA, "%=", satir, sutun);
                continue;
            }
            if ((ch == 63LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_SORU, "?", satir, sutun);
                continue;
            }
            if ((ch == 43LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_ARTI, "+", satir, sutun);
                continue;
            }
            if ((ch == 45LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_EKSI, "-", satir, sutun);
                continue;
            }
            if ((ch == 42LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_YILDIZ, "*", satir, sutun);
                continue;
            }
            if ((ch == 47LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_BOLU, "/", satir, sutun);
                continue;
            }
            if ((ch == 37LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_MOD, "%", satir, sutun);
                continue;
            }
            if ((ch == 61LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_ATAMA, "=", satir, sutun);
                continue;
            }
            if ((ch == 60LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_KUCUK, "<", satir, sutun);
                continue;
            }
            if ((ch == 62LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_BUYUK, ">", satir, sutun);
                continue;
            }
            if ((ch == 33LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_UNLEM, "!", satir, sutun);
                continue;
            }
            if ((ch == 38LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_VE_ISARET, "&", satir, sutun);
                continue;
            }
            if ((ch == 124LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_VAY_ISARET, "|", satir, sutun);
                continue;
            }
            if ((ch == 94LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_XOR, "^", satir, sutun);
                continue;
            }
            if ((ch == 40LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_PARANTEZ_AC, "(", satir, sutun);
                continue;
            }
            if ((ch == 41LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_PARANTEZ_KAPA, ")", satir, sutun);
                continue;
            }
            if ((ch == 123LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_SLUS_AC, "{", satir, sutun);
                continue;
            }
            if ((ch == 125LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_SLUS_KAPA, "}", satir, sutun);
                continue;
            }
            if ((ch == 91LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_KOSE_AC, "[", satir, sutun);
                continue;
            }
            if ((ch == 93LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_KOSE_KAPA, "]", satir, sutun);
                continue;
            }
            if ((ch == 59LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_NOKTALI_VIRGUL, ";", satir, sutun);
                continue;
            }
            if ((ch == 44LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_VIRGUL, ",", satir, sutun);
                continue;
            }
            if ((ch == 46LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_NOKTA, ".", satir, sutun);
                continue;
            }
            if ((ch == 58LL))
            {
                lex_ilerle();
                jeton_yeni(JetonTur_J_IKI_NOKTA, ":", satir, sutun);
                continue;
            }
            if (ident_bas(ch))
            {
                long long bas = lex_pos();
                while (((!lex_bitis()) && ident_devam(lex_cp())))
                {
                    lex_ilerle();
                }
                char* deger = lex_dilim(bas, lex_pos());
                if (((metin_esit(deger, "c_kod") || metin_esit(deger, "asm_kod")) && lex_sonraki_bosluksuz_suslu()))
                {
                    long long tur = JetonTur_J_C_GOMMU;
                    if (metin_esit(deger, "asm_kod"))
                    {
                        (tur = JetonTur_J_ASM_GOMMU);
                    }
                    lex_gomulu(tur, satir, sutun);
                    continue;
                }
                long long tur = JetonTur_J_KIMLIK;
                if (anahtar_mi(deger))
                {
                    (tur = JetonTur_J_ANAHTAR);
                }
                jeton_yeni(tur, deger, satir, sutun);
                continue;
            }
            hata_fmt_basla();
            hata_fmt_yaz("Tanımlanamayan simge.");
            hata_fmt_ver(satir, sutun);
            return;
        }
        jeton_yeni(JetonTur_J_EOF, "", lex_satir(), lex_sutun());
    }
}

long long jeton_id_at(long long off) {
    {
        long long i = (ayristir_i_al() + off);
        long long n = jeton_sayisi();
        if ((n <= 1LL))
        {
            return 0LL;
        }
        if ((i < 1LL))
        {
            (i = 1LL);
        }
        if ((i >= n))
        {
            return (n - 1LL);
        }
        return i;
    }
}

long long bak_tur(void) {
    {
        return jeton_tur(jeton_id_at(0LL));
    }
}

char* bak_deger(void) {
    {
        return jeton_deger(jeton_id_at(0LL));
    }
}

long long bak_satir(void) {
    {
        return jeton_satir(jeton_id_at(0LL));
    }
}

long long bak_sutun(void) {
    {
        return jeton_sutun(jeton_id_at(0LL));
    }
}

long long bak_tur_at(long long off) {
    {
        return jeton_tur(jeton_id_at(off));
    }
}

char* bak_deger_at(long long off) {
    {
        return jeton_deger(jeton_id_at(off));
    }
}

long long j_ilerle(void) {
    {
        long long id = jeton_id_at(0LL);
        if ((bak_tur() != JetonTur_J_EOF))
        {
            ayristir_i_yaz((ayristir_i_al() + 1LL));
        }
        return id;
    }
}

bool esles_tur(long long t) {
    {
        if ((bak_tur() == t))
        {
            j_ilerle();
            return true;
        }
        return false;
    }
}

bool esles_kw(char* w) {
    {
        if (((bak_tur() == JetonTur_J_ANAHTAR) && metin_esit(bak_deger(), w)))
        {
            j_ilerle();
            return true;
        }
        return false;
    }
}

bool kw_mi(char* w) {
    {
        return ((bak_tur() == JetonTur_J_ANAHTAR) && metin_esit(bak_deger(), w));
    }
}

long long beklenen_tur(long long t, char* goster) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        if ((bak_tur() != t))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'");
            hata_fmt_yaz(goster);
            hata_fmt_yaz("' bekleniyordu, '");
            hata_fmt_yaz(bak_deger());
            hata_fmt_yaz("' bulundu.");
            hata_fmt_ver(bak_satir(), bak_sutun());
            return 0LL;
        }
        return j_ilerle();
    }
}

long long beklenen_kw(char* w) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        if ((!kw_mi(w)))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'");
            hata_fmt_yaz(w);
            hata_fmt_yaz("' anahtar sözcüğü bekleniyordu, '");
            hata_fmt_yaz(bak_deger());
            hata_fmt_yaz("' bulundu.");
            hata_fmt_ver(bak_satir(), bak_sutun());
            return 0LL;
        }
        return j_ilerle();
    }
}

long long beklenen_kimlik(bool tur_kw_izin) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        if ((bak_tur() == JetonTur_J_KIMLIK))
        {
            return j_ilerle();
        }
        if ((bak_tur() == JetonTur_J_ANAHTAR))
        {
            if ((tur_kw_izin && tur_adi_mi(bak_deger())))
            {
                return j_ilerle();
            }
            hata_fmt_basla();
            hata_fmt_yaz("'");
            hata_fmt_yaz(bak_deger());
            hata_fmt_yaz("' bir anahtar sözcüktür; tanımlayıcı olarak kullanılamaz.");
            hata_fmt_ver(bak_satir(), bak_sutun());
            return 0LL;
        }
        hata_fmt_basla();
        hata_fmt_yaz("Tanımlayıcı bekleniyordu, '");
        hata_fmt_yaz(bak_deger());
        hata_fmt_yaz("' bulundu.");
        hata_fmt_ver(bak_satir(), bak_sutun());
        return 0LL;
    }
}

char* metin_coz(char* raw) {
    {
        long long n = metin_uzun(raw);
        if ((((((((n >= 6LL) && (metin_cp(raw, 0LL) == 34LL)) && (metin_cp(raw, 1LL) == 34LL)) && (metin_cp(raw, 2LL) == 34LL)) && (metin_cp(raw, (n - 3LL)) == 34LL)) && (metin_cp(raw, (n - 2LL)) == 34LL)) && (metin_cp(raw, (n - 1LL)) == 34LL)))
        {
            char* s = upp_metin_kopya("");
            long long i = 3LL;
            while ((i < (n - 3LL)))
            {
                long long c = metin_cp(raw, i);
                long long adv = metin_adv(raw, i);
                if ((c != 13LL))
                {
                    (s = kat(s, intern_dilim(raw, i, (i + adv))));
                }
                (i = (i + adv));
            }
            return s;
        }
        if ((((n < 2LL) || (metin_cp(raw, 0LL) != 34LL)) || (metin_cp(raw, (n - 1LL)) != 34LL)))
        {
            return raw;
        }
        char* s = upp_metin_kopya("");
        long long i = 1LL;
        while ((i < (n - 1LL)))
        {
            long long c = metin_cp(raw, i);
            long long adv = metin_adv(raw, i);
            if (((c == 92LL) && ((i + adv) < (n - 1LL))))
            {
                long long nxt = metin_cp(raw, (i + adv));
                long long nadv = metin_adv(raw, (i + adv));
                if ((nxt == 110LL))
                {
                    (s = kat(s, "\n"));
                }
                else
                    if ((nxt == 116LL))
                    {
                        (s = kat(s, "\t"));
                    }
                    else
                        if ((nxt == 114LL))
                        {
                            (s = kat(s, "\r"));
                        }
                        else
                            if ((nxt == 92LL))
                            {
                                (s = kat(s, "\\"));
                            }
                            else
                                if ((nxt == 34LL))
                                {
                                    (s = kat(s, "\""));
                                }
                                else
                                {
                                    (s = kat(s, intern_dilim(raw, (i + adv), ((i + adv) + nadv))));
                                }
                (i = ((i + adv) + nadv));
                continue;
            }
            (s = kat(s, intern_dilim(raw, i, (i + adv))));
            (i = (i + adv));
        }
        return s;
    }
}

bool tur_baslangici_mi(void) {
    {
        if (((bak_tur() == JetonTur_J_ANAHTAR) && tur_adi_mi(bak_deger())))
        {
            return true;
        }
        if ((bak_tur() == JetonTur_J_KIMLIK))
        {
            long long i = 1LL;
            if (((bak_tur_at(i) == JetonTur_J_NOKTA) && (bak_tur_at((i + 1LL)) == JetonTur_J_KIMLIK)))
            {
                (i = (i + 2LL));
            }
            while ((bak_tur_at(i) == JetonTur_J_YILDIZ))
            {
                (i = (i + 1LL));
            }
            return (bak_tur_at(i) == JetonTur_J_KIMLIK);
        }
        return false;
    }
}

char* ayristir_koleksiyon_elem(void) {
    {
        if (hata_var())
        {
            return "";
        }
        if (((bak_tur() == JetonTur_J_ANAHTAR) && koleksiyon_elem_mi(bak_deger())))
        {
            char* ad = bak_deger();
            j_ilerle();
            return ad;
        }
        hata_fmt_basla();
        hata_fmt_yaz("Koleksiyon eleman türü 'sayi', 'ondalik', 'mantik' veya 'metin' olmalı, '");
        hata_fmt_yaz(bak_deger());
        hata_fmt_yaz("' bulundu.");
        hata_fmt_ver(bak_satir(), bak_sutun());
        return "";
    }
}

long long ayristir_tur(void) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        long long satir = bak_satir();
        long long sutun = bak_sutun();
        char* ad = upp_metin_kopya("");
        if (((bak_tur() == JetonTur_J_ANAHTAR) && tur_adi_mi(bak_deger())))
        {
            (ad = bak_deger());
            j_ilerle();
        }
        else
            if ((bak_tur() == JetonTur_J_KIMLIK))
            {
                (ad = bak_deger());
                j_ilerle();
                if (esles_tur(JetonTur_J_NOKTA))
                {
                    long long p = beklenen_kimlik(false);
                    (ad = kat3(ad, ".", jeton_deger(p)));
                }
            }
            else
            {
                hata_fmt_basla();
                hata_fmt_yaz("Tür bekleniyordu ('sayi', 'ondalik', 'metin', 'mantik', 'bos', 'liste[T]', 'harita[K,V]' veya sınıf adı), '");
                hata_fmt_yaz(bak_deger());
                hata_fmt_yaz("' bulundu.");
                hata_fmt_ver(satir, sutun);
                return 0LL;
            }
        if (metin_esit(ad, "liste"))
        {
            beklenen_tur(JetonTur_J_KOSE_AC, "[");
            char* elem = ayristir_koleksiyon_elem();
            beklenen_tur(JetonTur_J_KOSE_KAPA, "]");
            (ad = kat3("liste[", elem, "]"));
        }
        else
            if (metin_esit(ad, "harita"))
            {
                beklenen_tur(JetonTur_J_KOSE_AC, "[");
                char* k = ayristir_koleksiyon_elem();
                beklenen_tur(JetonTur_J_VIRGUL, ",");
                char* v = ayristir_koleksiyon_elem();
                beklenen_tur(JetonTur_J_KOSE_KAPA, "]");
                if (((!metin_esit(k, "metin")) && (!metin_esit(k, "sayi"))))
                {
                    hata_ver(satir, sutun, "harita anahtarı 'metin' veya 'sayi' olmalı.");
                    return 0LL;
                }
                (ad = kat(kat3("harita[", k, ","), kat(v, "]")));
            }
        long long depth = 0LL;
        while (esles_tur(JetonTur_J_YILDIZ))
        {
            (depth = (depth + 1LL));
        }
        return tur_dugum(satir, sutun, ad, depth, 0LL);
    }
}

long long ayristir_dizi_sabit(void) {
    {
        long long tok = beklenen_tur(JetonTur_J_KOSE_AC, "[");
        long long n = dugum_yeni(DugumTur_D_DIZI_SABIT, jeton_satir(tok), jeton_sutun(tok));
        long long L = liste_yeni();
        dugum_yaz_liste(n, L);
        if ((bak_tur() != JetonTur_J_KOSE_KAPA))
        {
            liste_ekle(L, ayristir_ifade());
            while (esles_tur(JetonTur_J_VIRGUL))
            {
                if ((bak_tur() == JetonTur_J_KOSE_KAPA))
                {
                    break;
                }
                liste_ekle(L, ayristir_ifade());
            }
        }
        beklenen_tur(JetonTur_J_KOSE_KAPA, "]");
        return n;
    }
}

long long ayristir_baslatici(void) {
    {
        if ((bak_tur() == JetonTur_J_KOSE_AC))
        {
            return ayristir_dizi_sabit();
        }
        return ayristir_ifade();
    }
}

long long ayristir_birincil(void) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        long long satir = bak_satir();
        long long sutun = bak_sutun();
        if ((bak_tur() == JetonTur_J_SAYI))
        {
            char* v = bak_deger();
            j_ilerle();
            return sabit_sayi(satir, sutun, sayi_oku(v));
        }
        if ((bak_tur() == JetonTur_J_ONDALIK))
        {
            long long n = dugum_yeni(DugumTur_D_SABIT, satir, sutun);
            dugum_yaz_ad(n, bak_deger());
            dugum_yaz_ptrd(n, 1LL);
            j_ilerle();
            return n;
        }
        if ((bak_tur() == JetonTur_J_METIN))
        {
            long long n = dugum_yeni(DugumTur_D_SABIT, satir, sutun);
            dugum_yaz_ad(n, bak_deger());
            dugum_yaz_ptrd(n, 2LL);
            j_ilerle();
            return n;
        }
        if ((bak_tur() == JetonTur_J_ANAHTAR))
        {
            if (metin_esit(bak_deger(), "dogru"))
            {
                j_ilerle();
                long long n = dugum_yeni(DugumTur_D_SABIT, satir, sutun);
                dugum_yaz_ad(n, "mantik");
                dugum_yaz_sayi(n, 1LL);
                dugum_yaz_ptrd(n, 3LL);
                return n;
            }
            if (metin_esit(bak_deger(), "yanlis"))
            {
                j_ilerle();
                long long n = dugum_yeni(DugumTur_D_SABIT, satir, sutun);
                dugum_yaz_ad(n, "mantik");
                dugum_yaz_sayi(n, 0LL);
                dugum_yaz_ptrd(n, 3LL);
                return n;
            }
            if (metin_esit(bak_deger(), "yok"))
            {
                j_ilerle();
                long long n = dugum_yeni(DugumTur_D_SABIT, satir, sutun);
                dugum_yaz_ad(n, "yok");
                dugum_yaz_ptrd(n, 4LL);
                return n;
            }
            if (metin_esit(bak_deger(), "kendi"))
            {
                j_ilerle();
                return kimlik_dugum(satir, sutun, "kendi");
            }
            if (metin_esit(bak_deger(), "arkaplan"))
            {
                j_ilerle();
                long long sl = loop_al();
                long long ss = switch_al();
                loop_yaz(0LL);
                switch_yaz(0LL);
                long long body = ayristir_blok();
                loop_yaz(sl);
                switch_yaz(ss);
                long long n = dugum_yeni(DugumTur_D_ARKAPLAN_IFADE, satir, sutun);
                dugum_yaz_a(n, body);
                return n;
            }
            hata_fmt_basla();
            hata_fmt_yaz("'");
            hata_fmt_yaz(bak_deger());
            hata_fmt_yaz("' burada ifade olarak kullanılamaz.");
            hata_fmt_ver(satir, sutun);
            return 0LL;
        }
        if ((bak_tur() == JetonTur_J_KIMLIK))
        {
            char* ad = bak_deger();
            j_ilerle();
            return kimlik_dugum(satir, sutun, ad);
        }
        if ((bak_tur() == JetonTur_J_KOSE_AC))
        {
            return ayristir_dizi_sabit();
        }
        if (esles_tur(JetonTur_J_PARANTEZ_AC))
        {
            long long e = ayristir_ifade();
            beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
            return e;
        }
        hata_fmt_basla();
        hata_fmt_yaz("İfade bekleniyordu, '");
        hata_fmt_yaz(bak_deger());
        hata_fmt_yaz("' bulundu.");
        hata_fmt_ver(satir, sutun);
        return 0LL;
    }
}

long long ayristir_sonek(void) {
    {
        long long expr = ayristir_birincil();
        while ((!hata_var()))
        {
            if (esles_tur(JetonTur_J_PARANTEZ_AC))
            {
                long long n = dugum_yeni(DugumTur_D_CAGRI, dugum_satir(expr), dugum_sutun(expr));
                dugum_yaz_a(n, expr);
                long long L = liste_yeni();
                dugum_yaz_liste(n, L);
                if ((bak_tur() != JetonTur_J_PARANTEZ_KAPA))
                {
                    liste_ekle(L, ayristir_ifade());
                    while (esles_tur(JetonTur_J_VIRGUL))
                    {
                        liste_ekle(L, ayristir_ifade());
                    }
                }
                beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
                (expr = n);
            }
            else
                if (esles_tur(JetonTur_J_KOSE_AC))
                {
                    long long idx = ayristir_ifade();
                    beklenen_tur(JetonTur_J_KOSE_KAPA, "]");
                    long long n = dugum_yeni(DugumTur_D_INDIS, dugum_satir(expr), dugum_sutun(expr));
                    dugum_yaz_a(n, expr);
                    dugum_yaz_b(n, idx);
                    (expr = n);
                }
                else
                    if (esles_tur(JetonTur_J_NOKTA))
                    {
                        long long nm = beklenen_kimlik(true);
                        long long n = dugum_yeni(DugumTur_D_UYE, dugum_satir(expr), dugum_sutun(expr));
                        dugum_yaz_a(n, expr);
                        dugum_yaz_ad(n, jeton_deger(nm));
                        (expr = n);
                    }
                    else
                    {
                        break;
                    }
        }
        return expr;
    }
}

long long ayristir_tekil(void) {
    {
        long long t = bak_tur();
        if ((((((t == JetonTur_J_UNLEM) || (t == JetonTur_J_EKSI)) || (t == JetonTur_J_ARTI)) || (t == JetonTur_J_YILDIZ)) || (t == JetonTur_J_VE_ISARET)))
        {
            long long satir = bak_satir();
            long long sutun = bak_sutun();
            char* op = bak_deger();
            j_ilerle();
            long long n = dugum_yeni(DugumTur_D_TEKIL, satir, sutun);
            dugum_yaz_ad(n, op);
            dugum_yaz_a(n, ayristir_tekil());
            return n;
        }
        return ayristir_sonek();
    }
}

long long ayristir_carpan(void) {
    {
        long long left = ayristir_tekil();
        while ((!hata_var()))
        {
            if (((esles_tur(JetonTur_J_YILDIZ) || esles_tur(JetonTur_J_BOLU)) || esles_tur(JetonTur_J_MOD)))
            {
                long long opid = jeton_id_at((-1LL));
                long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
                dugum_yaz_ad(n, jeton_deger(opid));
                dugum_yaz_a(n, left);
                dugum_yaz_b(n, ayristir_tekil());
                (left = n);
            }
            else
            {
                break;
            }
        }
        return left;
    }
}

long long ayristir_terim(void) {
    {
        long long left = ayristir_carpan();
        while ((!hata_var()))
        {
            if ((esles_tur(JetonTur_J_ARTI) || esles_tur(JetonTur_J_EKSI)))
            {
                long long opid = jeton_id_at((-1LL));
                long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
                dugum_yaz_ad(n, jeton_deger(opid));
                dugum_yaz_a(n, left);
                dugum_yaz_b(n, ayristir_carpan());
                (left = n);
            }
            else
            {
                break;
            }
        }
        return left;
    }
}

long long ayristir_karsilastirma(void) {
    {
        long long left = ayristir_terim();
        while ((!hata_var()))
        {
            long long t = bak_tur();
            if (((((t == JetonTur_J_KUCUK) || (t == JetonTur_J_BUYUK)) || (t == JetonTur_J_KUCUK_ESIT)) || (t == JetonTur_J_BUYUK_ESIT)))
            {
                long long opid = j_ilerle();
                long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
                dugum_yaz_ad(n, jeton_deger(opid));
                dugum_yaz_a(n, left);
                dugum_yaz_b(n, ayristir_terim());
                (left = n);
            }
            else
            {
                break;
            }
        }
        return left;
    }
}

long long ayristir_esitlik(void) {
    {
        long long left = ayristir_karsilastirma();
        while ((!hata_var()))
        {
            if ((esles_tur(JetonTur_J_ESIT_ESIT) || esles_tur(JetonTur_J_ESIT_DEGIL)))
            {
                long long opid = jeton_id_at((-1LL));
                long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
                dugum_yaz_ad(n, jeton_deger(opid));
                dugum_yaz_a(n, left);
                dugum_yaz_b(n, ayristir_karsilastirma());
                (left = n);
            }
            else
            {
                break;
            }
        }
        return left;
    }
}

long long ayristir_bit_ve(void) {
    {
        long long left = ayristir_esitlik();
        while (((!hata_var()) && esles_tur(JetonTur_J_VE_ISARET)))
        {
            long long opid = jeton_id_at((-1LL));
            long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
            dugum_yaz_ad(n, "&");
            dugum_yaz_a(n, left);
            dugum_yaz_b(n, ayristir_esitlik());
            (left = n);
        }
        return left;
    }
}

long long ayristir_xor(void) {
    {
        long long left = ayristir_bit_ve();
        while (((!hata_var()) && esles_tur(JetonTur_J_XOR)))
        {
            long long opid = jeton_id_at((-1LL));
            long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
            dugum_yaz_ad(n, "^");
            dugum_yaz_a(n, left);
            dugum_yaz_b(n, ayristir_bit_ve());
            (left = n);
        }
        return left;
    }
}

long long ayristir_bit_veya(void) {
    {
        long long left = ayristir_xor();
        while (((!hata_var()) && esles_tur(JetonTur_J_VAY_ISARET)))
        {
            long long opid = jeton_id_at((-1LL));
            long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
            dugum_yaz_ad(n, "|");
            dugum_yaz_a(n, left);
            dugum_yaz_b(n, ayristir_xor());
            (left = n);
        }
        return left;
    }
}

long long ayristir_ve(void) {
    {
        long long left = ayristir_bit_veya();
        while (((!hata_var()) && esles_tur(JetonTur_J_VE)))
        {
            long long opid = jeton_id_at((-1LL));
            long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
            dugum_yaz_ad(n, "&&");
            dugum_yaz_a(n, left);
            dugum_yaz_b(n, ayristir_bit_veya());
            (left = n);
        }
        return left;
    }
}

long long ayristir_veya(void) {
    {
        long long left = ayristir_ve();
        while (((!hata_var()) && esles_tur(JetonTur_J_VEYA)))
        {
            long long opid = jeton_id_at((-1LL));
            long long n = dugum_yeni(DugumTur_D_IKIL, jeton_satir(opid), jeton_sutun(opid));
            dugum_yaz_ad(n, "||");
            dugum_yaz_a(n, left);
            dugum_yaz_b(n, ayristir_ve());
            (left = n);
        }
        return left;
    }
}

long long ayristir_uclu(void) {
    {
        long long expr = ayristir_veya();
        if (esles_tur(JetonTur_J_SORU))
        {
            long long thenb = ayristir_atama();
            beklenen_tur(JetonTur_J_IKI_NOKTA, ":");
            long long elseb = ayristir_uclu();
            long long n = dugum_yeni(DugumTur_D_UCLU, dugum_satir(expr), dugum_sutun(expr));
            dugum_yaz_a(n, expr);
            dugum_yaz_b(n, thenb);
            dugum_yaz_c(n, elseb);
            return n;
        }
        return expr;
    }
}

long long ayristir_atama(void) {
    {
        long long expr = ayristir_uclu();
        char* op = upp_metin_kopya("");
        if (esles_tur(JetonTur_J_ATAMA))
        {
            (op = "=");
        }
        else
            if (esles_tur(JetonTur_J_ARTI_ATAMA))
            {
                (op = "+=");
            }
            else
                if (esles_tur(JetonTur_J_EKSI_ATAMA))
                {
                    (op = "-=");
                }
                else
                    if (esles_tur(JetonTur_J_YILDIZ_ATAMA))
                    {
                        (op = "*=");
                    }
                    else
                        if (esles_tur(JetonTur_J_BOLU_ATAMA))
                        {
                            (op = "/=");
                        }
                        else
                            if (esles_tur(JetonTur_J_MOD_ATAMA))
                            {
                                (op = "%=");
                            }
        if ((!metin_esit(op, "")))
        {
            long long n = dugum_yeni(DugumTur_D_ATAMA, dugum_satir(expr), dugum_sutun(expr));
            dugum_yaz_ad(n, op);
            dugum_yaz_a(n, expr);
            dugum_yaz_b(n, ayristir_atama());
            return n;
        }
        return expr;
    }
}

long long ayristir_ifade(void) {
    {
        return ayristir_atama();
    }
}

long long ayristir_param(void) {
    {
        long long satir = bak_satir();
        long long sutun = bak_sutun();
        if (tur_baslangici_mi())
        {
            long long typ = ayristir_tur();
            long long nm = beklenen_kimlik(false);
            long long n = dugum_yeni(DugumTur_D_PARAM, jeton_satir(nm), jeton_sutun(nm));
            dugum_yaz_ad(n, jeton_deger(nm));
            dugum_yaz_a(n, typ);
            if (esles_tur(JetonTur_J_ATAMA))
            {
                dugum_yaz_c(n, ayristir_ifade());
            }
            return n;
        }
        long long nm = beklenen_kimlik(false);
        long long n = dugum_yeni(DugumTur_D_PARAM, jeton_satir(nm), jeton_sutun(nm));
        dugum_yaz_ad(n, jeton_deger(nm));
        if (esles_tur(JetonTur_J_ATAMA))
        {
            dugum_yaz_c(n, ayristir_ifade());
        }
        return n;
    }
}

long long ayristir_alan(void) {
    {
        long long typ = ayristir_tur();
        long long nm = beklenen_kimlik(false);
        long long n = dugum_yeni(DugumTur_D_ALAN, jeton_satir(nm), jeton_sutun(nm));
        dugum_yaz_ad(n, jeton_deger(nm));
        dugum_yaz_a(n, typ);
        if (esles_tur(JetonTur_J_KOSE_AC))
        {
            if ((bak_tur() == JetonTur_J_KOSE_KAPA))
            {
                dugum_yaz_bayrak(n, 8LL);
            }
            else
            {
                dugum_yaz_b(n, ayristir_ifade());
            }
            beklenen_tur(JetonTur_J_KOSE_KAPA, "]");
        }
        if (esles_tur(JetonTur_J_ATAMA))
        {
            hata_ver(jeton_satir(nm), jeton_sutun(nm), "Sınıf alanları başlatıcı alamaz. Değer atamasını 'olustur' metodunda yapın.");
        }
        beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
        return n;
    }
}

long long ayristir_oto(void) {
    {
        long long kw = beklenen_kw("oto");
        long long nm = beklenen_kimlik(false);
        if ((!esles_tur(JetonTur_J_ATAMA)))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'oto ");
            hata_fmt_yaz(jeton_deger(nm));
            hata_fmt_yaz("' bir başlatıcı gerektirir.");
            hata_fmt_ver(jeton_satir(nm), jeton_sutun(nm));
            return 0LL;
        }
        long long init = ayristir_baslatici();
        beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
        long long n = dugum_yeni(DugumTur_D_DEGISKEN, jeton_satir(nm), jeton_sutun(nm));
        dugum_yaz_ad(n, jeton_deger(nm));
        dugum_yaz_a(n, tur_dugum(jeton_satir(kw), jeton_sutun(kw), "", 0LL, 0LL));
        dugum_yaz_c(n, init);
        dugum_yaz_bayrak(n, 1LL);
        return n;
    }
}

long long ayristir_degisken_bayrak(long long sabit_mi) {
    {
        long long typ = ayristir_tur();
        long long nm = beklenen_kimlik(false);
        long long n = dugum_yeni(DugumTur_D_DEGISKEN, jeton_satir(nm), jeton_sutun(nm));
        dugum_yaz_ad(n, jeton_deger(nm));
        dugum_yaz_a(n, typ);
        if (esles_tur(JetonTur_J_KOSE_AC))
        {
            if ((sabit_mi != 0LL))
            {
                hata_ver(jeton_satir(nm), jeton_sutun(nm), "'sabit' dizi olamaz; skaler bir tür kullanın.");
            }
            if ((bak_tur() == JetonTur_J_KOSE_KAPA))
            {
                dugum_yaz_bayrak(n, 8LL);
            }
            else
            {
                dugum_yaz_b(n, ayristir_ifade());
            }
            beklenen_tur(JetonTur_J_KOSE_KAPA, "]");
        }
        if (esles_tur(JetonTur_J_ATAMA))
        {
            dugum_yaz_c(n, ayristir_baslatici());
        }
        if ((sabit_mi != 0LL))
        {
            dugum_yaz_bayrak(n, (dugum_bayrak(n) | 16LL));
            if ((dugum_c(n) == 0LL))
            {
                hata_fmt_basla();
                hata_fmt_yaz("'sabit ");
                hata_fmt_yaz(jeton_deger(nm));
                hata_fmt_yaz("' bir başlatıcı gerektirir.");
                hata_fmt_ver(jeton_satir(nm), jeton_sutun(nm));
            }
        }
        if ((((dugum_bayrak(n) & 8LL) != 0LL) && (dugum_c(n) == 0LL)))
        {
            hata_fmt_basla();
            hata_fmt_yaz("Boyutsuz dizi '");
            hata_fmt_yaz(jeton_deger(nm));
            hata_fmt_yaz("' bir başlatıcı gerektirir.");
            hata_fmt_ver(jeton_satir(nm), jeton_sutun(nm));
        }
        beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
        return n;
    }
}

long long ayristir_degisken(void) {
    {
        return ayristir_degisken_bayrak(0LL);
    }
}

long long ayristir_sabit(void) {
    {
        beklenen_kw("sabit");
        return ayristir_degisken_bayrak(1LL);
    }
}

long long ayristir_blok(void) {
    {
        long long tok = beklenen_tur(JetonTur_J_SLUS_AC, "{");
        long long n = dugum_yeni(DugumTur_D_BLOK, jeton_satir(tok), jeton_sutun(tok));
        long long L = liste_yeni();
        dugum_yaz_liste(n, L);
        while ((((bak_tur() != JetonTur_J_SLUS_KAPA) && (bak_tur() != JetonTur_J_EOF)) && (!hata_var())))
        {
            liste_ekle(L, ayristir_cumle());
        }
        beklenen_tur(JetonTur_J_SLUS_KAPA, "}");
        return n;
    }
}

long long ayristir_eger(void) {
    {
        long long kw = beklenen_kw("eger");
        beklenen_tur(JetonTur_J_PARANTEZ_AC, "(");
        long long cond = ayristir_ifade();
        beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
        long long thenb = ayristir_cumle();
        long long elseb = 0LL;
        if (esles_kw("yoksa"))
        {
            (elseb = ayristir_cumle());
        }
        long long n = dugum_yeni(DugumTur_D_EGER, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_a(n, cond);
        dugum_yaz_b(n, thenb);
        dugum_yaz_c(n, elseb);
        return n;
    }
}

long long ayristir_iken(void) {
    {
        long long kw = beklenen_kw("iken");
        beklenen_tur(JetonTur_J_PARANTEZ_AC, "(");
        long long cond = ayristir_ifade();
        beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
        loop_yaz((loop_al() + 1LL));
        long long body = ayristir_cumle();
        loop_yaz((loop_al() - 1LL));
        long long n = dugum_yeni(DugumTur_D_IKEN, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_a(n, cond);
        dugum_yaz_b(n, body);
        return n;
    }
}

long long ayristir_dongu(void) {
    {
        long long kw = beklenen_kw("dongu");
        beklenen_tur(JetonTur_J_PARANTEZ_AC, "(");
        long long init = 0LL;
        if (esles_tur(JetonTur_J_NOKTALI_VIRGUL))
        {
            (init = 0LL);
        }
        else
            if (kw_mi("oto"))
            {
                (init = ayristir_oto());
            }
            else
                if (tur_baslangici_mi())
                {
                    (init = ayristir_degisken());
                }
                else
                {
                    long long e = ayristir_ifade();
                    beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
                    (init = dugum_yeni(DugumTur_D_IFADE_CUMLE, dugum_satir(e), dugum_sutun(e)));
                    dugum_yaz_a(init, e);
                }
        long long cond = 0LL;
        if ((bak_tur() != JetonTur_J_NOKTALI_VIRGUL))
        {
            (cond = ayristir_ifade());
        }
        beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
        long long step = 0LL;
        if ((bak_tur() != JetonTur_J_PARANTEZ_KAPA))
        {
            (step = ayristir_ifade());
        }
        beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
        loop_yaz((loop_al() + 1LL));
        long long body = ayristir_cumle();
        loop_yaz((loop_al() - 1LL));
        long long n = dugum_yeni(DugumTur_D_DONGU, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_a(n, init);
        dugum_yaz_b(n, cond);
        dugum_yaz_c(n, step);
        dugum_yaz_d(n, body);
        return n;
    }
}

long long ayristir_secim_kol(void) {
    {
        long long L = liste_yeni();
        while ((((((bak_tur() != JetonTur_J_SLUS_KAPA) && (bak_tur() != JetonTur_J_EOF)) && (!kw_mi("durum"))) && (!kw_mi("varsayilan"))) && (!hata_var())))
        {
            liste_ekle(L, ayristir_cumle());
        }
        return L;
    }
}

long long ayristir_secim(void) {
    {
        long long kw = beklenen_kw("secim");
        beklenen_tur(JetonTur_J_PARANTEZ_AC, "(");
        long long expr = ayristir_ifade();
        beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
        beklenen_tur(JetonTur_J_SLUS_AC, "{");
        long long L = liste_yeni();
        bool gordu_varsayilan = false;
        switch_yaz((switch_al() + 1LL));
        while ((((bak_tur() != JetonTur_J_SLUS_KAPA) && (bak_tur() != JetonTur_J_EOF)) && (!hata_var())))
        {
            if (kw_mi("durum"))
            {
                long long et = j_ilerle();
                long long val = ayristir_ifade();
                if ((bak_tur() != JetonTur_J_IKI_NOKTA))
                {
                    hata_ver(jeton_satir(et), bak_sutun(), "':' bekleniyordu.");
                }
                else
                {
                    j_ilerle();
                }
                long long c = dugum_yeni(DugumTur_D_DURUM, jeton_satir(et), jeton_sutun(et));
                dugum_yaz_a(c, val);
                dugum_yaz_liste(c, ayristir_secim_kol());
                liste_ekle(L, c);
            }
            else
                if (kw_mi("varsayilan"))
                {
                    long long et = j_ilerle();
                    if (gordu_varsayilan)
                    {
                        hata_ver(jeton_satir(et), jeton_sutun(et), "'varsayilan' bir 'secim' içinde bir kez yazılabilir.");
                    }
                    (gordu_varsayilan = true);
                    if ((bak_tur() != JetonTur_J_IKI_NOKTA))
                    {
                        hata_ver(jeton_satir(et), bak_sutun(), "':' bekleniyordu.");
                    }
                    else
                    {
                        j_ilerle();
                    }
                    long long c = dugum_yeni(DugumTur_D_DURUM, jeton_satir(et), jeton_sutun(et));
                    dugum_yaz_liste(c, ayristir_secim_kol());
                    liste_ekle(L, c);
                }
                else
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("'durum' veya 'varsayilan' bekleniyordu, '");
                    hata_fmt_yaz(bak_deger());
                    hata_fmt_yaz("' bulundu.");
                    hata_fmt_ver(bak_satir(), bak_sutun());
                    break;
                }
        }
        switch_yaz((switch_al() - 1LL));
        beklenen_tur(JetonTur_J_SLUS_KAPA, "}");
        long long n = dugum_yeni(DugumTur_D_SECIM, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_a(n, expr);
        dugum_yaz_liste(n, L);
        return n;
    }
}

long long ayristir_her(void) {
    {
        long long kw = beklenen_kw("her");
        beklenen_tur(JetonTur_J_PARANTEZ_AC, "(");
        long long inferred = 0LL;
        long long typ = 0LL;
        long long nm = 0LL;
        if (kw_mi("oto"))
        {
            j_ilerle();
            (inferred = 1LL);
            (typ = tur_dugum(jeton_satir(kw), jeton_sutun(kw), "", 0LL, 0LL));
            (nm = beklenen_kimlik(false));
        }
        else
        {
            (typ = ayristir_tur());
            (nm = beklenen_kimlik(false));
        }
        beklenen_kw("in");
        long long it = ayristir_ifade();
        beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
        loop_yaz((loop_al() + 1LL));
        long long body = ayristir_cumle();
        loop_yaz((loop_al() - 1LL));
        long long n = dugum_yeni(DugumTur_D_HER, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_ad(n, jeton_deger(nm));
        dugum_yaz_a(n, typ);
        dugum_yaz_b(n, it);
        dugum_yaz_c(n, body);
        if ((inferred != 0LL))
        {
            dugum_yaz_bayrak(n, 1LL);
        }
        return n;
    }
}

long long ayristir_cumle(void) {
    {
        if (hata_var())
        {
            return 0LL;
        }
        if (kw_mi("c_ekle"))
        {
            hata_ver(bak_satir(), bak_sutun(), "'c_ekle' yalnızca dosya başında (üst düzeyde) kullanılır. Fonksiyon içine #include koymayın.");
            return 0LL;
        }
        if (kw_mi("kullan"))
        {
            hata_ver(bak_satir(), bak_sutun(), "'kullan' yalnızca dosya başında (üst düzeyde) kullanılır.");
            return 0LL;
        }
        if (kw_mi("sabit"))
        {
            return ayristir_sabit();
        }
        if (kw_mi("eger"))
        {
            return ayristir_eger();
        }
        if (kw_mi("secim"))
        {
            return ayristir_secim();
        }
        if (kw_mi("her"))
        {
            return ayristir_her();
        }
        if (kw_mi("iken"))
        {
            return ayristir_iken();
        }
        if (kw_mi("dongu"))
        {
            return ayristir_dongu();
        }
        if (kw_mi("don"))
        {
            long long kw = j_ilerle();
            long long val = 0LL;
            if ((bak_tur() != JetonTur_J_NOKTALI_VIRGUL))
            {
                (val = ayristir_ifade());
            }
            beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
            long long n = dugum_yeni(DugumTur_D_DON, jeton_satir(kw), jeton_sutun(kw));
            dugum_yaz_a(n, val);
            return n;
        }
        if (kw_mi("dur"))
        {
            long long kw = j_ilerle();
            if (((loop_al() <= 0LL) && (switch_al() <= 0LL)))
            {
                hata_ver(jeton_satir(kw), jeton_sutun(kw), "'dur' yalnızca 'iken', 'dongu', 'her' veya 'secim' içinde kullanılabilir.");
            }
            beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
            return dugum_yeni(DugumTur_D_DUR, jeton_satir(kw), jeton_sutun(kw));
        }
        if (kw_mi("devam"))
        {
            long long kw = j_ilerle();
            if ((loop_al() <= 0LL))
            {
                hata_ver(jeton_satir(kw), jeton_sutun(kw), "'devam' yalnızca 'iken', 'dongu' veya 'her' içinde kullanılabilir.");
            }
            beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
            return dugum_yeni(DugumTur_D_DEVAM, jeton_satir(kw), jeton_sutun(kw));
        }
        if (kw_mi("guvensiz"))
        {
            long long kw = j_ilerle();
            long long body = ayristir_blok();
            long long n = dugum_yeni(DugumTur_D_GUVENSIZ, jeton_satir(kw), jeton_sutun(kw));
            dugum_yaz_a(n, body);
            return n;
        }
        if (kw_mi("arkaplan"))
        {
            long long kw = j_ilerle();
            long long sl = loop_al();
            long long ss = switch_al();
            loop_yaz(0LL);
            switch_yaz(0LL);
            long long body = ayristir_blok();
            loop_yaz(sl);
            switch_yaz(ss);
            long long n = dugum_yeni(DugumTur_D_ARKAPLAN, jeton_satir(kw), jeton_sutun(kw));
            dugum_yaz_a(n, body);
            return n;
        }
        if (kw_mi("guvenli"))
        {
            long long kw = j_ilerle();
            long long body = ayristir_blok();
            long long n = dugum_yeni(DugumTur_D_GUVENLI, jeton_satir(kw), jeton_sutun(kw));
            dugum_yaz_a(n, body);
            return n;
        }
        if (kw_mi("oto"))
        {
            return ayristir_oto();
        }
        if ((bak_tur() == JetonTur_J_SLUS_AC))
        {
            return ayristir_blok();
        }
        if ((bak_tur() == JetonTur_J_C_GOMMU))
        {
            long long tok = j_ilerle();
            long long n = dugum_yeni(DugumTur_D_C_GOMMU, jeton_satir(tok), jeton_sutun(tok));
            dugum_yaz_ad(n, jeton_deger(tok));
            return n;
        }
        if ((bak_tur() == JetonTur_J_ASM_GOMMU))
        {
            long long tok = j_ilerle();
            long long n = dugum_yeni(DugumTur_D_ASM_GOMMU, jeton_satir(tok), jeton_sutun(tok));
            dugum_yaz_ad(n, jeton_deger(tok));
            return n;
        }
        if (tur_baslangici_mi())
        {
            return ayristir_degisken();
        }
        if (esles_tur(JetonTur_J_NOKTALI_VIRGUL))
        {
            long long tok = jeton_id_at((-1LL));
            return dugum_yeni(DugumTur_D_BOS_CUMLE, jeton_satir(tok), jeton_sutun(tok));
        }
        long long e = ayristir_ifade();
        beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
        long long n = dugum_yeni(DugumTur_D_IFADE_CUMLE, dugum_satir(e), dugum_sutun(e));
        dugum_yaz_a(n, e);
        return n;
    }
}

long long ayristir_fonksiyon(char* sinif_ad) {
    {
        long long kw = beklenen_kw("fonk");
        long long nm = beklenen_kimlik(false);
        if ((metin_esit(jeton_deger(nm), "upp") && metin_esit(sinif_ad, "")))
        {
            hata_ver(jeton_satir(nm), jeton_sutun(nm), "'upp' yerleşik ad alanıdır; fonksiyon adı olamaz.");
        }
        beklenen_tur(JetonTur_J_PARANTEZ_AC, "(");
        long long params = liste_yeni();
        if ((bak_tur() != JetonTur_J_PARANTEZ_KAPA))
        {
            liste_ekle(params, ayristir_param());
            while (esles_tur(JetonTur_J_VIRGUL))
            {
                liste_ekle(params, ayristir_param());
            }
        }
        beklenen_tur(JetonTur_J_PARANTEZ_KAPA, ")");
        long long saw_def = 0LL;
        long long pi = 0LL;
        while ((pi < liste_uzunluk(params)))
        {
            long long p = liste_al(params, pi);
            if ((dugum_c(p) != 0LL))
            {
                (saw_def = 1LL);
            }
            else
                if ((saw_def != 0LL))
                {
                    hata_ver(dugum_satir(p), dugum_sutun(p), "Varsayılan parametreler sonda art arda olmalıdır.");
                }
            (pi = (pi + 1LL));
        }
        long long ret = tur_dugum(jeton_satir(kw), jeton_sutun(kw), "bos", 0LL, 0LL);
        long long ret_ex = 0LL;
        if (esles_tur(JetonTur_J_OK))
        {
            (ret = ayristir_tur());
            (ret_ex = 1LL);
        }
        long long body = 0LL;
        long long arrow = 0LL;
        if (esles_tur(JetonTur_J_YAG_OK))
        {
            long long expr = ayristir_ifade();
            beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
            (body = dugum_yeni(DugumTur_D_BLOK, dugum_satir(expr), dugum_sutun(expr)));
            long long L = liste_yeni();
            dugum_yaz_liste(body, L);
            long long rs = dugum_yeni(DugumTur_D_DON, dugum_satir(expr), dugum_sutun(expr));
            dugum_yaz_a(rs, expr);
            liste_ekle(L, rs);
            (arrow = 1LL);
        }
        else
        {
            (body = ayristir_blok());
        }
        long long n = dugum_yeni(DugumTur_D_FONK, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_ad(n, jeton_deger(nm));
        dugum_yaz_a(n, ret);
        dugum_yaz_b(n, body);
        dugum_yaz_liste(n, params);
        if ((!metin_esit(sinif_ad, "")))
        {
            dugum_yaz_c(n, kimlik_dugum(jeton_satir(nm), jeton_sutun(nm), sinif_ad));
        }
        long long fl = 0LL;
        if ((arrow != 0LL))
        {
            (fl = (fl + 2LL));
        }
        if ((ret_ex != 0LL))
        {
            (fl = (fl + 4LL));
        }
        dugum_yaz_bayrak(n, fl);
        return n;
    }
}

bool sinif_uye_var(long long fields, long long methods, char* ad) {
    {
        long long i = 0LL;
        while ((i < liste_uzunluk(fields)))
        {
            if (metin_esit(dugum_ad(liste_al(fields, i)), ad))
            {
                return true;
            }
            (i = (i + 1LL));
        }
        (i = 0LL);
        while ((i < liste_uzunluk(methods)))
        {
            if (metin_esit(dugum_ad(liste_al(methods, i)), ad))
            {
                return true;
            }
            (i = (i + 1LL));
        }
        return false;
    }
}

long long ayristir_sinif(void) {
    {
        long long kw = beklenen_kw("sinif");
        long long nm = beklenen_kimlik(false);
        if (metin_esit(jeton_deger(nm), "upp"))
        {
            hata_ver(jeton_satir(nm), jeton_sutun(nm), "'upp' yerleşik ad alanıdır; sınıf adı olamaz.");
        }
        beklenen_tur(JetonTur_J_SLUS_AC, "{");
        long long fields = liste_yeni();
        long long methods = liste_yeni();
        long long constants = liste_yeni();
        char* sinif_ad = jeton_deger(nm);
        while ((((bak_tur() != JetonTur_J_SLUS_KAPA) && (bak_tur() != JetonTur_J_EOF)) && (!hata_var())))
        {
            if (kw_mi("fonk"))
            {
                long long m = ayristir_fonksiyon(sinif_ad);
                if (sinif_uye_var(fields, methods, dugum_ad(m)))
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("'");
                    hata_fmt_yaz(sinif_ad);
                    hata_fmt_yaz("' içinde '");
                    hata_fmt_yaz(dugum_ad(m));
                    hata_fmt_yaz("' birden fazla kez tanımlandı.");
                    hata_fmt_ver(dugum_satir(m), dugum_sutun(m));
                }
                else
                {
                    liste_ekle(methods, m);
                }
            }
            else
                if (kw_mi("sabit"))
                {
                    long long c = ayristir_sabit();
                    if (sinif_uye_var(fields, methods, dugum_ad(c)))
                    {
                        hata_fmt_basla();
                        hata_fmt_yaz("'");
                        hata_fmt_yaz(sinif_ad);
                        hata_fmt_yaz("' içinde '");
                        hata_fmt_yaz(dugum_ad(c));
                        hata_fmt_yaz("' birden fazla kez tanımlandı.");
                        hata_fmt_ver(dugum_satir(c), dugum_sutun(c));
                    }
                    else
                    {
                        long long ci = 0LL;
                        while ((ci < liste_uzunluk(constants)))
                        {
                            if (metin_esit(dugum_ad(liste_al(constants, ci)), dugum_ad(c)))
                            {
                                hata_ver(dugum_satir(c), dugum_sutun(c), "Sınıf sabiti birden fazla kez tanımlandı.");
                            }
                            (ci = (ci + 1LL));
                        }
                        liste_ekle(constants, c);
                    }
                }
                else
                    if (tur_baslangici_mi())
                    {
                        long long f = ayristir_alan();
                        if (metin_esit(dugum_ad(f), "kendi"))
                        {
                            hata_ver(dugum_satir(f), dugum_sutun(f), "'kendi' alan adı olarak kullanılamaz.");
                        }
                        if (sinif_uye_var(fields, methods, dugum_ad(f)))
                        {
                            hata_fmt_basla();
                            hata_fmt_yaz("'");
                            hata_fmt_yaz(sinif_ad);
                            hata_fmt_yaz("' içinde '");
                            hata_fmt_yaz(dugum_ad(f));
                            hata_fmt_yaz("' birden fazla kez tanımlandı.");
                            hata_fmt_ver(dugum_satir(f), dugum_sutun(f));
                        }
                        else
                        {
                            liste_ekle(fields, f);
                        }
                    }
                    else
                    {
                        hata_fmt_basla();
                        hata_fmt_yaz("Sınıf gövdesinde alan, 'sabit' veya 'fonk' bekleniyordu, '");
                        hata_fmt_yaz(bak_deger());
                        hata_fmt_yaz("' bulundu.");
                        hata_fmt_ver(bak_satir(), bak_sutun());
                        break;
                    }
        }
        beklenen_tur(JetonTur_J_SLUS_KAPA, "}");
        long long n = dugum_yeni(DugumTur_D_SINIF, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_ad(n, jeton_deger(nm));
        dugum_yaz_liste(n, fields);
        dugum_yaz_liste2(n, methods);
        if ((liste_uzunluk(constants) > 0LL))
        {
            long long holder = dugum_yeni(DugumTur_D_BLOK, jeton_satir(kw), jeton_sutun(kw));
            dugum_yaz_liste(holder, constants);
            dugum_yaz_c(n, holder);
        }
        return n;
    }
}

long long ayristir_secenek(void) {
    {
        long long kw = beklenen_kw("secenek");
        long long nm = beklenen_kimlik(false);
        char* ad = jeton_deger(nm);
        if ((((tur_adi_mi(ad) || metin_esit(ad, "upp")) || metin_esit(ad, "ana")) || metin_esit(ad, "main")))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'");
            hata_fmt_yaz(ad);
            hata_fmt_yaz("' seçenek adı olarak kullanılamaz.");
            hata_fmt_ver(jeton_satir(nm), jeton_sutun(nm));
        }
        beklenen_tur(JetonTur_J_SLUS_AC, "{");
        long long L = liste_yeni();
        long long next_val = 0LL;
        while ((((bak_tur() != JetonTur_J_SLUS_KAPA) && (bak_tur() != JetonTur_J_EOF)) && (!hata_var())))
        {
            long long ident = beklenen_kimlik(false);
            long long value = next_val;
            if (esles_tur(JetonTur_J_ATAMA))
            {
                long long num = beklenen_tur(JetonTur_J_SAYI, "tam sayı");
                (value = hex_deger(jeton_deger(num)));
            }
            long long v = dugum_yeni(DugumTur_D_SECENEK_UYE, jeton_satir(ident), jeton_sutun(ident));
            dugum_yaz_ad(v, jeton_deger(ident));
            dugum_yaz_sayi(v, value);
            liste_ekle(L, v);
            (next_val = (value + 1LL));
            if ((!esles_tur(JetonTur_J_VIRGUL)))
            {
                break;
            }
        }
        beklenen_tur(JetonTur_J_SLUS_KAPA, "}");
        if ((liste_uzunluk(L) == 0LL))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'");
            hata_fmt_yaz(ad);
            hata_fmt_yaz("' en az bir seçenek değeri içermelidir.");
            hata_fmt_ver(jeton_satir(nm), jeton_sutun(nm));
        }
        long long n = dugum_yeni(DugumTur_D_SECENEK, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_ad(n, ad);
        dugum_yaz_liste(n, L);
        return n;
    }
}

long long ayristir_kullan(void) {
    {
        long long kw = beklenen_kw("kullan");
        long long spec = beklenen_tur(JetonTur_J_METIN, "kütüphane yolu");
        long long alias = 0LL;
        if (kw_mi("olarak"))
        {
            j_ilerle();
            long long ad = beklenen_kimlik(false);
            (alias = kimlik_dugum(jeton_satir(ad), jeton_sutun(ad), jeton_deger(ad)));
        }
        beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
        char* inner = metin_coz(jeton_deger(spec));
        if (metin_esit(inner, ""))
        {
            hata_ver(jeton_satir(kw), jeton_sutun(kw), "'kullan' boş bir yol alamaz.");
        }
        long long n = dugum_yeni(DugumTur_D_KULLAN, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_ad(n, inner);
        if ((alias != 0LL))
        {
            dugum_yaz_b(n, alias);
        }
        return n;
    }
}

long long ayristir_include(void) {
    {
        long long kw = beklenen_kw("c_ekle");
        long long spec = beklenen_tur(JetonTur_J_METIN, "başlık dizgesi");
        beklenen_tur(JetonTur_J_NOKTALI_VIRGUL, ";");
        char* inner = metin_coz(jeton_deger(spec));
        if (metin_esit(inner, ""))
        {
            hata_ver(jeton_satir(kw), jeton_sutun(kw), "c_ekle boş bir başlık alamaz.");
        }
        long long n = dugum_yeni(DugumTur_D_CEKLE, jeton_satir(kw), jeton_sutun(kw));
        dugum_yaz_ad(n, inner);
        return n;
    }
}

long long ayristir_birim(void) {
    {
        long long first_satir = bak_satir();
        long long first_sutun = bak_sutun();
        long long decls = liste_yeni();
        while (((bak_tur() != JetonTur_J_EOF) && (!hata_var())))
        {
            if (kw_mi("c_ekle"))
            {
                liste_ekle(decls, ayristir_include());
            }
            else
                if (kw_mi("kullan"))
                {
                    liste_ekle(decls, ayristir_kullan());
                }
                else
                    if (kw_mi("sabit"))
                    {
                        liste_ekle(decls, ayristir_sabit());
                    }
                    else
                        if (kw_mi("sinif"))
                        {
                            liste_ekle(decls, ayristir_sinif());
                        }
                        else
                            if (kw_mi("secenek"))
                            {
                                liste_ekle(decls, ayristir_secenek());
                            }
                            else
                                if (kw_mi("fonk"))
                                {
                                    liste_ekle(decls, ayristir_fonksiyon(""));
                                }
                                else
                                    if ((kw_mi("oto") || tur_baslangici_mi()))
                                    {
                                        if (kw_mi("oto"))
                                        {
                                            liste_ekle(decls, ayristir_oto());
                                        }
                                        else
                                        {
                                            liste_ekle(decls, ayristir_degisken());
                                        }
                                    }
                                    else
                                    {
                                        hata_fmt_basla();
                                        hata_fmt_yaz("Üst düzeyde 'c_ekle', 'kullan', 'sabit', değişken, 'sinif', 'secenek' veya 'fonk' bekleniyordu, '");
                                        hata_fmt_yaz(bak_deger());
                                        hata_fmt_yaz("' bulundu.");
                                        hata_fmt_ver(bak_satir(), bak_sutun());
                                        break;
                                    }
        }
        if (hata_var())
        {
            return 0LL;
        }
        long long n = dugum_yeni(DugumTur_D_PROGRAM, first_satir, first_sutun);
        dugum_yaz_liste(n, decls);
        return n;
    }
}

long long ayristir_program(void) {
    {
        long long birim = ayristir_birim();
        if (((birim == 0LL) || hata_var()))
        {
            return 0LL;
        }
        long long decls = dugum_liste(birim);
        long long ana_say = 0LL;
        long long ana_ikinci = 0LL;
        long long i = 0LL;
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if (((dugum_tur(d) == DugumTur_D_FONK) && metin_esit(dugum_ad(d), "ana")))
            {
                (ana_say = (ana_say + 1LL));
                if ((ana_say > 1LL))
                {
                    (ana_ikinci = d);
                }
            }
            (i = (i + 1LL));
        }
        if ((ana_say == 0LL))
        {
            hata_ver(dugum_satir(birim), dugum_sutun(birim), "Giriş noktası bulunamadı. Bir 'fonk ana()' tanımlayın.");
            return 0LL;
        }
        if ((ana_say > 1LL))
        {
            hata_ver(dugum_satir(ana_ikinci), dugum_sutun(ana_ikinci), "Birden fazla 'fonk ana()' tanımlanamaz; giriş noktası tek olmalıdır.");
            return 0LL;
        }
        return birim;
    }
}

long long sabit_tur_kodu(long long n) {
    {
        if ((n == 0LL))
        {
            return 0LL;
        }
        if (metin_esit(dugum_ad(n), "mantik"))
        {
            return 3LL;
        }
        if (metin_esit(dugum_ad(n), "yok"))
        {
            return 4LL;
        }
        return dugum_sayi(n);
    }
}

long long tip_bos(long long satir, long long sutun) {
    {
        return tur_dugum(satir, sutun, "bos", 0LL, 0LL);
    }
}

long long tip_sayi(long long satir, long long sutun) {
    {
        return tur_dugum(satir, sutun, "sayi", 0LL, 0LL);
    }
}

long long tip_bayt(long long satir, long long sutun) {
    {
        return tur_dugum(satir, sutun, "bayt", 0LL, 0LL);
    }
}

long long tip_ondalik(long long satir, long long sutun) {
    {
        return tur_dugum(satir, sutun, "ondalik", 0LL, 0LL);
    }
}

long long tip_metin(long long satir, long long sutun) {
    {
        return tur_dugum(satir, sutun, "metin", 0LL, 0LL);
    }
}

long long tip_mantik(long long satir, long long sutun) {
    {
        return tur_dugum(satir, sutun, "mantik", 0LL, 0LL);
    }
}

void cift_tanim_ver(long long eski, long long yeni, char* ad) {
    {
        hata_fmt_basla();
        hata_fmt_yaz("'");
        hata_fmt_yaz(ad);
        hata_fmt_yaz("' birden fazla kez tanımlandı.");
        char* ey = dugum_kaynak(eski);
        char* ny = dugum_kaynak(yeni);
        if (((!metin_esit(ey, "")) || (!metin_esit(ny, ""))))
        {
            hata_fmt_yaz(" (");
            if (metin_esit(ey, ""))
            {
                hata_fmt_yaz("?");
            }
            else
            {
                hata_fmt_yaz(ey);
            }
            hata_fmt_yaz(" ve ");
            if (metin_esit(ny, ""))
            {
                hata_fmt_yaz("?");
            }
            else
            {
                hata_fmt_yaz(ny);
            }
            hata_fmt_yaz(")");
        }
        hata_fmt_ver(dugum_satir(yeni), dugum_sutun(yeni));
    }
}

bool tip_void_mi(long long t) {
    {
        return ((((t != 0LL) && metin_esit(dugum_ad(t), "bos")) && (dugum_ptrd(t) == 0LL)) && (dugum_dims(t) == 0LL));
    }
}

bool tip_yok_mi(long long t) {
    {
        return ((t != 0LL) && metin_esit(dugum_ad(t), "yok"));
    }
}

bool tip_metin_mi(long long t) {
    {
        return (((t != 0LL) && metin_esit(dugum_ad(t), "metin")) && (dugum_ptrd(t) == 0LL));
    }
}

bool tip_tam_sayi_mi(long long t) {
    {
        return ((((t != 0LL) && (metin_esit(dugum_ad(t), "sayi") || metin_esit(dugum_ad(t), "bayt"))) && (dugum_ptrd(t) == 0LL)) && (dugum_dims(t) == 0LL));
    }
}

void arg_tam_sayi_olmali(long long e) {
    {
        long long t = tip_ifade(e);
        if (hata_var())
        {
            return;
        }
        if ((!tip_tam_sayi_mi(t)))
        {
            hata_ver(dugum_satir(e), dugum_sutun(e), "Bu konumda 'sayi' bekleniyor (ondalik/mantik sessizce kesilmez).");
        }
    }
}

bool tip_sayisal_mi(long long t) {
    {
        if ((t == 0LL))
        {
            return false;
        }
        if (((dugum_ptrd(t) != 0LL) || (dugum_dims(t) != 0LL)))
        {
            return false;
        }
        char* n = dugum_ad(t);
        return (((metin_esit(n, "sayi") || metin_esit(n, "bayt")) || metin_esit(n, "ondalik")) || metin_esit(n, "mantik"));
    }
}

bool tip_sinif_deger_mi(long long t) {
    {
        if ((((t == 0LL) || (dugum_ptrd(t) != 0LL)) || (dugum_dims(t) != 0LL)))
        {
            return false;
        }
        return (tablo_bul(0LL, dugum_ad(t)) != 0LL);
    }
}

bool tip_enum_mi(long long t) {
    {
        if ((((t == 0LL) || (dugum_ptrd(t) != 0LL)) || (dugum_dims(t) != 0LL)))
        {
            return false;
        }
        return (tablo_bul(2LL, dugum_ad(t)) != 0LL);
    }
}

bool tip_num_genis(long long t) {
    {
        return (tip_sayisal_mi(t) || tip_enum_mi(t));
    }
}

bool tip_atanir(long long dst, long long src) {
    {
        if (((dst == 0LL) || (src == 0LL)))
        {
            return false;
        }
        if ((tip_void_mi(src) || tip_void_mi(dst)))
        {
            return false;
        }
        if (tip_yok_mi(src))
        {
            return (tip_metin_mi(dst) || (dugum_ptrd(dst) > 0LL));
        }
        if (((dugum_ptrd(dst) > 0LL) && (dugum_ptrd(src) > 0LL)))
        {
            return ((metin_esit(dugum_ad(src), dugum_ad(dst)) || metin_esit(dugum_ad(src), "bos")) || metin_esit(dugum_ad(dst), "bos"));
        }
        if ((dugum_ptrd(dst) != dugum_ptrd(src)))
        {
            return false;
        }
        if ((dugum_dims(dst) != dugum_dims(src)))
        {
            return false;
        }
        if (metin_esit(dugum_ad(dst), dugum_ad(src)))
        {
            return true;
        }
        if ((tip_enum_mi(dst) && (dugum_dims(src) == 0LL)))
        {
            char* sn = dugum_ad(src);
            return ((metin_esit(sn, dugum_ad(dst)) || metin_esit(sn, "sayi")) || metin_esit(sn, "mantik"));
        }
        if (((tip_enum_mi(src) && metin_esit(dugum_ad(dst), "sayi")) && (dugum_dims(dst) == 0LL)))
        {
            return true;
        }
        if ((metin_esit(dugum_ad(dst), "ondalik") && ((metin_esit(dugum_ad(src), "sayi") || metin_esit(dugum_ad(src), "bayt")) || metin_esit(dugum_ad(src), "mantik"))))
        {
            return true;
        }
        if ((metin_esit(dugum_ad(dst), "sayi") && (metin_esit(dugum_ad(src), "mantik") || metin_esit(dugum_ad(src), "bayt"))))
        {
            return true;
        }
        if ((metin_esit(dugum_ad(dst), "mantik") && (metin_esit(dugum_ad(src), "sayi") || metin_esit(dugum_ad(src), "bayt"))))
        {
            return true;
        }
        if ((metin_esit(dugum_ad(dst), "bayt") && (metin_esit(dugum_ad(src), "sayi") || metin_esit(dugum_ad(src), "mantik"))))
        {
            return true;
        }
        return false;
    }
}

bool lvalue_mi(long long e) {
    {
        if ((e == 0LL))
        {
            return false;
        }
        long long t = dugum_tur(e);
        if ((t == DugumTur_D_KIMLIK))
        {
            return ((!metin_esit(dugum_ad(e), "upp")) && (!tur_adi_mi(dugum_ad(e))));
        }
        if (((t == DugumTur_D_UYE) || (t == DugumTur_D_INDIS)))
        {
            return true;
        }
        if (((t == DugumTur_D_TEKIL) && metin_esit(dugum_ad(e), "*")))
        {
            return true;
        }
        return false;
    }
}

long long tip_yerlesik_sinif(char* ad, char* a, char* ta, char* b, char* tb, char* c, char* tc) {
    {
        long long n = dugum_yeni(DugumTur_D_SINIF, 0LL, 1LL);
        dugum_yaz_ad(n, ad);
        long long F = liste_yeni();
        dugum_yaz_liste(n, F);
        dugum_yaz_liste2(n, liste_yeni());
        if ((!metin_esit(a, "")))
        {
            long long f = dugum_yeni(DugumTur_D_ALAN, 0LL, 1LL);
            dugum_yaz_ad(f, a);
            dugum_yaz_a(f, tur_dugum(0LL, 1LL, ta, 0LL, 0LL));
            liste_ekle(F, f);
        }
        if ((!metin_esit(b, "")))
        {
            long long f = dugum_yeni(DugumTur_D_ALAN, 0LL, 1LL);
            dugum_yaz_ad(f, b);
            dugum_yaz_a(f, tur_dugum(0LL, 1LL, tb, 0LL, 0LL));
            liste_ekle(F, f);
        }
        if ((!metin_esit(c, "")))
        {
            long long f = dugum_yeni(DugumTur_D_ALAN, 0LL, 1LL);
            dugum_yaz_ad(f, c);
            dugum_yaz_a(f, tur_dugum(0LL, 1LL, tc, 0LL, 0LL));
            liste_ekle(F, f);
        }
        tablo_yaz(0LL, ad, n);
        return n;
    }
}

void tip_uyelik_yolu(long long e, long long i, long long L) {
    {
        if ((e == 0LL))
        {
            return;
        }
        if ((dugum_tur(e) == DugumTur_D_UYE))
        {
            tip_uyelik_yolu(dugum_a(e), i, L);
            liste_ekle(L, e);
            return;
        }
        if ((dugum_tur(e) == DugumTur_D_KIMLIK))
        {
            liste_ekle(L, e);
        }
    }
}

long long upp_yol(long long e) {
    {
        long long L = liste_yeni();
        tip_uyelik_yolu(e, 0LL, L);
        if ((liste_uzunluk(L) < 1LL))
        {
            return 0LL;
        }
        long long first = liste_al(L, 0LL);
        if (((dugum_tur(first) != DugumTur_D_KIMLIK) || (!metin_esit(dugum_ad(first), "upp"))))
        {
            return 0LL;
        }
        return L;
    }
}

bool sayilabilir_dizi_mi(long long e) {
    {
        if ((e == 0LL))
        {
            return false;
        }
        if ((dugum_tur(e) == DugumTur_D_DIZI_SABIT))
        {
            return true;
        }
        if ((dugum_tur(e) == DugumTur_D_KIMLIK))
        {
            long long t = kapsam_bul(dugum_ad(e));
            return ((t != 0LL) && (dugum_dims(t) > 0LL));
        }
        if ((dugum_tur(e) == DugumTur_D_UYE))
        {
            long long t = tip_ifade(e);
            return ((t != 0LL) && (dugum_dims(t) > 0LL));
        }
        return false;
    }
}

bool secim_uygun_tur(long long t) {
    {
        if ((t == 0LL))
        {
            return false;
        }
        if (((dugum_ptrd(t) != 0LL) || (dugum_dims(t) != 0LL)))
        {
            return false;
        }
        char* n = dugum_ad(t);
        if (((metin_esit(n, "sayi") || metin_esit(n, "mantik")) || metin_esit(n, "metin")))
        {
            return true;
        }
        return tip_enum_mi(t);
    }
}

bool secim_sabit_mi(long long e) {
    {
        long long t = 0LL;
        long long k = 0LL;
        long long obj_t = 0LL;
        char* on = upp_metin_kopya("");
        if ((e == 0LL))
        {
            return false;
        }
        (t = dugum_tur(e));
        if ((t == DugumTur_D_SABIT))
        {
            (k = dugum_ptrd(e));
            return (((k == 0LL) || (k == 3LL)) || (k == 2LL));
        }
        if (((t == DugumTur_D_TEKIL) && metin_esit(dugum_ad(e), "-")))
        {
            return secim_sabit_mi(dugum_a(e));
        }
        if ((t == DugumTur_D_UYE))
        {
            (obj_t = tip_ifade(dugum_a(e)));
            if ((obj_t == 0LL))
            {
                return false;
            }
            (on = dugum_ad(obj_t));
            if (((metin_uzun(on) >= 8LL) && metin_esit(intern_dilim(on, 0LL, 8LL), "secenek.")))
            {
                return true;
            }
            return false;
        }
        return false;
    }
}

long long bg_ad_listesi_kopya(long long L) {
    {
        long long N = liste_yeni();
        long long i = 0LL;
        while ((i < liste_uzunluk(L)))
        {
            liste_ekle(N, liste_al(L, i));
            (i = (i + 1LL));
        }
        return N;
    }
}

bool bg_liste_ad_var(long long L, char* ad) {
    {
        long long i = 0LL;
        while ((i < liste_uzunluk(L)))
        {
            if (metin_esit(dugum_ad(liste_al(L, i)), ad))
            {
                return true;
            }
            (i = (i + 1LL));
        }
        return false;
    }
}

void bg_ident_ekle(long long idn, long long declared, long long used, long long yer) {
    {
        char* ad = upp_metin_kopya("");
        long long ty = 0LL;
        long long cap = 0LL;
        if (((idn == 0LL) || hata_var()))
        {
            return;
        }
        (ad = dugum_ad(idn));
        if (((tur_adi_mi(ad) || metin_esit(ad, "upp")) || (tablo_bul(2LL, ad) != 0LL)))
        {
            return;
        }
        if (bg_liste_ad_var(declared, ad))
        {
            return;
        }
        if (bg_liste_ad_var(used, ad))
        {
            return;
        }
        if (metin_esit(ad, "kendi"))
        {
            if (metin_esit(cur_class_al(), ""))
            {
                return;
            }
            (ty = tur_dugum(dugum_satir(yer), dugum_sutun(yer), cur_class_al(), 1LL, 0LL));
        }
        else
        {
            (ty = kapsam_bul(ad));
            if ((ty == 0LL))
            {
                return;
            }
            (ty = tur_kopya(ty));
        }
        if ((dugum_dims(ty) > 0LL))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'arkaplan' dizi '");
            hata_fmt_yaz(ad);
            hata_fmt_yaz("' yakalayamaz. Elemanı kopyalayın veya diziyi iş parçacığı içinde oluşturun.");
            hata_fmt_ver(dugum_satir(yer), dugum_sutun(yer));
            return;
        }
        if ((liste_tur_mi(dugum_ad(ty)) || harita_tur_mi(dugum_ad(ty))))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'arkaplan' '");
            hata_fmt_yaz(dugum_ad(ty));
            hata_fmt_yaz("' '");
            hata_fmt_yaz(ad);
            hata_fmt_yaz("' yakalayamaz. Koleksiyonu iş parçacığı içinde oluşturun.");
            hata_fmt_ver(dugum_satir(yer), dugum_sutun(yer));
            return;
        }
        if (metin_esit(dugum_ad(ty), "JSONDeger"))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'arkaplan' JSONDeger '");
            hata_fmt_yaz(ad);
            hata_fmt_yaz("' yakalayamaz. Değeri iş parçacığı içinde okuyun.");
            hata_fmt_ver(dugum_satir(yer), dugum_sutun(yer));
            return;
        }
        (cap = kimlik_dugum(dugum_satir(yer), dugum_sutun(yer), ad));
        dugum_yaz_a(cap, ty);
        liste_ekle(used, cap);
    }
}

void bg_interp_topla(long long e, long long declared, long long used, long long yer) {
    {
        char* raw = upp_metin_kopya("");
        char* text = upp_metin_kopya("");
        char* name = upp_metin_kopya("");
        char* ilk = upp_metin_kopya("");
        long long n = 0LL;
        long long i = 0LL;
        long long c = 0LL;
        long long adv = 0LL;
        long long j = 0LL;
        long long k = 0LL;
        long long nn = 0LL;
        long long nokta = (-1LL);
        if ((((e == 0LL) || (dugum_tur(e) != DugumTur_D_SABIT)) || (dugum_ptrd(e) != 2LL)))
        {
            return;
        }
        (raw = dugum_ad(e));
        (text = metin_coz(raw));
        (n = metin_uzun(text));
        (i = 0LL);
        while (((i < n) && (!hata_var())))
        {
            (c = metin_cp(text, i));
            (adv = metin_adv(text, i));
            if ((((c == 123LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 123LL)))
            {
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((((c == 125LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 125LL)))
            {
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((c == 123LL))
            {
                (j = (i + adv));
                while (((j < n) && (metin_cp(text, j) != 125LL)))
                {
                    (j = (j + metin_adv(text, j)));
                }
                if ((j >= n))
                {
                    return;
                }
                (name = intern_dilim(text, (i + adv), j));
                if ((!metin_esit(name, "")))
                {
                    (nokta = (-1LL));
                    (k = 0LL);
                    (nn = metin_uzun(name));
                    while ((k < nn))
                    {
                        if ((metin_cp(name, k) == 46LL))
                        {
                            (nokta = k);
                        }
                        (k = (k + metin_adv(name, k)));
                    }
                    if ((nokta < 0LL))
                    {
                        (ilk = name);
                    }
                    else
                    {
                        (ilk = intern_dilim(name, 0LL, nokta));
                    }
                    bg_ident_ekle(kimlik_dugum(dugum_satir(e), dugum_sutun(e), ilk), declared, used, yer);
                }
                (i = (j + 1LL));
                continue;
            }
            (i = (i + adv));
        }
    }
}

void bg_ident_topla(long long n, long long declared, long long used, long long yer) {
    {
        long long t = 0LL;
        long long L = 0LL;
        long long i = 0LL;
        long long inner = 0LL;
        if (((n == 0LL) || hata_var()))
        {
            return;
        }
        (t = dugum_tur(n));
        if ((t == DugumTur_D_DEGISKEN))
        {
            bg_ident_topla(dugum_c(n), declared, used, yer);
            bg_ident_topla(dugum_b(n), declared, used, yer);
            liste_ekle(declared, kimlik_dugum(dugum_satir(n), dugum_sutun(n), dugum_ad(n)));
            return;
        }
        if ((t == DugumTur_D_KIMLIK))
        {
            bg_ident_ekle(n, declared, used, yer);
            return;
        }
        if ((t == DugumTur_D_SABIT))
        {
            bg_interp_topla(n, declared, used, yer);
            return;
        }
        if (((t == DugumTur_D_ARKAPLAN) || (t == DugumTur_D_ARKAPLAN_IFADE)))
        {
            (inner = bg_ad_listesi_kopya(declared));
            bg_ident_topla(dugum_a(n), inner, used, yer);
            return;
        }
        if ((t == DugumTur_D_HER))
        {
            bg_ident_topla(dugum_b(n), declared, used, yer);
            (inner = bg_ad_listesi_kopya(declared));
            liste_ekle(inner, kimlik_dugum(dugum_satir(n), dugum_sutun(n), dugum_ad(n)));
            bg_ident_topla(dugum_c(n), inner, used, yer);
            return;
        }
        bg_ident_topla(dugum_a(n), declared, used, yer);
        bg_ident_topla(dugum_b(n), declared, used, yer);
        bg_ident_topla(dugum_c(n), declared, used, yer);
        bg_ident_topla(dugum_d(n), declared, used, yer);
        (L = dugum_liste(n));
        (i = 0LL);
        while ((i < liste_uzunluk(L)))
        {
            bg_ident_topla(liste_al(L, i), declared, used, yer);
            (i = (i + 1LL));
        }
    }
}

void bg_yakala(long long node) {
    {
        long long used = 0LL;
        long long declared = 0LL;
        if (((node == 0LL) || hata_var()))
        {
            return;
        }
        (used = liste_yeni());
        (declared = liste_yeni());
        bg_ident_topla(dugum_a(node), declared, used, node);
        dugum_yaz_liste2(node, used);
    }
}

bool sabit_dogru_mu(long long e) {
    {
        if ((e == 0LL))
        {
            return false;
        }
        return (((dugum_tur(e) == DugumTur_D_SABIT) && (dugum_ptrd(e) == 3LL)) && (dugum_sayi(e) != 0LL));
    }
}

bool her_zaman_doner(long long stmt) {
    {
        if ((stmt == 0LL))
        {
            return false;
        }
        long long t = dugum_tur(stmt);
        if ((t == DugumTur_D_DON))
        {
            return true;
        }
        if ((t == DugumTur_D_BLOK))
        {
            long long L = dugum_liste(stmt);
            long long i = 0LL;
            while ((i < liste_uzunluk(L)))
            {
                if (her_zaman_doner(liste_al(L, i)))
                {
                    return true;
                }
                (i = (i + 1LL));
            }
            return false;
        }
        if (((t == DugumTur_D_GUVENSIZ) || (t == DugumTur_D_GUVENLI)))
        {
            return her_zaman_doner(dugum_a(stmt));
        }
        if ((t == DugumTur_D_EGER))
        {
            return (her_zaman_doner(dugum_b(stmt)) && her_zaman_doner(dugum_c(stmt)));
        }
        if ((t == DugumTur_D_IKEN))
        {
            if (sabit_dogru_mu(dugum_a(stmt)))
            {
                return her_zaman_doner(dugum_b(stmt));
            }
            return false;
        }
        if ((t == DugumTur_D_DONGU))
        {
            long long c = dugum_b(stmt);
            if (((c == 0LL) || sabit_dogru_mu(c)))
            {
                return her_zaman_doner(dugum_d(stmt));
            }
            return false;
        }
        if ((t == DugumTur_D_SECIM))
        {
            long long L = dugum_liste(stmt);
            if ((liste_uzunluk(L) == 0LL))
            {
                return false;
            }
            bool def = false;
            long long i = 0LL;
            while ((i < liste_uzunluk(L)))
            {
                if ((dugum_a(liste_al(L, i)) == 0LL))
                {
                    (def = true);
                }
                (i = (i + 1LL));
            }
            if ((!def))
            {
                return false;
            }
            (i = 0LL);
            while ((i < liste_uzunluk(L)))
            {
                long long k = i;
                bool ok = false;
                while (((k < liste_uzunluk(L)) && (!ok)))
                {
                    long long sl = dugum_liste(liste_al(L, k));
                    long long dolu = 0LL;
                    bool var_don = false;
                    long long j = 0LL;
                    while ((j < liste_uzunluk(sl)))
                    {
                        long long s = liste_al(sl, j);
                        if ((dugum_tur(s) != DugumTur_D_BOS_CUMLE))
                        {
                            (dolu = (dolu + 1LL));
                            if (her_zaman_doner(s))
                            {
                                (var_don = true);
                            }
                        }
                        (j = (j + 1LL));
                    }
                    if (var_don)
                    {
                        (ok = true);
                    }
                    else
                        if ((dolu > 0LL))
                        {
                            break;
                        }
                        else
                        {
                            (k = (k + 1LL));
                        }
                }
                if ((!ok))
                {
                    return false;
                }
                (i = (i + 1LL));
            }
            return true;
        }
        return false;
    }
}

void tip_interp(long long e) {
    {
        if ((((e == 0LL) || (dugum_tur(e) != DugumTur_D_SABIT)) || (dugum_ptrd(e) != 2LL)))
        {
            return;
        }
        char* raw = dugum_ad(e);
        char* text = metin_coz(raw);
        long long n = metin_uzun(text);
        long long i = 0LL;
        while ((i < n))
        {
            long long c = metin_cp(text, i);
            long long adv = metin_adv(text, i);
            if ((((c == 123LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 123LL)))
            {
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((((c == 125LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 125LL)))
            {
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((c == 123LL))
            {
                long long j = (i + adv);
                while (((j < n) && (metin_cp(text, j) != 125LL)))
                {
                    (j = (j + metin_adv(text, j)));
                }
                if ((j >= n))
                {
                    (i = (i + adv));
                    continue;
                }
                char* name = intern_dilim(text, (i + adv), j);
                if (metin_esit(name, ""))
                {
                    hata_ver(dugum_satir(e), dugum_sutun(e), "Boş interpolasyon '{{ }}' geçersiz.");
                    return;
                }
                if ((!interp_ad_mi(name)))
                {
                    (i = (j + 1LL));
                    continue;
                }
                long long nokta = (-1LL);
                long long k = 0LL;
                long long nn = metin_uzun(name);
                while ((k < nn))
                {
                    if ((metin_cp(name, k) == 46LL))
                    {
                        (nokta = k);
                    }
                    (k = (k + metin_adv(name, k)));
                }
                long long expr = 0LL;
                if ((nokta < 0LL))
                {
                    (expr = kimlik_dugum(dugum_satir(e), dugum_sutun(e), name));
                }
                else
                {
                    (expr = dugum_yeni(DugumTur_D_UYE, dugum_satir(e), dugum_sutun(e)));
                    dugum_yaz_a(expr, kimlik_dugum(dugum_satir(e), dugum_sutun(e), intern_dilim(name, 0LL, nokta)));
                    dugum_yaz_ad(expr, intern_dilim(name, (nokta + 1LL), nn));
                }
                tip_ifade(expr);
                (i = (j + 1LL));
                continue;
            }
            (i = (i + adv));
        }
    }
}

long long tip_liste_uye(long long expr, char* on) {
    {
        char* name = dugum_ad(expr);
        char* elem = liste_elem_ad(on);
        if (((metin_esit(name, "ekle") || metin_esit(name, "yaz")) || metin_esit(name, "bosalt")))
        {
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "al"))
        {
            return tur_dugum(dugum_satir(expr), dugum_sutun(expr), elem, 0LL, 0LL);
        }
        if (metin_esit(name, "uzunluk"))
        {
            return tip_sayi(dugum_satir(expr), dugum_sutun(expr));
        }
        hata_ver(dugum_satir(expr), dugum_sutun(expr), "'liste' üzerinde bu üye yok.");
        return tip_bos(dugum_satir(expr), dugum_sutun(expr));
    }
}

long long tip_harita_uye(long long expr, char* on) {
    {
        char* name = dugum_ad(expr);
        char* val = harita_deger_ad(on);
        if ((metin_esit(name, "koy") || metin_esit(name, "bosalt")))
        {
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "al"))
        {
            return tur_dugum(dugum_satir(expr), dugum_sutun(expr), val, 0LL, 0LL);
        }
        if (metin_esit(name, "var_mi"))
        {
            return tip_mantik(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "uzunluk"))
        {
            return tip_sayi(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "anahtarlar"))
        {
            return tur_dugum(dugum_satir(expr), dugum_sutun(expr), kat3("liste[", harita_anahtar_ad(on), "]"), 0LL, 0LL);
        }
        hata_ver(dugum_satir(expr), dugum_sutun(expr), "'harita' üzerinde bu üye yok.");
        return tip_bos(dugum_satir(expr), dugum_sutun(expr));
    }
}

long long tip_uye(long long expr, bool as_call) {
    {
        long long obj_t = tip_ifade(dugum_a(expr));
        if ((hata_var() || (obj_t == 0LL)))
        {
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        char* on = dugum_ad(obj_t);
        if ((metin_uzun(on) >= 8LL))
        {
            char* pref = intern_dilim(on, 0LL, 8LL);
            if (metin_esit(pref, "secenek."))
            {
                char* en = intern_dilim(on, 8LL, metin_uzun(on));
                long long ed = tablo_bul(2LL, en);
                if ((ed == 0LL))
                {
                    hata_ver(dugum_satir(expr), dugum_sutun(expr), "Seçenek bulunamadı.");
                    return tip_bos(dugum_satir(expr), dugum_sutun(expr));
                }
                long long L = dugum_liste(ed);
                long long i = 0LL;
                while ((i < liste_uzunluk(L)))
                {
                    long long v = liste_al(L, i);
                    if (metin_esit(dugum_ad(v), dugum_ad(expr)))
                    {
                        if (as_call)
                        {
                            hata_ver(dugum_satir(expr), dugum_sutun(expr), "Seçenek değeri çağrılamaz.");
                        }
                        return tur_dugum(dugum_satir(expr), dugum_sutun(expr), en, 0LL, 0LL);
                    }
                    (i = (i + 1LL));
                }
                hata_fmt_basla();
                hata_fmt_yaz("Seçenekte üye yok: ");
                hata_fmt_yaz(dugum_ad(expr));
                hata_fmt_ver(dugum_satir(expr), dugum_sutun(expr));
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
        }
        if ((metin_esit(on, "upp") || ((metin_uzun(on) >= 4LL) && metin_esit(intern_dilim(on, 0LL, 4LL), "upp."))))
        {
            return tur_dugum(dugum_satir(expr), dugum_sutun(expr), intern(kat3(on, ".", dugum_ad(expr))), 0LL, 0LL);
        }
        if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "modul.")))
        {
            char* alias = intern_dilim(on, 6LL, metin_uzun(on));
            char* uye = dugum_ad(expr);
            long long fn = tablo_bul(1LL, c_sembol(uye, alias));
            if ((fn != 0LL))
            {
                long long rt = dugum_a(fn);
                return tur_dugum(dugum_satir(fn), dugum_sutun(fn), dugum_ad(rt), dugum_ptrd(rt), 0LL);
            }
            long long cls = tablo_bul(0LL, kat3(alias, ".", uye));
            if ((cls != 0LL))
            {
                return tur_dugum(dugum_satir(expr), dugum_sutun(expr), intern(kat("sinif.", kat3(alias, ".", uye))), 0LL, 0LL);
            }
            long long ed = tablo_bul(2LL, kat3(alias, ".", uye));
            if ((ed != 0LL))
            {
                return tur_dugum(dugum_satir(expr), dugum_sutun(expr), intern(kat("secenek.", kat3(alias, ".", uye))), 0LL, 0LL);
            }
            long long vr = tablo_bul(3LL, c_sembol(uye, alias));
            if ((vr != 0LL))
            {
                return tur_kopya(vr);
            }
            hata_ver(dugum_satir(expr), dugum_sutun(expr), "Modülde üye yok.");
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "sinif.")))
        {
            char* key = intern_dilim(on, 6LL, metin_uzun(on));
            long long cls = tablo_bul(0LL, key);
            if ((cls == 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "Sınıf bulunamadı.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            if ((dugum_c(cls) != 0LL))
            {
                long long L = dugum_liste(dugum_c(cls));
                long long i = 0LL;
                while ((i < liste_uzunluk(L)))
                {
                    long long c = liste_al(L, i);
                    if (metin_esit(dugum_ad(c), dugum_ad(expr)))
                    {
                        if (as_call)
                        {
                            hata_ver(dugum_satir(expr), dugum_sutun(expr), "Sınıf sabiti çağrılamaz.");
                        }
                        return tur_kopya(dugum_a(c));
                    }
                    (i = (i + 1LL));
                }
            }
            hata_ver(dugum_satir(expr), dugum_sutun(expr), "Sınıf sabiti yok.");
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if ((liste_tur_mi(on) && (dugum_ptrd(obj_t) == 0LL)))
        {
            return tip_liste_uye(expr, on);
        }
        if ((harita_tur_mi(on) && (dugum_ptrd(obj_t) == 0LL)))
        {
            return tip_harita_uye(expr, on);
        }
        long long cls = tablo_bul(0LL, on);
        if ((cls == 0LL))
        {
            hata_fmt_basla();
            hata_fmt_yaz("Sınıf değil; üye erişimi yok: ");
            hata_fmt_yaz(on);
            hata_fmt_ver(dugum_satir(expr), dugum_sutun(expr));
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        long long F = dugum_liste(cls);
        long long i = 0LL;
        while ((i < liste_uzunluk(F)))
        {
            long long f = liste_al(F, i);
            if (metin_esit(dugum_ad(f), dugum_ad(expr)))
            {
                if (as_call)
                {
                    hata_ver(dugum_satir(expr), dugum_sutun(expr), "Alan fonksiyon gibi çağrılamaz.");
                }
                long long dims = 0LL;
                if (((dugum_b(f) != 0LL) || ((dugum_bayrak(f) & 8LL) != 0LL)))
                {
                    (dims = 1LL);
                }
                long long ft = dugum_a(f);
                return tur_dugum(dugum_satir(f), dugum_sutun(f), dugum_ad(ft), dugum_ptrd(ft), dims);
            }
            (i = (i + 1LL));
        }
        long long M = dugum_liste2(cls);
        (i = 0LL);
        while ((i < liste_uzunluk(M)))
        {
            long long m = liste_al(M, i);
            if (metin_esit(dugum_ad(m), dugum_ad(expr)))
            {
                long long rt = dugum_a(m);
                return tur_dugum(dugum_satir(m), dugum_sutun(m), dugum_ad(rt), dugum_ptrd(rt), 0LL);
            }
            (i = (i + 1LL));
        }
        if ((metin_esit(dugum_ad(cls), "ArkaplanIs") && metin_esit(dugum_ad(expr), "bekle")))
        {
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if ((metin_esit(dugum_ad(cls), "Kilit") && (metin_esit(dugum_ad(expr), "kilitle") || metin_esit(dugum_ad(expr), "birak"))))
        {
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        hata_fmt_basla();
        hata_fmt_yaz("Sınıfta üye yok: ");
        hata_fmt_yaz(dugum_ad(expr));
        hata_fmt_ver(dugum_satir(expr), dugum_sutun(expr));
        return tip_bos(dugum_satir(expr), dugum_sutun(expr));
    }
}

long long upp_donus(char* name, long long satir, long long sutun) {
    {
        if (((((((((metin_esit(name, "yaz") || metin_esit(name, "satir_yaz")) || metin_esit(name, "mesaj")) || metin_esit(name, "hata")) || metin_esit(name, "uyut")) || metin_esit(name, "metin_bosalt")) || metin_esit(name, "bekle")) || metin_esit(name, "hata_yaz")) || metin_esit(name, "hata_satir_yaz")))
        {
            return tip_bos(satir, sutun);
        }
        if ((((((((((((metin_esit(name, "ses_cal") || metin_esit(name, "rastgele")) || metin_esit(name, "dosya_yaz")) || metin_esit(name, "uzunluk")) || metin_esit(name, "min")) || metin_esit(name, "max")) || metin_esit(name, "mutlak")) || metin_esit(name, "arguman_sayisi")) || metin_esit(name, "zaman")) || metin_esit(name, "metinden_sayiya")) || metin_esit(name, "dosya_ekle")) || metin_esit(name, "dosya_boyut")))
        {
            return tip_sayi(satir, sutun);
        }
        if (((((((metin_esit(name, "dosya_oku") || metin_esit(name, "giris")) || metin_esit(name, "arguman")) || metin_esit(name, "sayidan_metin")) || metin_esit(name, "ondalikdan_metin")) || metin_esit(name, "ortam_al")) || metin_esit(name, "platform")))
        {
            return tip_metin(satir, sutun);
        }
        if (metin_esit(name, "metinden_ondalik"))
        {
            return tip_ondalik(satir, sutun);
        }
        if (((metin_esit(name, "dosya_var_mi") || metin_esit(name, "dosya_sil")) || metin_esit(name, "ortam_yaz")))
        {
            return tur_dugum(satir, sutun, "mantik", 0LL, 0LL);
        }
        return 0LL;
    }
}

bool json_ad_mi(char* name) {
    {
        if ((((((((((((((((((((((metin_esit(name, "ayristir") || metin_esit(name, "olustur")) || metin_esit(name, "tur")) || metin_esit(name, "anahtarlar")) || metin_esit(name, "anahtar_al")) || metin_esit(name, "dizi_eleman")) || metin_esit(name, "metin_al")) || metin_esit(name, "sayi_al")) || metin_esit(name, "ondalik_al")) || metin_esit(name, "mantik_al")) || metin_esit(name, "uzunluk")) || metin_esit(name, "var_mi")) || metin_esit(name, "nesne")) || metin_esit(name, "dizi")) || metin_esit(name, "metin_yap")) || metin_esit(name, "sayi_yap")) || metin_esit(name, "ondalik_yap")) || metin_esit(name, "mantik_yap")) || metin_esit(name, "yok_yap")) || metin_esit(name, "koy")) || metin_esit(name, "ekle")) || metin_esit(name, "bosalt")))
        {
            return true;
        }
        return false;
    }
}

bool json_arity_ok(char* name, long long n) {
    {
        if (((metin_esit(name, "nesne") || metin_esit(name, "dizi")) || metin_esit(name, "yok_yap")))
        {
            return (n == 0LL);
        }
        if (metin_esit(name, "koy"))
        {
            return (n == 3LL);
        }
        if (((metin_esit(name, "anahtar_al") || metin_esit(name, "dizi_eleman")) || metin_esit(name, "ekle")))
        {
            return (n == 2LL);
        }
        return (n == 1LL);
    }
}

long long json_donus(char* name, long long satir, long long sutun) {
    {
        if (((metin_esit(name, "olustur") || metin_esit(name, "tur")) || metin_esit(name, "metin_al")))
        {
            return tip_metin(satir, sutun);
        }
        if ((metin_esit(name, "sayi_al") || metin_esit(name, "uzunluk")))
        {
            return tip_sayi(satir, sutun);
        }
        if (metin_esit(name, "ondalik_al"))
        {
            return tip_ondalik(satir, sutun);
        }
        if ((metin_esit(name, "mantik_al") || metin_esit(name, "var_mi")))
        {
            return tip_mantik(satir, sutun);
        }
        if (((metin_esit(name, "koy") || metin_esit(name, "ekle")) || metin_esit(name, "bosalt")))
        {
            return tip_bos(satir, sutun);
        }
        return tur_dugum(satir, sutun, "JSONDeger", 0LL, 0LL);
    }
}

long long upp_ns_donus(char* ns, char* name, long long n, long long satir, long long sutun) {
    {
        if (metin_esit(ns, "json"))
        {
            if ((!json_ad_mi(name)))
            {
                return 0LL;
            }
            if ((!json_arity_ok(name, n)))
            {
                hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                return 0LL;
            }
            return json_donus(name, satir, sutun);
        }
        if (metin_esit(ns, "stdio"))
        {
            if (metin_esit(name, "gelen_oku"))
            {
                if ((n != 0LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_metin(satir, sutun);
            }
            if (metin_esit(name, "yaz"))
            {
                if ((n != 1LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_bos(satir, sutun);
            }
            return 0LL;
        }
        if (metin_esit(ns, "sistem"))
        {
            if (metin_esit(name, "calistir"))
            {
                if ((n != 1LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tur_dugum(satir, sutun, "SurecCikti", 0LL, 0LL);
            }
            if (metin_esit(name, "komut_bul"))
            {
                if ((n != 1LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_metin(satir, sutun);
            }
            if (metin_esit(name, "exe_dizin"))
            {
                if ((n != 0LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_metin(satir, sutun);
            }
            if (metin_esit(name, "pid"))
            {
                if ((n != 0LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_sayi(satir, sutun);
            }
            return 0LL;
        }
        if (metin_esit(ns, "linux"))
        {
            if (metin_esit(name, "pid"))
            {
                if ((n != 0LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_sayi(satir, sutun);
            }
            if (metin_esit(name, "sinyal_gonder"))
            {
                if ((n != 2LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_mantik(satir, sutun);
            }
            if (metin_esit(name, "proc_oku"))
            {
                if ((n != 2LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_metin(satir, sutun);
            }
            if (metin_esit(name, "sayi_oku"))
            {
                if ((n != 2LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_sayi(satir, sutun);
            }
            if (metin_esit(name, "sayi_yaz"))
            {
                if ((n != 3LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_mantik(satir, sutun);
            }
            return 0LL;
        }
        if (metin_esit(ns, "windows"))
        {
            if ((metin_esit(name, "mesaj") || metin_esit(name, "hata")))
            {
                if ((n != 2LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_bos(satir, sutun);
            }
            if (metin_esit(name, "ses_cal"))
            {
                if ((n != 1LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_sayi(satir, sutun);
            }
            return 0LL;
        }
        if (metin_esit(ns, "yol"))
        {
            if ((metin_esit(name, "dizin") || metin_esit(name, "gercek")))
            {
                if ((n != 1LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_metin(satir, sutun);
            }
            if (metin_esit(name, "birlestir"))
            {
                if ((n != 2LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_metin(satir, sutun);
            }
            if (metin_esit(name, "calisma"))
            {
                if ((n != 0LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_metin(satir, sutun);
            }
            if ((metin_esit(name, "mutlak_mi") || metin_esit(name, "klasor_olustur")))
            {
                if ((n != 1LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tip_mantik(satir, sutun);
            }
            return 0LL;
        }
        if (metin_esit(ns, "dosya"))
        {
            if (metin_esit(name, "listele"))
            {
                if ((n != 1LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return 0LL;
                }
                return tur_dugum(satir, sutun, "liste[metin]", 0LL, 0LL);
            }
            return 0LL;
        }
        return 0LL;
    }
}

long long tip_liste_cagri(long long expr, char* on) {
    {
        long long callee = dugum_a(expr);
        long long args = dugum_liste(expr);
        char* name = dugum_ad(callee);
        char* elem = liste_elem_ad(on);
        if (metin_esit(name, "ekle"))
        {
            if ((liste_uzunluk(args) != 1LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.ekle 1 argüman bekler.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            if ((!tip_atanir(tur_dugum(dugum_satir(expr), dugum_sutun(expr), elem, 0LL, 0LL), tip_ifade(liste_al(args, 0LL)))))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.ekle argüman türü uyumsuz.");
            }
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "al"))
        {
            if ((liste_uzunluk(args) != 1LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.al 1 argüman bekler.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            tip_ifade(liste_al(args, 0LL));
            return tur_dugum(dugum_satir(expr), dugum_sutun(expr), elem, 0LL, 0LL);
        }
        if (metin_esit(name, "yaz"))
        {
            if ((liste_uzunluk(args) != 2LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.yaz 2 argüman bekler.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            tip_ifade(liste_al(args, 0LL));
            if ((!tip_atanir(tur_dugum(dugum_satir(expr), dugum_sutun(expr), elem, 0LL, 0LL), tip_ifade(liste_al(args, 1LL)))))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.yaz argüman türü uyumsuz.");
            }
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "uzunluk"))
        {
            if ((liste_uzunluk(args) != 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.uzunluk argüman almaz.");
            }
            return tip_sayi(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "bosalt"))
        {
            if ((liste_uzunluk(args) != 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.bosalt argüman almaz.");
            }
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "sil"))
        {
            if ((liste_uzunluk(args) != 1LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.sil 1 argüman bekler.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            tip_ifade(liste_al(args, 0LL));
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "temizle"))
        {
            if ((liste_uzunluk(args) != 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "liste.temizle argüman almaz.");
            }
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        hata_ver(dugum_satir(expr), dugum_sutun(expr), "'liste' üzerinde bu metot yok.");
        return tip_bos(dugum_satir(expr), dugum_sutun(expr));
    }
}

long long tip_harita_cagri(long long expr, char* on) {
    {
        long long callee = dugum_a(expr);
        long long args = dugum_liste(expr);
        char* name = dugum_ad(callee);
        char* ktur = harita_anahtar_ad(on);
        char* vtur = harita_deger_ad(on);
        if (metin_esit(name, "koy"))
        {
            if ((liste_uzunluk(args) != 2LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.koy 2 argüman bekler.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            if ((!tip_atanir(tur_dugum(dugum_satir(expr), dugum_sutun(expr), ktur, 0LL, 0LL), tip_ifade(liste_al(args, 0LL)))))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.koy anahtar türü uyumsuz.");
            }
            if ((!tip_atanir(tur_dugum(dugum_satir(expr), dugum_sutun(expr), vtur, 0LL, 0LL), tip_ifade(liste_al(args, 1LL)))))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.koy değer türü uyumsuz.");
            }
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if ((metin_esit(name, "al") || metin_esit(name, "var_mi")))
        {
            if ((liste_uzunluk(args) != 1LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.al/var_mi 1 argüman bekler.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            tip_ifade(liste_al(args, 0LL));
            if (metin_esit(name, "var_mi"))
            {
                return tip_mantik(dugum_satir(expr), dugum_sutun(expr));
            }
            return tur_dugum(dugum_satir(expr), dugum_sutun(expr), vtur, 0LL, 0LL);
        }
        if (metin_esit(name, "uzunluk"))
        {
            if ((liste_uzunluk(args) != 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.uzunluk argüman almaz.");
            }
            return tip_sayi(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "bosalt"))
        {
            if ((liste_uzunluk(args) != 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.bosalt argüman almaz.");
            }
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "anahtarlar"))
        {
            if ((liste_uzunluk(args) != 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.anahtarlar argüman almaz.");
            }
            return tur_dugum(dugum_satir(expr), dugum_sutun(expr), kat3("liste[", ktur, "]"), 0LL, 0LL);
        }
        if (metin_esit(name, "sil"))
        {
            if ((liste_uzunluk(args) != 1LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.sil 1 argüman bekler.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            tip_ifade(liste_al(args, 0LL));
            return tip_mantik(dugum_satir(expr), dugum_sutun(expr));
        }
        if (metin_esit(name, "temizle"))
        {
            if ((liste_uzunluk(args) != 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "harita.temizle argüman almaz.");
            }
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        hata_ver(dugum_satir(expr), dugum_sutun(expr), "'harita' üzerinde bu metot yok.");
        return tip_bos(dugum_satir(expr), dugum_sutun(expr));
    }
}

bool tip_args_ok(long long P, long long args, long long satir, long long sutun) {
    {
        long long ngot = liste_uzunluk(args);
        long long nneed = liste_uzunluk(P);
        long long i = 0LL;
        if ((ngot > nneed))
        {
            hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
            return false;
        }
        while ((i < nneed))
        {
            long long p = liste_al(P, i);
            if ((i < ngot))
            {
                long long got = tip_ifade(liste_al(args, i));
                long long pt = dugum_a(p);
                if (((pt != 0LL) && (!tip_atanir(pt, got))))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman türü uyumsuz.");
                }
            }
            else
                if ((dugum_c(p) == 0LL))
                {
                    hata_ver(satir, sutun, "Fonksiyon argüman sayısı uyumsuz.");
                    return false;
                }
            (i = (i + 1LL));
        }
        return true;
    }
}

long long tip_kurucu(long long cls, long long expr) {
    {
        long long args = dugum_liste(expr);
        long long M = dugum_liste2(cls);
        long long ol = 0LL;
        long long say = 0LL;
        long long i = 0LL;
        while ((i < liste_uzunluk(M)))
        {
            long long m = liste_al(M, i);
            if (metin_esit(dugum_ad(m), "olustur"))
            {
                (ol = m);
                (say = (say + 1LL));
            }
            (i = (i + 1LL));
        }
        if ((say != 1LL))
        {
            hata_ver(dugum_satir(expr), dugum_sutun(expr), "Kurucu için tek 'olustur' metodu gerekir.");
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if (((!tip_void_mi(dugum_a(ol))) || (dugum_ptrd(dugum_a(ol)) != 0LL)))
        {
            hata_ver(dugum_satir(expr), dugum_sutun(expr), "olustur dönüşü 'bos' olmalı.");
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        tip_args_ok(dugum_liste(ol), args, dugum_satir(expr), dugum_sutun(expr));
        return tur_dugum(dugum_satir(expr), dugum_sutun(expr), sinif_tur_ad(cls), 0LL, 0LL);
    }
}

long long tip_cagri(long long expr) {
    {
        long long callee = dugum_a(expr);
        long long args = dugum_liste(expr);
        if ((dugum_tur(callee) == DugumTur_D_UYE))
        {
            long long obj_t = tip_ifade(dugum_a(callee));
            if (hata_var())
            {
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            char* on = dugum_ad(obj_t);
            if (metin_esit(on, "upp"))
            {
                char* name = dugum_ad(callee);
                long long rt = upp_donus(name, dugum_satir(expr), dugum_sutun(expr));
                if ((rt == 0LL))
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("'upp.");
                    hata_fmt_yaz(name);
                    hata_fmt_yaz("' yerleşik bir fonksiyon değil.");
                    hata_fmt_ver(dugum_satir(expr), dugum_sutun(expr));
                    return tip_bos(dugum_satir(expr), dugum_sutun(expr));
                }
                long long i = 0LL;
                while ((i < liste_uzunluk(args)))
                {
                    tip_ifade(liste_al(args, i));
                    tip_interp(liste_al(args, i));
                    (i = (i + 1LL));
                }
                if ((metin_esit(name, "uzunluk") && (liste_uzunluk(args) == 1LL)))
                {
                    long long a0 = liste_al(args, 0LL);
                    long long at = tip_ifade(a0);
                    if (((dugum_dims(at) > 0LL) && (!sayilabilir_dizi_mi(a0))))
                    {
                        hata_ver(dugum_satir(a0), dugum_sutun(a0), "'upp.uzunluk' gerçek dizi veya '[...]' bekler; işaretçi verilemez.");
                    }
                    if ((((((dugum_dims(at) == 0LL) && (!tip_metin_mi(at))) && (!tip_yok_mi(at))) && (!liste_tur_mi(dugum_ad(at)))) && (!harita_tur_mi(dugum_ad(at)))))
                    {
                        hata_ver(dugum_satir(expr), dugum_sutun(expr), "'upp.uzunluk' metin, dizi, liste veya harita bekler.");
                    }
                }
                return rt;
            }
            if (((metin_uzun(on) >= 4LL) && metin_esit(intern_dilim(on, 0LL, 4LL), "upp.")))
            {
                char* ns = intern_dilim(on, 4LL, metin_uzun(on));
                char* name = dugum_ad(callee);
                long long i = 0LL;
                while ((i < liste_uzunluk(args)))
                {
                    tip_ifade(liste_al(args, i));
                    (i = (i + 1LL));
                }
                if (metin_esit(ns, "metin"))
                {
                    if (((metin_esit(name, "icinde") || metin_esit(name, "kod")) || metin_esit(name, "ileri")))
                    {
                        return tip_sayi(dugum_satir(expr), dugum_sutun(expr));
                    }
                    if ((metin_esit(name, "baslar") || metin_esit(name, "biter")))
                    {
                        return tip_mantik(dugum_satir(expr), dugum_sutun(expr));
                    }
                    if (metin_esit(name, "bol"))
                    {
                        return tur_dugum(dugum_satir(expr), dugum_sutun(expr), "liste[metin]", 0LL, 0LL);
                    }
                    return tip_metin(dugum_satir(expr), dugum_sutun(expr));
                }
                if (metin_esit(ns, "matematik"))
                {
                    if ((metin_esit(name, "dunya_ekran") && (liste_uzunluk(args) >= 4LL)))
                    {
                        long long m = liste_al(args, 3LL);
                        if ((!sayilabilir_dizi_mi(m)))
                        {
                            hata_ver(dugum_satir(m), dugum_sutun(m), "'upp.matematik.dunya_ekran' gerçek ondalik dizisi bekler.");
                        }
                        else
                            if (((dugum_tur(m) == DugumTur_D_DIZI_SABIT) && (liste_uzunluk(dugum_liste(m)) < 16LL)))
                            {
                                hata_ver(dugum_satir(m), dugum_sutun(m), "'upp.matematik.dunya_ekran' 16 elemanlı (4x4) ondalik dizisi bekler.");
                            }
                            else
                                if ((dugum_tur(m) == DugumTur_D_KIMLIK))
                                {
                                    long long ty = kapsam_bul(dugum_ad(m));
                                    if ((((ty != 0LL) && (dugum_sayi(ty) > 0LL)) && (dugum_sayi(ty) < 16LL)))
                                    {
                                        hata_ver(dugum_satir(m), dugum_sutun(m), "'upp.matematik.dunya_ekran' 16 elemanlı (4x4) ondalik dizisi bekler.");
                                    }
                                }
                    }
                    if (metin_esit(name, "aci_hesapla"))
                    {
                        return tur_dugum(dugum_satir(expr), dugum_sutun(expr), "Yonelme", 0LL, 0LL);
                    }
                    if (metin_esit(name, "dunya_ekran"))
                    {
                        return tur_dugum(dugum_satir(expr), dugum_sutun(expr), "EkranNokta", 0LL, 0LL);
                    }
                    return tip_ondalik(dugum_satir(expr), dugum_sutun(expr));
                }
                if ((metin_esit(ns, "windows.bellek") || metin_esit(ns, "bellek")))
                {
                    if ((((((((metin_esit(name, "sayioku") || metin_esit(name, "oku_ondalik")) || metin_esit(name, "yama_yap")) || metin_esit(name, "zincir_oku")) || metin_esit(name, "baglan")) || metin_esit(name, "modul_bul")) || metin_esit(name, "son_hata")) || metin_esit(name, "okundu")))
                    {
                        if ((liste_uzunluk(args) >= 1LL))
                        {
                            arg_tam_sayi_olmali(liste_al(args, 0LL));
                        }
                    }
                    else
                        if ((((metin_esit(name, "oku_metin") || metin_esit(name, "sayiyaz")) || metin_esit(name, "koruma_degistir")) || metin_esit(name, "koruma_degistir_sayi")))
                        {
                            if ((liste_uzunluk(args) >= 1LL))
                            {
                                arg_tam_sayi_olmali(liste_al(args, 0LL));
                            }
                            if ((liste_uzunluk(args) >= 2LL))
                            {
                                arg_tam_sayi_olmali(liste_al(args, 1LL));
                            }
                        }
                        else
                            if (metin_esit(name, "yaz_ondalik"))
                            {
                                if ((liste_uzunluk(args) >= 1LL))
                                {
                                    arg_tam_sayi_olmali(liste_al(args, 0LL));
                                }
                            }
                    if (metin_esit(name, "oku_ondalik"))
                    {
                        return tip_ondalik(dugum_satir(expr), dugum_sutun(expr));
                    }
                    if (metin_esit(name, "oku_metin"))
                    {
                        return tip_metin(dugum_satir(expr), dugum_sutun(expr));
                    }
                    return tip_sayi(dugum_satir(expr), dugum_sutun(expr));
                }
                if ((metin_esit(ns, "windows.cizim") || metin_esit(ns, "cizim")))
                {
                    return tip_bos(dugum_satir(expr), dugum_sutun(expr));
                }
                if ((metin_esit(ns, "windows.girdi") || metin_esit(ns, "girdi")))
                {
                    return tip_bos(dugum_satir(expr), dugum_sutun(expr));
                }
                if (((((((metin_esit(ns, "json") || metin_esit(ns, "stdio")) || metin_esit(ns, "sistem")) || metin_esit(ns, "yol")) || metin_esit(ns, "dosya")) || metin_esit(ns, "linux")) || metin_esit(ns, "windows")))
                {
                    long long rt = upp_ns_donus(ns, name, liste_uzunluk(args), dugum_satir(expr), dugum_sutun(expr));
                    if ((rt == 0LL))
                    {
                        if ((!hata_var()))
                        {
                            hata_fmt_basla();
                            hata_fmt_yaz("'");
                            hata_fmt_yaz(on);
                            hata_fmt_yaz(".");
                            hata_fmt_yaz(name);
                            hata_fmt_yaz("' yerleşik bir fonksiyon değil.");
                            hata_fmt_ver(dugum_satir(expr), dugum_sutun(expr));
                        }
                        return tip_bos(dugum_satir(expr), dugum_sutun(expr));
                    }
                    return rt;
                }
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "modul.")))
            {
                char* alias = intern_dilim(on, 6LL, metin_uzun(on));
                char* name = dugum_ad(callee);
                long long fn = tablo_bul(1LL, c_sembol(name, alias));
                if ((fn != 0LL))
                {
                    tip_args_ok(dugum_liste(fn), args, dugum_satir(expr), dugum_sutun(expr));
                    long long rt = dugum_a(fn);
                    return tur_dugum(dugum_satir(fn), dugum_sutun(fn), dugum_ad(rt), dugum_ptrd(rt), 0LL);
                }
                long long kcls = tablo_bul(0LL, intern(kat3(alias, ".", name)));
                if ((kcls != 0LL))
                {
                    return tip_kurucu(kcls, expr);
                }
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "Modülde üye yok.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "sinif.")))
            {
                char* key = intern_dilim(on, 6LL, metin_uzun(on));
                long long kcls = tablo_bul(0LL, key);
                if ((kcls == 0LL))
                {
                    hata_ver(dugum_satir(expr), dugum_sutun(expr), "Sınıf bulunamadı.");
                    return tip_bos(dugum_satir(expr), dugum_sutun(expr));
                }
                return tip_kurucu(kcls, expr);
            }
            if ((liste_tur_mi(on) && (dugum_ptrd(obj_t) == 0LL)))
            {
                return tip_liste_cagri(expr, on);
            }
            if ((harita_tur_mi(on) && (dugum_ptrd(obj_t) == 0LL)))
            {
                return tip_harita_cagri(expr, on);
            }
            long long cls = tablo_bul(0LL, on);
            if ((cls == 0LL))
            {
                hata_ver(dugum_satir(expr), dugum_sutun(expr), "Sınıf değil; metot çağrılamaz.");
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            if ((metin_esit(dugum_ad(cls), "ArkaplanIs") && metin_esit(dugum_ad(callee), "bekle")))
            {
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            if ((metin_esit(dugum_ad(cls), "Kilit") && (metin_esit(dugum_ad(callee), "kilitle") || metin_esit(dugum_ad(callee), "birak"))))
            {
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            long long M = dugum_liste2(cls);
            long long i = 0LL;
            while ((i < liste_uzunluk(M)))
            {
                long long m = liste_al(M, i);
                if (metin_esit(dugum_ad(m), dugum_ad(callee)))
                {
                    tip_args_ok(dugum_liste(m), args, dugum_satir(expr), dugum_sutun(expr));
                    long long rt = dugum_a(m);
                    return tur_dugum(dugum_satir(m), dugum_sutun(m), dugum_ad(rt), dugum_ptrd(rt), 0LL);
                }
                (i = (i + 1LL));
            }
            hata_ver(dugum_satir(expr), dugum_sutun(expr), "Sınıfta metot yok.");
            return tip_bos(dugum_satir(expr), dugum_sutun(expr));
        }
        if ((dugum_tur(callee) == DugumTur_D_KIMLIK))
        {
            char* name = dugum_ad(callee);
            if (metin_esit(name, "bellek_ayir"))
            {
                return tur_dugum(dugum_satir(expr), dugum_sutun(expr), "bos", 1LL, 0LL);
            }
            if (metin_esit(name, "bellek_bosalt"))
            {
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            long long cls_id = tablo_bul(0LL, name);
            if (((cls_id != 0LL) && (!yerlesik_sinif_mi(name))))
            {
                return tip_kurucu(cls_id, expr);
            }
            long long fn = tablo_bul(1LL, name);
            if ((fn == 0LL))
            {
                hata_fmt_basla();
                hata_fmt_yaz("Tanımsız fonksiyon: '");
                hata_fmt_yaz(name);
                hata_fmt_yaz("'.");
                hata_fmt_ver(dugum_satir(expr), dugum_sutun(expr));
                return tip_bos(dugum_satir(expr), dugum_sutun(expr));
            }
            long long P = dugum_liste(fn);
            tip_args_ok(P, args, dugum_satir(expr), dugum_sutun(expr));
            long long rt = dugum_a(fn);
            return tur_dugum(dugum_satir(fn), dugum_sutun(fn), dugum_ad(rt), dugum_ptrd(rt), 0LL);
        }
        hata_ver(dugum_satir(expr), dugum_sutun(expr), "Yalnızca ad veya 'nesne.metot' çağrılabilir.");
        return tip_bos(dugum_satir(expr), dugum_sutun(expr));
    }
}

long long tip_ikil(long long expr) {
    {
        long long left = tip_ifade(dugum_a(expr));
        long long right = tip_ifade(dugum_b(expr));
        char* op = dugum_ad(expr);
        long long satir = dugum_satir(expr);
        long long sutun = dugum_sutun(expr);
        if ((tip_sinif_deger_mi(left) || tip_sinif_deger_mi(right)))
        {
            hata_ver(satir, sutun, "Sınıf değerleri işlemle kullanılamaz. C'de yapı (struct) karşılaştırması veya aritmetiği yoktur.");
            return tip_mantik(satir, sutun);
        }
        if (metin_esit(op, "+"))
        {
            if ((tip_metin_mi(left) && tip_metin_mi(right)))
            {
                return tip_metin(satir, sutun);
            }
            if ((tip_metin_mi(left) || tip_metin_mi(right)))
            {
                hata_ver(satir, sutun, "Metin birleştirme yalnızca 'metin + metin' biçimindedir.");
                return tip_metin(satir, sutun);
            }
        }
        if ((metin_esit(op, "==") || metin_esit(op, "!=")))
        {
            return tip_mantik(satir, sutun);
        }
        if ((((metin_esit(op, "<") || metin_esit(op, ">")) || metin_esit(op, "<=")) || metin_esit(op, ">=")))
        {
            return tip_mantik(satir, sutun);
        }
        if ((metin_esit(op, "&&") || metin_esit(op, "||")))
        {
            return tip_mantik(satir, sutun);
        }
        if (((dugum_ptrd(left) > 0LL) || (dugum_ptrd(right) > 0LL)))
        {
            hata_ver(satir, sutun, "İşaretçi aritmetiği u++'ta yoktur.");
            return left;
        }
        if ((metin_esit(dugum_ad(left), "ondalik") || metin_esit(dugum_ad(right), "ondalik")))
        {
            return tip_ondalik(satir, sutun);
        }
        return tip_sayi(satir, sutun);
    }
}

bool sabit_tamsayi_mi(long long e) {
    {
        if ((e == 0LL))
        {
            return false;
        }
        if (((dugum_tur(e) == DugumTur_D_SABIT) && (dugum_ptrd(e) == 0LL)))
        {
            return true;
        }
        if ((dugum_tur(e) == DugumTur_D_TEKIL))
        {
            char* op = dugum_ad(e);
            if ((metin_esit(op, "-") || metin_esit(op, "+")))
            {
                return sabit_tamsayi_mi(dugum_a(e));
            }
        }
        return false;
    }
}

long long sabit_tamsayi_deger(long long e) {
    {
        if ((e == 0LL))
        {
            return 0LL;
        }
        if ((dugum_tur(e) == DugumTur_D_SABIT))
        {
            return dugum_sayi(e);
        }
        if ((dugum_tur(e) == DugumTur_D_TEKIL))
        {
            char* op = dugum_ad(e);
            if (metin_esit(op, "-"))
            {
                return (0LL - sabit_tamsayi_deger(dugum_a(e)));
            }
            if (metin_esit(op, "+"))
            {
                return sabit_tamsayi_deger(dugum_a(e));
            }
        }
        return 0LL;
    }
}

long long tip_ifade(long long e) {
    {
        if ((e == 0LL))
        {
            return tip_bos(0LL, 1LL);
        }
        if (hata_var())
        {
            return tip_bos(dugum_satir(e), dugum_sutun(e));
        }
        long long t = dugum_tur(e);
        if ((t == DugumTur_D_SABIT))
        {
            long long k = dugum_ptrd(e);
            if ((k == 0LL))
            {
                return tip_sayi(dugum_satir(e), dugum_sutun(e));
            }
            if ((k == 1LL))
            {
                return tip_ondalik(dugum_satir(e), dugum_sutun(e));
            }
            if ((k == 2LL))
            {
                tip_interp(e);
                return tip_metin(dugum_satir(e), dugum_sutun(e));
            }
            if ((k == 3LL))
            {
                return tip_mantik(dugum_satir(e), dugum_sutun(e));
            }
            if ((k == 4LL))
            {
                return tur_dugum(dugum_satir(e), dugum_sutun(e), "yok", 1LL, 0LL);
            }
            return tip_sayi(dugum_satir(e), dugum_sutun(e));
        }
        if ((t == DugumTur_D_KIMLIK))
        {
            char* name = dugum_ad(e);
            if (metin_esit(name, "kendi"))
            {
                if (metin_esit(cur_class_al(), ""))
                {
                    hata_ver(dugum_satir(e), dugum_sutun(e), "'kendi' yalnızca sınıf metodu içinde kullanılabilir.");
                    return tip_bos(dugum_satir(e), dugum_sutun(e));
                }
                return tur_dugum(dugum_satir(e), dugum_sutun(e), cur_class_al(), 1LL, 0LL);
            }
            if (metin_esit(name, "upp"))
            {
                return tur_dugum(dugum_satir(e), dugum_sutun(e), "upp", 0LL, 0LL);
            }
            if ((tablo_bul(3LL, intern(kat("modul.", name))) != 0LL))
            {
                return tur_dugum(dugum_satir(e), dugum_sutun(e), intern(kat("modul.", name)), 0LL, 0LL);
            }
            if (((tablo_bul(0LL, name) != 0LL) && (!yerlesik_sinif_mi(name))))
            {
                return tur_dugum(dugum_satir(e), dugum_sutun(e), intern(kat("sinif.", name)), 0LL, 0LL);
            }
            if ((tablo_bul(2LL, name) != 0LL))
            {
                return tur_dugum(dugum_satir(e), dugum_sutun(e), kat("secenek.", name), 0LL, 0LL);
            }
            long long found = kapsam_bul(name);
            if ((found == 0LL))
            {
                (found = tablo_bul(3LL, name));
            }
            if ((found == 0LL))
            {
                hata_fmt_basla();
                hata_fmt_yaz("Tanımsız ad: '");
                hata_fmt_yaz(name);
                hata_fmt_yaz("'.");
                hata_fmt_ver(dugum_satir(e), dugum_sutun(e));
                return tip_bos(dugum_satir(e), dugum_sutun(e));
            }
            return tur_kopya(found);
        }
        if ((t == DugumTur_D_UYE))
        {
            return tip_uye(e, false);
        }
        if ((t == DugumTur_D_INDIS))
        {
            long long inner = tip_ifade(dugum_a(e));
            long long idx = dugum_b(e);
            tip_ifade(idx);
            if ((((dugum_dims(inner) > 0LL) && (dugum_sayi(inner) > 0LL)) && sabit_tamsayi_mi(idx)))
            {
                long long iv = sabit_tamsayi_deger(idx);
                long long n = dugum_sayi(inner);
                if (((iv < 0LL) || (iv >= n)))
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("indeks ");
                    hata_fmt_sayi(iv);
                    hata_fmt_yaz(", dizi boyutu ");
                    hata_fmt_sayi(n);
                    hata_fmt_yaz("'yi aşıyor");
                    hata_fmt_ver(dugum_satir(e), dugum_sutun(e));
                }
            }
            if ((liste_tur_mi(dugum_ad(inner)) && (dugum_ptrd(inner) == 0LL)))
            {
                return tur_dugum(dugum_satir(e), dugum_sutun(e), liste_elem_ad(dugum_ad(inner)), 0LL, 0LL);
            }
            if ((dugum_dims(inner) > 0LL))
            {
                return tur_dugum(dugum_satir(inner), dugum_sutun(inner), dugum_ad(inner), dugum_ptrd(inner), (dugum_dims(inner) - 1LL));
            }
            if ((dugum_ptrd(inner) > 0LL))
            {
                return tur_dugum(dugum_satir(inner), dugum_sutun(inner), dugum_ad(inner), (dugum_ptrd(inner) - 1LL), 0LL);
            }
            if (tip_metin_mi(inner))
            {
                return tip_sayi(dugum_satir(inner), dugum_sutun(inner));
            }
            hata_ver(dugum_satir(e), dugum_sutun(e), "Dizinlenemez.");
            return inner;
        }
        if ((t == DugumTur_D_CAGRI))
        {
            return tip_cagri(e);
        }
        if ((t == DugumTur_D_ATAMA))
        {
            if ((!lvalue_mi(dugum_a(e))))
            {
                hata_ver(dugum_satir(e), dugum_sutun(e), "Atama hedefi bir lvalue olmalı.");
            }
            if ((dugum_tur(dugum_a(e)) == DugumTur_D_KIMLIK))
            {
                char* nm = dugum_ad(dugum_a(e));
                long long gt = kapsam_bul(nm);
                if ((gt == 0LL))
                {
                    (gt = tablo_bul(3LL, nm));
                }
                if (((gt != 0LL) && ((dugum_bayrak(gt) & 16LL) != 0LL)))
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("'");
                    hata_fmt_yaz(nm);
                    hata_fmt_yaz("' sabittir; yeniden atanamaz.");
                    hata_fmt_ver(dugum_satir(e), dugum_sutun(e));
                }
            }
            long long tt = tip_ifade(dugum_a(e));
            long long vt = tip_ifade(dugum_b(e));
            char* aop = dugum_ad(e);
            if (((!metin_esit(aop, "")) && (!metin_esit(aop, "="))))
            {
                if (((metin_esit(aop, "+=") && tip_metin_mi(tt)) && (tip_metin_mi(vt) || tip_yok_mi(vt))))
                {
                    return tt;
                }
                if ((tip_metin_mi(tt) || tip_metin_mi(vt)))
                {
                    hata_ver(dugum_satir(e), dugum_sutun(e), "Bileşik atama metin üzerinde kullanılamaz.");
                }
            }
            if ((!tip_atanir(tt, vt)))
            {
                hata_ver(dugum_satir(e), dugum_sutun(e), "Atama türü uyumsuz.");
            }
            return tt;
        }
        if ((t == DugumTur_D_UCLU))
        {
            tip_ifade(dugum_a(e));
            long long tt = tip_ifade(dugum_b(e));
            long long et = tip_ifade(dugum_c(e));
            if ((tip_void_mi(tt) || tip_void_mi(et)))
            {
                hata_ver(dugum_satir(e), dugum_sutun(e), "'?:' dalları 'bos' olamaz.");
                return tt;
            }
            if (tip_atanir(tt, et))
            {
                return tt;
            }
            if (tip_atanir(et, tt))
            {
                return et;
            }
            hata_ver(dugum_satir(e), dugum_sutun(e), "'?:' dal türleri uyumsuz.");
            return tt;
        }
        if ((t == DugumTur_D_TEKIL))
        {
            long long inner = tip_ifade(dugum_a(e));
            char* op = dugum_ad(e);
            if (metin_esit(op, "&"))
            {
                return tur_dugum(dugum_satir(inner), dugum_sutun(inner), dugum_ad(inner), (dugum_ptrd(inner) + 1LL), dugum_dims(inner));
            }
            if (metin_esit(op, "*"))
            {
                if ((dugum_ptrd(inner) <= 0LL))
                {
                    hata_ver(dugum_satir(e), dugum_sutun(e), "İşaretçi değil; '*' uygulanamaz.");
                    return inner;
                }
                return tur_dugum(dugum_satir(inner), dugum_sutun(inner), dugum_ad(inner), (dugum_ptrd(inner) - 1LL), 0LL);
            }
            if (metin_esit(op, "!"))
            {
                return tip_mantik(dugum_satir(e), dugum_sutun(e));
            }
            return inner;
        }
        if ((t == DugumTur_D_IKIL))
        {
            return tip_ikil(e);
        }
        if ((t == DugumTur_D_DIZI_SABIT))
        {
            long long L = dugum_liste(e);
            if ((liste_uzunluk(L) == 0LL))
            {
                return tur_dugum(dugum_satir(e), dugum_sutun(e), "sayi", 0LL, 1LL);
            }
            long long inner = tip_ifade(liste_al(L, 0LL));
            if ((dugum_dims(inner) > 0LL))
            {
                hata_ver(dugum_satir(e), dugum_sutun(e), "İç içe dizi sabiti yok; tek boyut veya 'liste[T]' kullanın.");
                return tur_dugum(dugum_satir(e), dugum_sutun(e), "sayi", 0LL, 1LL);
            }
            return tur_dugum(dugum_satir(inner), dugum_sutun(inner), dugum_ad(inner), dugum_ptrd(inner), (dugum_dims(inner) + 1LL));
        }
        if ((t == DugumTur_D_ARKAPLAN_IFADE))
        {
            bg_yaz((bg_al() + 1LL));
            tip_yuru(dugum_a(e));
            bg_yaz((bg_al() - 1LL));
            if ((!hata_var()))
            {
                bg_yakala(e);
            }
            return tur_dugum(dugum_satir(e), dugum_sutun(e), "ArkaplanIs", 0LL, 0LL);
        }
        return tip_bos(dugum_satir(e), dugum_sutun(e));
    }
}

bool sabit_ifade_mi(long long e) {
    {
        if ((e == 0LL))
        {
            return false;
        }
        long long t = dugum_tur(e);
        if ((t == DugumTur_D_SABIT))
        {
            return true;
        }
        if ((t == DugumTur_D_TEKIL))
        {
            char* op = dugum_ad(e);
            if ((metin_esit(op, "+") || metin_esit(op, "-")))
            {
                return sabit_ifade_mi(dugum_a(e));
            }
            return false;
        }
        if ((t == DugumTur_D_IKIL))
        {
            return (sabit_ifade_mi(dugum_a(e)) && sabit_ifade_mi(dugum_b(e)));
        }
        if ((t == DugumTur_D_UCLU))
        {
            return ((sabit_ifade_mi(dugum_a(e)) && sabit_ifade_mi(dugum_b(e))) && sabit_ifade_mi(dugum_c(e)));
        }
        if ((t == DugumTur_D_KIMLIK))
        {
            long long gt = kapsam_bul(dugum_ad(e));
            if ((gt == 0LL))
            {
                (gt = tablo_bul(3LL, dugum_ad(e)));
            }
            return ((gt != 0LL) && ((dugum_bayrak(gt) & 16LL) != 0LL));
        }
        if ((t == DugumTur_D_UYE))
        {
            long long obj_t = tip_ifade(dugum_a(e));
            if (((metin_uzun(dugum_ad(obj_t)) >= 8LL) && metin_esit(intern_dilim(dugum_ad(obj_t), 0LL, 8LL), "secenek.")))
            {
                return true;
            }
            return false;
        }
        return false;
    }
}

void kapsam_bagla_yeni(char* ad, long long ty, long long satir, long long sutun) {
    {
        if (kapsam_yerel_var(ad))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'");
            hata_fmt_yaz(ad);
            hata_fmt_yaz("' bu kapsamda birden fazla kez tanımlandı.");
            hata_fmt_ver(satir, sutun);
            return;
        }
        kapsam_bagla(ad, ty);
    }
}

void tip_yuru(long long stmt) {
    {
        if (((stmt == 0LL) || hata_var()))
        {
            return;
        }
        long long t = dugum_tur(stmt);
        if ((t == DugumTur_D_BLOK))
        {
            kapsam_ac();
            long long L = dugum_liste(stmt);
            long long i = 0LL;
            while ((i < liste_uzunluk(L)))
            {
                tip_yuru(liste_al(L, i));
                (i = (i + 1LL));
            }
            kapsam_kapat();
            return;
        }
        if (((t == DugumTur_D_GUVENSIZ) || (t == DugumTur_D_GUVENLI)))
        {
            tip_yuru(dugum_a(stmt));
            return;
        }
        if ((t == DugumTur_D_ARKAPLAN))
        {
            bg_yaz((bg_al() + 1LL));
            tip_yuru(dugum_a(stmt));
            bg_yaz((bg_al() - 1LL));
            if ((!hata_var()))
            {
                bg_yakala(stmt);
            }
            return;
        }
        if ((t == DugumTur_D_DEGISKEN))
        {
            long long init_t = 0LL;
            if ((dugum_c(stmt) != 0LL))
            {
                (init_t = tip_ifade(dugum_c(stmt)));
                if (((dugum_bayrak(stmt) & 1LL) != 0LL))
                {
                    if ((dugum_tur(dugum_c(stmt)) == DugumTur_D_DIZI_SABIT))
                    {
                        long long dd = dugum_dims(init_t);
                        if ((dd < 1LL))
                        {
                            (dd = 1LL);
                        }
                        dugum_yaz_dims(init_t, dd);
                    }
                    dugum_yaz_a(stmt, init_t);
                }
            }
            long long bound = dugum_a(stmt);
            if (((dugum_b(stmt) != 0LL) || ((dugum_bayrak(stmt) & 8LL) != 0LL)))
            {
                (bound = tur_dugum(dugum_satir(stmt), dugum_sutun(stmt), dugum_ad(dugum_a(stmt)), dugum_ptrd(dugum_a(stmt)), 1LL));
                dugum_yaz_a(stmt, bound);
            }
            long long alen = 0LL;
            if (((dugum_c(stmt) != 0LL) && (dugum_tur(dugum_c(stmt)) == DugumTur_D_DIZI_SABIT)))
            {
                (alen = liste_uzunluk(dugum_liste(dugum_c(stmt))));
            }
            else
                if (((((dugum_b(stmt) != 0LL) && (dugum_tur(dugum_b(stmt)) == DugumTur_D_SABIT)) && (dugum_ptrd(dugum_b(stmt)) == 0LL)) && (dugum_sayi(dugum_b(stmt)) > 0LL)))
                {
                    (alen = dugum_sayi(dugum_b(stmt)));
                }
            if (((alen > 0LL) && (bound != 0LL)))
            {
                dugum_yaz_sayi(bound, alen);
            }
            if (((dugum_bayrak(stmt) & 16LL) != 0LL))
            {
                if (((dugum_c(stmt) == 0LL) || (!sabit_ifade_mi(dugum_c(stmt)))))
                {
                    hata_ver(dugum_satir(stmt), dugum_sutun(stmt), "sabit derleme zamanı ifadesi ile başlatılmalıdır.");
                }
                if ((bound != 0LL))
                {
                    dugum_yaz_bayrak(bound, (dugum_bayrak(bound) | 16LL));
                }
            }
            if ((((init_t != 0LL) && ((dugum_bayrak(stmt) & 1LL) == 0LL)) && (bound != 0LL)))
            {
                if ((!tip_atanir(bound, init_t)))
                {
                    hata_ver(dugum_satir(stmt), dugum_sutun(stmt), "Atama türü uyumsuz.");
                }
            }
            kapsam_bagla_yeni(dugum_ad(stmt), bound, dugum_satir(stmt), dugum_sutun(stmt));
            return;
        }
        if ((t == DugumTur_D_EGER))
        {
            tip_ifade(dugum_a(stmt));
            tip_yuru(dugum_b(stmt));
            tip_yuru(dugum_c(stmt));
            return;
        }
        if ((t == DugumTur_D_IKEN))
        {
            tip_ifade(dugum_a(stmt));
            tip_yuru(dugum_b(stmt));
            return;
        }
        if ((t == DugumTur_D_DONGU))
        {
            kapsam_ac();
            tip_yuru(dugum_a(stmt));
            tip_ifade(dugum_b(stmt));
            tip_ifade(dugum_c(stmt));
            tip_yuru(dugum_d(stmt));
            kapsam_kapat();
            return;
        }
        if ((t == DugumTur_D_SECIM))
        {
            long long sw_t = tip_ifade(dugum_a(stmt));
            long long L = dugum_liste(stmt);
            long long i = 0LL;
            if (hata_var())
            {
                return;
            }
            if ((!secim_uygun_tur(sw_t)))
            {
                hata_ver(dugum_satir(stmt), dugum_sutun(stmt), "'secim' tam sayı, metin veya seçenek bekler.");
                return;
            }
            while ((i < liste_uzunluk(L)))
            {
                long long c = liste_al(L, i);
                if ((dugum_a(c) != 0LL))
                {
                    long long val = dugum_a(c);
                    long long ct = tip_ifade(val);
                    if (hata_var())
                    {
                        return;
                    }
                    if ((!secim_sabit_mi(val)))
                    {
                        hata_ver(dugum_satir(c), dugum_sutun(c), "'durum' değeri derleme zamanı sabiti (sayı, metin veya seçenek üyesi) olmalıdır.");
                        return;
                    }
                    if (((!tip_atanir(sw_t, ct)) && (!tip_atanir(ct, sw_t))))
                    {
                        hata_ver(dugum_satir(c), dugum_sutun(c), "'durum' türü uyumsuz.");
                        return;
                    }
                }
                long long sl = dugum_liste(c);
                long long j = 0LL;
                while ((j < liste_uzunluk(sl)))
                {
                    tip_yuru(liste_al(sl, j));
                    (j = (j + 1LL));
                }
                (i = (i + 1LL));
            }
            return;
        }
        if ((t == DugumTur_D_HER))
        {
            long long it_t = tip_ifade(dugum_b(stmt));
            long long elem = 0LL;
            if ((liste_tur_mi(dugum_ad(it_t)) && (dugum_ptrd(it_t) == 0LL)))
            {
                (elem = tur_dugum(dugum_satir(stmt), dugum_sutun(stmt), liste_elem_ad(dugum_ad(it_t)), 0LL, 0LL));
            }
            else
                if ((harita_tur_mi(dugum_ad(it_t)) && (dugum_ptrd(it_t) == 0LL)))
                {
                    (elem = tur_dugum(dugum_satir(stmt), dugum_sutun(stmt), harita_anahtar_ad(dugum_ad(it_t)), 0LL, 0LL));
                }
                else
                {
                    if (((dugum_dims(it_t) < 1LL) && (dugum_tur(dugum_b(stmt)) != DugumTur_D_DIZI_SABIT)))
                    {
                        hata_ver(dugum_satir(stmt), dugum_sutun(stmt), "'her' gerçek bir dizi veya liste bekler.");
                    }
                    if ((dugum_dims(it_t) > 1LL))
                    {
                        hata_ver(dugum_satir(stmt), dugum_sutun(stmt), "'her' tek boyutlu dizi bekler; iç içe '[...]' desteklenmez.");
                    }
                    long long edim = dugum_dims(it_t);
                    if ((edim > 0LL))
                    {
                        (edim = (edim - 1LL));
                    }
                    (elem = tur_dugum(dugum_satir(stmt), dugum_sutun(stmt), dugum_ad(it_t), dugum_ptrd(it_t), edim));
                }
            if (((dugum_bayrak(stmt) & 1LL) != 0LL))
            {
                dugum_yaz_a(stmt, elem);
            }
            kapsam_ac();
            kapsam_bagla_yeni(dugum_ad(stmt), dugum_a(stmt), dugum_satir(stmt), dugum_sutun(stmt));
            tip_yuru(dugum_c(stmt));
            kapsam_kapat();
            return;
        }
        if ((t == DugumTur_D_DON))
        {
            long long got = tip_bos(dugum_satir(stmt), dugum_sutun(stmt));
            long long fn = 0LL;
            long long expect = 0LL;
            if ((dugum_a(stmt) != 0LL))
            {
                (got = tip_ifade(dugum_a(stmt)));
            }
            if (hata_var())
            {
                return;
            }
            if ((bg_al() > 0LL))
            {
                if (((dugum_a(stmt) != 0LL) && (!tip_void_mi(got))))
                {
                    hata_ver(dugum_satir(stmt), dugum_sutun(stmt), "'arkaplan' bloğunda 'don' değer alamaz; iş parçacığını bitirmek için 'don;' yazın.");
                }
                return;
            }
            (fn = cur_fn_al());
            if ((fn == 0LL))
            {
                return;
            }
            (expect = dugum_a(fn));
            if (tip_void_mi(expect))
            {
                if (((dugum_a(stmt) != 0LL) && (!tip_void_mi(got))))
                {
                    char* bul = tur_aciklama(got);
                    hata_fmt_basla();
                    hata_fmt_yaz("'bos' dönen fonksiyonda değer döndürülemez (ifade türü '");
                    hata_fmt_yaz(bul);
                    hata_fmt_yaz("').");
                    hata_fmt_ver(dugum_satir(stmt), dugum_sutun(stmt));
                }
                return;
            }
            if (((dugum_a(stmt) == 0LL) || tip_void_mi(got)))
            {
                char* bek = tur_aciklama(expect);
                hata_fmt_basla();
                hata_fmt_yaz("'");
                hata_fmt_yaz(bek);
                hata_fmt_yaz("' dönen fonksiyonda 'don' bir değer gerektirir.");
                hata_fmt_ver(dugum_satir(stmt), dugum_sutun(stmt));
                return;
            }
            if ((!tip_atanir(expect, got)))
            {
                char* bek = tur_aciklama(expect);
                char* bul = tur_aciklama(got);
                hata_fmt_basla();
                hata_fmt_yaz("Dönüş türü uyumsuz: beklenen '");
                hata_fmt_yaz(bek);
                hata_fmt_yaz("', bulunan '");
                hata_fmt_yaz(bul);
                hata_fmt_yaz("'.");
                hata_fmt_ver(dugum_satir(stmt), dugum_sutun(stmt));
            }
            return;
        }
        if ((t == DugumTur_D_IFADE_CUMLE))
        {
            tip_ifade(dugum_a(stmt));
            return;
        }
    }
}

void tip_fn_doldur(long long fn) {
    {
        long long P = dugum_liste(fn);
        long long i = 0LL;
        while ((i < liste_uzunluk(P)))
        {
            long long p = liste_al(P, i);
            if (((dugum_a(p) == 0LL) || metin_esit(dugum_ad(dugum_a(p)), "")))
            {
                dugum_yaz_a(p, tip_sayi(dugum_satir(fn), dugum_sutun(fn)));
            }
            if (((dugum_c(p) != 0LL) && (!sabit_ifade_mi(dugum_c(p)))))
            {
                hata_ver(dugum_satir(p), dugum_sutun(p), "Varsayılan derleme sabiti olmalı ve parametreye bakamaz.");
            }
            (i = (i + 1LL));
        }
    }
}

void tip_fn_govde(long long fn) {
    {
        kapsam_sifirla();
        kapsam_ac();
        cur_fn_yaz(fn);
        if ((dugum_c(fn) != 0LL))
        {
            char* stad = metot_sinif_tur_ad(fn);
            cur_class_yaz(stad);
            kapsam_bagla_yeni("kendi", tur_dugum(dugum_satir(fn), dugum_sutun(fn), stad, 1LL, 0LL), dugum_satir(fn), dugum_sutun(fn));
        }
        else
        {
            cur_class_yaz("");
        }
        long long P = dugum_liste(fn);
        long long i = 0LL;
        while ((i < liste_uzunluk(P)))
        {
            long long p = liste_al(P, i);
            kapsam_bagla_yeni(dugum_ad(p), dugum_a(p), dugum_satir(p), dugum_sutun(p));
            (i = (i + 1LL));
        }
        tip_yuru(dugum_b(fn));
        if ((!hata_var()))
        {
            long long rt = dugum_a(fn);
            if ((!tip_void_mi(rt)))
            {
                if ((!her_zaman_doner(dugum_b(fn))))
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("'");
                    hata_fmt_yaz(dugum_ad(fn));
                    hata_fmt_yaz("' dönüş değeri olmayan yollara sahip.");
                    hata_fmt_ver(dugum_satir(fn), dugum_sutun(fn));
                }
            }
        }
        cur_fn_yaz(0LL);
        cur_class_yaz("");
    }
}

void tip_calistir(long long program) {
    {
        tablo_sifirla();
        tip_yerlesik_sinif("Yonelme", "pitch", "ondalik", "yaw", "ondalik", "", "");
        tip_yerlesik_sinif("EkranNokta", "x", "ondalik", "y", "ondalik", "gorunur", "sayi");
        tip_yerlesik_sinif("Vektor3", "x", "ondalik", "y", "ondalik", "z", "ondalik");
        tip_yerlesik_sinif("ArkaplanIs", "handle", "sayi", "", "", "", "");
        tip_yerlesik_sinif("JSONDeger", "id", "sayi", "", "", "", "");
        tip_yerlesik_sinif("SurecCikti", "cikti", "metin", "kod", "sayi", "basarili", "mantik");
        tip_yerlesik_sinif("Kilit", "id", "sayi", "", "", "", "");
        long long decls = dugum_liste(program);
        long long i = 0LL;
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            long long t = dugum_tur(d);
            if ((t == DugumTur_D_SECENEK))
            {
                char* ekey = sinif_tur_ad(d);
                char* ealias = dugum_alias_ad(d);
                if (((!metin_esit(ealias, "")) && (tablo_bul(3LL, intern(kat("modul.", ealias))) == 0LL)))
                {
                    tablo_yaz(3LL, intern(kat("modul.", ealias)), tur_dugum(dugum_satir(d), dugum_sutun(d), intern(kat("modul.", ealias)), 0LL, 0LL));
                }
                if ((((tablo_bul(2LL, ekey) != 0LL) || (tablo_bul(0LL, ekey) != 0LL)) || tur_adi_mi(dugum_ad(d))))
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("'");
                    hata_fmt_yaz(dugum_ad(d));
                    hata_fmt_yaz("' zaten tanımlı; seçenek olarak kullanılamaz.");
                    hata_fmt_ver(dugum_satir(d), dugum_sutun(d));
                }
                else
                {
                    tablo_yaz(2LL, ekey, d);
                }
            }
            else
                if ((t == DugumTur_D_SINIF))
                {
                    char* skey = sinif_tur_ad(d);
                    char* alias = dugum_alias_ad(d);
                    if (((!metin_esit(alias, "")) && (tablo_bul(3LL, intern(kat("modul.", alias))) == 0LL)))
                    {
                        tablo_yaz(3LL, intern(kat("modul.", alias)), tur_dugum(dugum_satir(d), dugum_sutun(d), intern(kat("modul.", alias)), 0LL, 0LL));
                    }
                    if ((yerlesik_sinif_mi(dugum_ad(d)) && metin_esit(alias, "")))
                    {
                        hata_fmt_basla();
                        hata_fmt_yaz("'");
                        hata_fmt_yaz(dugum_ad(d));
                        hata_fmt_yaz("' yerleşik bir türdür; yeniden tanımlanamaz.");
                        hata_fmt_ver(dugum_satir(d), dugum_sutun(d));
                    }
                    else
                        if ((tablo_bul(2LL, dugum_ad(d)) != 0LL))
                        {
                            hata_fmt_basla();
                            hata_fmt_yaz("'");
                            hata_fmt_yaz(dugum_ad(d));
                            hata_fmt_yaz("' seçenek adı olarak tanımlı; sınıf olamaz.");
                            hata_fmt_ver(dugum_satir(d), dugum_sutun(d));
                        }
                        else
                            if ((tablo_bul(0LL, skey) != 0LL))
                            {
                                cift_tanim_ver(tablo_bul(0LL, skey), d, skey);
                            }
                            else
                            {
                                tablo_yaz(0LL, skey, d);
                                long long M = dugum_liste2(d);
                                long long j = 0LL;
                                while ((j < liste_uzunluk(M)))
                                {
                                    long long m = liste_al(M, j);
                                    char* anahtar = kat3(nokta_altcizgi(skey), "_", dugum_ad(m));
                                    if ((tablo_bul(1LL, anahtar) != 0LL))
                                    {
                                        cift_tanim_ver(tablo_bul(1LL, anahtar), m, anahtar);
                                    }
                                    tablo_yaz(1LL, anahtar, m);
                                    (j = (j + 1LL));
                                }
                            }
                }
                else
                    if ((t == DugumTur_D_FONK))
                    {
                        char* fkey = dugum_c_ad(d);
                        char* falias = dugum_alias_ad(d);
                        if (((!metin_esit(falias, "")) && (tablo_bul(3LL, intern(kat("modul.", falias))) == 0LL)))
                        {
                            tablo_yaz(3LL, intern(kat("modul.", falias)), tur_dugum(dugum_satir(d), dugum_sutun(d), intern(kat("modul.", falias)), 0LL, 0LL));
                        }
                        if ((tablo_bul(1LL, fkey) != 0LL))
                        {
                            cift_tanim_ver(tablo_bul(1LL, fkey), d, fkey);
                        }
                        tablo_yaz(1LL, fkey, d);
                    }
            (i = (i + 1LL));
        }
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_FONK))
            {
                tip_fn_doldur(d);
            }
            if ((dugum_tur(d) == DugumTur_D_SINIF))
            {
                long long M = dugum_liste2(d);
                long long j = 0LL;
                while ((j < liste_uzunluk(M)))
                {
                    tip_fn_doldur(liste_al(M, j));
                    (j = (j + 1LL));
                }
            }
            (i = (i + 1LL));
        }
        kapsam_sifirla();
        kapsam_ac();
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_DEGISKEN))
            {
                char* vkey = dugum_c_ad(d);
                char* valias = dugum_alias_ad(d);
                if (((!metin_esit(valias, "")) && (tablo_bul(3LL, intern(kat("modul.", valias))) == 0LL)))
                {
                    tablo_yaz(3LL, intern(kat("modul.", valias)), tur_dugum(dugum_satir(d), dugum_sutun(d), intern(kat("modul.", valias)), 0LL, 0LL));
                }
                if (((kapsam_bul(vkey) != 0LL) || (tablo_bul(3LL, vkey) != 0LL)))
                {
                    long long eski = kapsam_bul(vkey);
                    if ((eski == 0LL))
                    {
                        (eski = tablo_bul(3LL, vkey));
                    }
                    cift_tanim_ver(eski, d, vkey);
                }
                if ((dugum_ptrd(dugum_a(d)) > 0LL))
                {
                    hata_ver(dugum_satir(d), dugum_sutun(d), "Üst düzeyde işaretçi değişken yok.");
                }
                if ((((dugum_b(d) != 0LL) || ((dugum_bayrak(d) & 8LL) != 0LL)) || (dugum_dims(dugum_a(d)) > 0LL)))
                {
                    hata_ver(dugum_satir(d), dugum_sutun(d), "Üst düzeyde dizi yok; 'liste[T]' kullanın.");
                }
                if (((((dugum_bayrak(d) & 16LL) == 0LL) && (dugum_c(d) != 0LL)) && (!sabit_ifade_mi(dugum_c(d)))))
                {
                    hata_ver(dugum_satir(d), dugum_sutun(d), "Üst düzey değişken derleme sabiti veya sıfır ile başlatılmalıdır.");
                }
                if ((dugum_c(d) != 0LL))
                {
                    long long got = tip_ifade(dugum_c(d));
                    if ((!tip_atanir(dugum_a(d), got)))
                    {
                        hata_ver(dugum_satir(d), dugum_sutun(d), "Atama türü uyumsuz.");
                    }
                }
                long long bound = dugum_a(d);
                if ((((dugum_bayrak(d) & 16LL) != 0LL) && (bound != 0LL)))
                {
                    dugum_yaz_bayrak(bound, (dugum_bayrak(bound) | 16LL));
                }
                kapsam_bagla_yeni(vkey, bound, dugum_satir(d), dugum_sutun(d));
                if ((bound != 0LL))
                {
                    tablo_yaz(3LL, vkey, bound);
                }
            }
            (i = (i + 1LL));
        }
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_FONK))
            {
                tip_fn_govde(d);
            }
            if ((dugum_tur(d) == DugumTur_D_SINIF))
            {
                long long M = dugum_liste2(d);
                long long j = 0LL;
                while ((j < liste_uzunluk(M)))
                {
                    tip_fn_govde(liste_al(M, j));
                    (j = (j + 1LL));
                }
            }
            (i = (i + 1LL));
        }
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if (((dugum_tur(d) == DugumTur_D_FONK) && metin_esit(dugum_ad(d), "ana")))
            {
                long long rt = dugum_a(d);
                if (((!(metin_esit(dugum_ad(rt), "sayi") || metin_esit(dugum_ad(rt), "bos"))) || (dugum_ptrd(rt) != 0LL)))
                {
                    hata_ver(dugum_satir(d), dugum_sutun(d), "'ana' dönüş türü 'sayi' veya 'bos' olmalı.");
                }
                if ((liste_uzunluk(dugum_liste(d)) != 0LL))
                {
                    hata_ver(dugum_satir(d), dugum_sutun(d), "'ana' parametre alamaz.");
                }
            }
            (i = (i + 1LL));
        }
    }
}

void guven_yuru(long long node, bool unsafe) {
    {
        if ((node == 0LL))
        {
            return;
        }
        if ((hata_var() && (!hata_analiz_mi())))
        {
            return;
        }
        long long t = dugum_tur(node);
        if ((t == DugumTur_D_GUVENSIZ))
        {
            guven_yuru(dugum_a(node), true);
            return;
        }
        if ((t == DugumTur_D_GUVENLI))
        {
            guven_yuru(dugum_a(node), false);
            return;
        }
        if (((t == DugumTur_D_ARKAPLAN) || (t == DugumTur_D_ARKAPLAN_IFADE)))
        {
            guven_yuru(dugum_a(node), unsafe);
            return;
        }
        if ((t == DugumTur_D_C_GOMMU))
        {
            if ((!unsafe))
            {
                hata_guvenlik(dugum_satir(node), dugum_sutun(node), "Ham C gömme ('c_kod') yalnızca 'guvensiz' blokta kullanılabilir.");
                return;
            }
            if (c_on_islemci(dugum_ad(node)))
            {
                hata_ver(dugum_satir(node), dugum_sutun(node), "c_kod içinde '#include' / '#define' kullanılamaz. Başlık eklemek için dosya başında c_ekle yazın.");
            }
            return;
        }
        if ((t == DugumTur_D_ASM_GOMMU))
        {
            if ((!unsafe))
            {
                hata_guvenlik(dugum_satir(node), dugum_sutun(node), "Ham asm gömme ('asm_kod') yalnızca 'guvensiz' blokta kullanılabilir.");
            }
            return;
        }
        if ((t == DugumTur_D_TEKIL))
        {
            char* op = dugum_ad(node);
            if (((metin_esit(op, "*") || metin_esit(op, "&")) && (!unsafe)))
            {
                hata_guvenlik(dugum_satir(node), dugum_sutun(node), "Doğrudan işaretçi işlemi 'guvenli' alanda yasaktır.");
            }
        }
        if ((t == DugumTur_D_CAGRI))
        {
            long long cal = dugum_a(node);
            if ((dugum_tur(cal) == DugumTur_D_KIMLIK))
            {
                char* nm = dugum_ad(cal);
                if (((metin_esit(nm, "bellek_ayir") || metin_esit(nm, "bellek_bosalt")) && (!unsafe)))
                {
                    hata_guvenlik(dugum_satir(node), dugum_sutun(node), "Doğrudan bellek manipülasyonu yalnızca 'guvensiz' blokta çağrılabilir.");
                }
            }
            if ((dugum_tur(cal) == DugumTur_D_UYE))
            {
                long long yol = upp_yol(cal);
                if (((yol != 0LL) && (liste_uzunluk(yol) >= 2LL)))
                {
                    long long yi = 1LL;
                    while ((yi < liste_uzunluk(yol)))
                    {
                        long long adim = liste_al(yol, yi);
                        char* anm = dugum_ad(adim);
                        if ((((metin_esit(anm, "bellek") || metin_esit(anm, "sayi_oku")) || metin_esit(anm, "sayi_yaz")) && (!unsafe)))
                        {
                            hata_guvenlik(dugum_satir(node), dugum_sutun(node), "Süreç belleği erişimi ('upp.windows.bellek' / 'upp.linux.sayi_oku/yaz') yalnızca 'guvensiz' blokta çağrılabilir.");
                            break;
                        }
                        (yi = (yi + 1LL));
                    }
                }
            }
        }
        if ((t == DugumTur_D_DEGISKEN))
        {
            long long ty = dugum_a(node);
            if ((((ty != 0LL) && (dugum_ptrd(ty) > 0LL)) && (!unsafe)))
            {
                hata_guvenlik(dugum_satir(node), dugum_sutun(node), "İşaretçi türü yalnızca 'guvensiz' blokta bildirilebilir.");
            }
        }
        if ((t == DugumTur_D_SINIF))
        {
            long long F = dugum_liste(node);
            long long i = 0LL;
            while ((i < liste_uzunluk(F)))
            {
                long long f = liste_al(F, i);
                long long ty = dugum_a(f);
                if (((ty != 0LL) && (dugum_ptrd(ty) > 0LL)))
                {
                    hata_guvenlik(dugum_satir(f), dugum_sutun(f), "Ham işaretçi alanı sınıf içinde kullanılamaz.");
                }
                (i = (i + 1LL));
            }
            long long M = dugum_liste2(node);
            (i = 0LL);
            while ((i < liste_uzunluk(M)))
            {
                guven_yuru(dugum_b(liste_al(M, i)), false);
                (i = (i + 1LL));
            }
            return;
        }
        if ((t == DugumTur_D_FONK))
        {
            guven_yuru(dugum_b(node), false);
            return;
        }
        if ((t == DugumTur_D_PROGRAM))
        {
            long long L = dugum_liste(node);
            long long i = 0LL;
            while ((i < liste_uzunluk(L)))
            {
                guven_yuru(liste_al(L, i), false);
                (i = (i + 1LL));
            }
            return;
        }
        guven_yuru(dugum_a(node), unsafe);
        if ((t != DugumTur_D_UYE))
        {
            guven_yuru(dugum_b(node), unsafe);
            guven_yuru(dugum_c(node), unsafe);
            if ((t == DugumTur_D_DONGU))
            {
                guven_yuru(dugum_d(node), unsafe);
            }
        }
        long long L = dugum_liste(node);
        if ((L != 0LL))
        {
            long long i = 0LL;
            while ((i < liste_uzunluk(L)))
            {
                guven_yuru(liste_al(L, i), unsafe);
                (i = (i + 1LL));
            }
        }
        long long L2 = dugum_liste2(node);
        if ((L2 != 0LL))
        {
            long long i = 0LL;
            while ((i < liste_uzunluk(L2)))
            {
                guven_yuru(liste_al(L2, i), unsafe);
                (i = (i + 1LL));
            }
        }
    }
}

void guven_kontrol(long long program) {
    {
        guven_yuru(program, false);
    }
}

char* uret_ctur(long long t) {
    {
        if ((t == 0LL))
        {
            return "void";
        }
        char* n = dugum_ad(t);
        char* s = n;
        if (metin_esit(n, "sayi"))
        {
            (s = "long long");
        }
        else
            if (metin_esit(n, "bayt"))
            {
                (s = "uint8_t");
            }
            else
                if (metin_esit(n, "ondalik"))
                {
                    (s = "double");
                }
                else
                    if (metin_esit(n, "metin"))
                    {
                        (s = "char*");
                    }
                    else
                        if (metin_esit(n, "mantik"))
                        {
                            (s = "bool");
                        }
                        else
                            if (metin_esit(n, "bos"))
                            {
                                (s = "void");
                            }
                            else
                                if (liste_tur_mi(n))
                                {
                                    (s = "UppKolListe");
                                }
                                else
                                    if (harita_tur_mi(n))
                                    {
                                        (s = "UppKolHarita");
                                    }
                                    else
                                    {
                                        (s = nokta_altcizgi(n));
                                    }
        long long i = 0LL;
        while ((i < dugum_ptrd(t)))
        {
            (s = kat(s, "*"));
            (i = (i + 1LL));
        }
        return s;
    }
}

char* uret_args_fn(long long fn, long long args) {
    {
        long long P = dugum_liste(fn);
        long long nneed = liste_uzunluk(P);
        long long ngot = liste_uzunluk(args);
        char* s = upp_metin_kopya("");
        long long i = 0LL;
        while ((i < nneed))
        {
            if ((i > 0LL))
            {
                (s = kat(s, ", "));
            }
            if ((i < ngot))
            {
                (s = kat(s, uret_ifade(liste_al(args, i))));
            }
            else
            {
                (s = kat(s, uret_ifade(dugum_c(liste_al(P, i)))));
            }
            (i = (i + 1LL));
        }
        return s;
    }
}

char* uret_kurucu(long long cls, long long args) {
    {
        long long M = dugum_liste2(cls);
        long long ol = 0LL;
        long long i = 0LL;
        while ((i < liste_uzunluk(M)))
        {
            if (metin_esit(dugum_ad(liste_al(M, i)), "olustur"))
            {
                (ol = liste_al(M, i));
            }
            (i = (i + 1LL));
        }
        char* cn = dugum_c_ad(cls);
        long long id = tmp_yeni();
        char* tn = kat("_upp_ct", sayi_metin(id));
        char* s = upp_metin_kopya("({ ");
        (s = kat(s, cn));
        (s = kat(s, " "));
        (s = kat(s, tn));
        (s = kat(s, " = {0}; "));
        (s = kat(s, cn));
        (s = kat(s, "_olustur(&"));
        (s = kat(s, tn));
        if (((ol != 0LL) && (liste_uzunluk(dugum_liste(ol)) > 0LL)))
        {
            (s = kat(s, ", "));
            (s = kat(s, uret_args_fn(ol, args)));
        }
        (s = kat(s, "); "));
        (s = kat(s, tn));
        (s = kat(s, "; })"));
        return s;
    }
}

char* uret_args(long long L) {
    {
        char* s = upp_metin_kopya("");
        long long i = 0LL;
        while ((i < liste_uzunluk(L)))
        {
            if ((i > 0LL))
            {
                (s = kat(s, ", "));
            }
            (s = kat(s, uret_ifade(liste_al(L, i))));
            (i = (i + 1LL));
        }
        return s;
    }
}

char* uret_sabit(long long e) {
    {
        long long k = dugum_ptrd(e);
        if ((k == 0LL))
        {
            return kat(sayi_metin(dugum_sayi(e)), "LL");
        }
        if ((k == 1LL))
        {
            return dugum_ad(e);
        }
        if ((k == 2LL))
        {
            return uret_metin_sabit(e);
        }
        if ((k == 3LL))
        {
            if ((dugum_sayi(e) != 0LL))
            {
                return "true";
            }
            return "false";
        }
        return "NULL";
    }
}

bool interp_var_mi(char* text) {
    {
        long long n = metin_uzun(text);
        long long i = 0LL;
        while ((i < n))
        {
            long long c = metin_cp(text, i);
            long long adv = metin_adv(text, i);
            if ((((c == 123LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 123LL)))
            {
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((c == 123LL))
            {
                long long j = (i + adv);
                while (((j < n) && (metin_cp(text, j) != 125LL)))
                {
                    (j = (j + metin_adv(text, j)));
                }
                if ((j >= n))
                {
                    return false;
                }
                char* name = intern_dilim(text, (i + adv), j);
                if (interp_ad_mi(name))
                {
                    return true;
                }
                (i = (j + 1LL));
                continue;
            }
            (i = (i + adv));
        }
        return false;
    }
}

char* yuzde_kacis(char* s) {
    {
        char* o = upp_metin_kopya("");
        long long i = 0LL;
        long long n = metin_uzun(s);
        while ((i < n))
        {
            long long adv = metin_adv(s, i);
            if ((metin_cp(s, i) == 37LL))
            {
                (o = kat(o, "%%"));
            }
            else
            {
                (o = kat(o, intern_dilim(s, i, (i + adv))));
            }
            (i = (i + adv));
        }
        return o;
    }
}

long long interp_expr_kur(char* name, long long satir, long long sutun) {
    {
        long long n = metin_uzun(name);
        long long start = 0LL;
        long long i = 0LL;
        long long expr = 0LL;
        while ((i <= n))
        {
            if (((i == n) || (metin_cp(name, i) == 46LL)))
            {
                char* parca = intern_dilim(name, start, i);
                if ((expr == 0LL))
                {
                    (expr = kimlik_dugum(satir, sutun, parca));
                }
                else
                {
                    long long m = dugum_yeni(DugumTur_D_UYE, satir, sutun);
                    dugum_yaz_a(m, expr);
                    dugum_yaz_ad(m, parca);
                    (expr = m);
                }
                (start = (i + 1LL));
            }
            if ((i < n))
            {
                (i = (i + metin_adv(name, i)));
            }
            else
            {
                (i = (i + 1LL));
            }
        }
        return expr;
    }
}

char* uret_printf_spec(long long t) {
    {
        if ((t == 0LL))
        {
            return "%s";
        }
        char* n = dugum_ad(t);
        if (metin_esit(n, "sayi"))
        {
            return "%lld";
        }
        if (metin_esit(n, "ondalik"))
        {
            return "%g";
        }
        if (metin_esit(n, "metin"))
        {
            return "%s";
        }
        if (metin_esit(n, "mantik"))
        {
            return "%s";
        }
        if ((tablo_bul(2LL, n) != 0LL))
        {
            return "%lld";
        }
        return "%s";
    }
}

char* uret_interp_arg(long long expr) {
    {
        long long t = tip_ifade(expr);
        char* c = uret_ifade(expr);
        if (((t != 0LL) && metin_esit(dugum_ad(t), "mantik")))
        {
            return kat3("((", c, ") ? \"dogru\" : \"yanlis\")");
        }
        if (((t != 0LL) && (tablo_bul(2LL, dugum_ad(t)) != 0LL)))
        {
            return kat3("(long long)(", c, ")");
        }
        return c;
    }
}

char* uret_interp_printf(long long lit, bool newline, char* akis) {
    {
        char* text = metin_coz(dugum_ad(lit));
        bool stderr_mi = metin_esit(akis, "stderr");
        if ((!interp_var_mi(text)))
        {
            if (stderr_mi)
            {
                if (newline)
                {
                    return kat3("fprintf(stderr, \"%s\\n\", ", c_escape(text), ")");
                }
                return kat3("fprintf(stderr, \"%s\", ", c_escape(text), ")");
            }
            if (newline)
            {
                return kat3("printf(\"%s\\n\", ", c_escape(text), ")");
            }
            return kat3("printf(\"%s\", ", c_escape(text), ")");
        }
        long long n = metin_uzun(text);
        char* fmt = upp_metin_kopya("");
        char* args = upp_metin_kopya("");
        long long i = 0LL;
        while ((i < n))
        {
            long long c = metin_cp(text, i);
            long long adv = metin_adv(text, i);
            if ((((c == 123LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 123LL)))
            {
                (fmt = kat(fmt, "{"));
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((((c == 125LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 125LL)))
            {
                (fmt = kat(fmt, "}"));
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((c == 123LL))
            {
                long long j = (i + adv);
                while (((j < n) && (metin_cp(text, j) != 125LL)))
                {
                    (j = (j + metin_adv(text, j)));
                }
                char* name = intern_dilim(text, (i + adv), j);
                if ((!interp_ad_mi(name)))
                {
                    (fmt = kat(fmt, intern_dilim(text, i, (j + 1LL))));
                    (i = (j + 1LL));
                    continue;
                }
                long long expr = interp_expr_kur(name, dugum_satir(lit), dugum_sutun(lit));
                (fmt = kat(fmt, uret_printf_spec(tip_ifade(expr))));
                (args = kat3(args, ", ", uret_interp_arg(expr)));
                (i = (j + 1LL));
                continue;
            }
            if ((c == 37LL))
            {
                (fmt = kat(fmt, "%%"));
            }
            else
            {
                (fmt = kat(fmt, intern_dilim(text, i, (i + adv))));
            }
            (i = (i + adv));
        }
        if (newline)
        {
            (fmt = kat(fmt, "\n"));
        }
        if (stderr_mi)
        {
            return kat4("fprintf(stderr, ", c_escape(fmt), args, ")");
        }
        return kat4("printf(", c_escape(fmt), args, ")");
    }
}

char* uret_interp_heap(long long lit) {
    {
        char* text = metin_coz(dugum_ad(lit));
        long long n = metin_uzun(text);
        char* fmt = upp_metin_kopya("");
        char* args = upp_metin_kopya("");
        long long i = 0LL;
        while ((i < n))
        {
            long long c = metin_cp(text, i);
            long long adv = metin_adv(text, i);
            if ((((c == 123LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 123LL)))
            {
                (fmt = kat(fmt, "{"));
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((((c == 125LL) && ((i + adv) < n)) && (metin_cp(text, (i + adv)) == 125LL)))
            {
                (fmt = kat(fmt, "}"));
                (i = ((i + adv) + metin_adv(text, (i + adv))));
                continue;
            }
            if ((c == 123LL))
            {
                long long j = (i + adv);
                while (((j < n) && (metin_cp(text, j) != 125LL)))
                {
                    (j = (j + metin_adv(text, j)));
                }
                char* name = intern_dilim(text, (i + adv), j);
                if ((!interp_ad_mi(name)))
                {
                    (fmt = kat(fmt, intern_dilim(text, i, (j + 1LL))));
                    (i = (j + 1LL));
                    continue;
                }
                long long expr = interp_expr_kur(name, dugum_satir(lit), dugum_sutun(lit));
                (fmt = kat(fmt, uret_printf_spec(tip_ifade(expr))));
                (args = kat3(args, ", ", uret_interp_arg(expr)));
                (i = (j + 1LL));
                continue;
            }
            if ((c == 37LL))
            {
                (fmt = kat(fmt, "%%"));
            }
            else
            {
                (fmt = kat(fmt, intern_dilim(text, i, (i + adv))));
            }
            (i = (i + adv));
        }
        long long id = tmp_yeni();
        char* nvar = kat("_upp_sn", sayi_metin(id));
        char* var = kat("_upp_s", sayi_metin(id));
        char* s = upp_metin_kopya("({ int ");
        (s = kat(s, nvar));
        (s = kat(s, " = snprintf(NULL, 0, "));
        (s = kat(s, c_escape(fmt)));
        (s = kat(s, args));
        (s = kat(s, "); char* "));
        (s = kat(s, var));
        (s = kat(s, " = NULL; if ("));
        (s = kat(s, nvar));
        (s = kat(s, " >= 0) { "));
        (s = kat(s, var));
        (s = kat(s, " = (char*)malloc((size_t)"));
        (s = kat(s, nvar));
        (s = kat(s, " + 1u); if ("));
        (s = kat(s, var));
        (s = kat(s, ") snprintf("));
        (s = kat(s, var));
        (s = kat(s, ", (size_t)"));
        (s = kat(s, nvar));
        (s = kat(s, " + 1u, "));
        (s = kat(s, c_escape(fmt)));
        (s = kat(s, args));
        (s = kat(s, "); } "));
        (s = kat(s, var));
        (s = kat(s, "; })"));
        return s;
    }
}

char* uret_metin_sabit(long long e) {
    {
        char* text = metin_coz(dugum_ad(e));
        if ((!interp_var_mi(text)))
        {
            return c_escape(text);
        }
        return uret_interp_heap(e);
    }
}

char* uret_kol_pack(char* elem, char* cval) {
    {
        if (metin_esit(elem, "ondalik"))
        {
            return kat3("0LL, ", cval, ", NULL");
        }
        if (metin_esit(elem, "metin"))
        {
            return kat("0LL, 0.0, ", cval);
        }
        if (metin_esit(elem, "bayt"))
        {
            return kat3("(long long)(uint8_t)(", cval, "), 0.0, NULL");
        }
        return kat3("(long long)(", cval, "), 0.0, NULL");
    }
}

char* uret_liste_al(char* elem, char* recv, char* idx) {
    {
        if (metin_esit(elem, "ondalik"))
        {
            return kat4("upp_liste_al_d(", recv, ", ", kat(idx, ")"));
        }
        if (metin_esit(elem, "metin"))
        {
            return kat4("upp_liste_al_s(", recv, ", ", kat(idx, ")"));
        }
        if (metin_esit(elem, "mantik"))
        {
            return kat4("(bool)upp_liste_al_i(", recv, ", ", kat(idx, ")"));
        }
        if (metin_esit(elem, "bayt"))
        {
            return kat4("(uint8_t)upp_liste_al_i(", recv, ", ", kat(idx, ")"));
        }
        return kat4("upp_liste_al_i(", recv, ", ", kat(idx, ")"));
    }
}

char* uret_harita_key(char* ktur, long long e) {
    {
        char* c = uret_ifade(e);
        if (metin_esit(ktur, "metin"))
        {
            return kat("0LL, ", c);
        }
        return kat3("(long long)(", c, "), NULL");
    }
}

char* uret_liste_cagri(long long expr, char* on) {
    {
        long long member = dugum_a(expr);
        char* recv = uret_ifade(dugum_a(member));
        char* elem = liste_elem_ad(on);
        long long tag = koleksiyon_etiket(elem);
        char* name = dugum_ad(member);
        long long args = dugum_liste(expr);
        if (metin_esit(name, "ekle"))
        {
            return kat(kat5("upp_liste_ekle(&(", recv, "), ", sayi_metin(tag), ", "), kat(uret_kol_pack(elem, uret_ifade(liste_al(args, 0LL))), ")"));
        }
        if (metin_esit(name, "al"))
        {
            return uret_liste_al(elem, recv, uret_ifade(liste_al(args, 0LL)));
        }
        if (metin_esit(name, "yaz"))
        {
            return kat(kat5("upp_liste_yaz(&(", recv, "), ", sayi_metin(tag), ", "), kat(kat3(uret_ifade(liste_al(args, 0LL)), ", ", uret_kol_pack(elem, uret_ifade(liste_al(args, 1LL)))), ")"));
        }
        if (metin_esit(name, "uzunluk"))
        {
            return kat3("upp_liste_uzunluk(", recv, ")");
        }
        if (metin_esit(name, "bosalt"))
        {
            return kat3("upp_liste_bosalt(&(", recv, "))");
        }
        if (metin_esit(name, "sil"))
        {
            return kat(kat3("upp_liste_sil(&(", recv, "), "), kat(uret_ifade(liste_al(args, 0LL)), ")"));
        }
        if (metin_esit(name, "temizle"))
        {
            return kat3("upp_liste_temizle(&(", recv, "))");
        }
        return "0";
    }
}

char* uret_harita_al(char* vtur, char* recv, char* kpack) {
    {
        if (metin_esit(vtur, "ondalik"))
        {
            return kat4("upp_harita_al_d(", recv, ", ", kat(kpack, ")"));
        }
        if (metin_esit(vtur, "metin"))
        {
            return kat4("upp_harita_al_s(", recv, ", ", kat(kpack, ")"));
        }
        if (metin_esit(vtur, "mantik"))
        {
            return kat4("(bool)upp_harita_al_i(", recv, ", ", kat(kpack, ")"));
        }
        return kat4("upp_harita_al_i(", recv, ", ", kat(kpack, ")"));
    }
}

char* uret_harita_cagri(long long expr, char* on) {
    {
        long long member = dugum_a(expr);
        char* recv = uret_ifade(dugum_a(member));
        char* ktur = harita_anahtar_ad(on);
        char* vtur = harita_deger_ad(on);
        long long ktag = koleksiyon_etiket(ktur);
        long long vtag = koleksiyon_etiket(vtur);
        char* name = dugum_ad(member);
        long long args = dugum_liste(expr);
        if (metin_esit(name, "koy"))
        {
            return kat(kat5("upp_harita_koy(&(", recv, "), ", sayi_metin(ktag), ", "), kat(kat5(sayi_metin(vtag), ", ", uret_harita_key(ktur, liste_al(args, 0LL)), ", ", uret_kol_pack(vtur, uret_ifade(liste_al(args, 1LL)))), ")"));
        }
        if (metin_esit(name, "al"))
        {
            return uret_harita_al(vtur, recv, kat3(sayi_metin(ktag), ", ", uret_harita_key(ktur, liste_al(args, 0LL))));
        }
        if (metin_esit(name, "var_mi"))
        {
            return kat(kat5("(bool)upp_harita_var_mi(", recv, ", ", sayi_metin(ktag), ", "), kat(uret_harita_key(ktur, liste_al(args, 0LL)), ")"));
        }
        if (metin_esit(name, "uzunluk"))
        {
            return kat3("upp_harita_uzunluk(", recv, ")");
        }
        if (metin_esit(name, "bosalt"))
        {
            return kat3("upp_harita_bosalt(&(", recv, "))");
        }
        if (metin_esit(name, "anahtarlar"))
        {
            return kat4("upp_harita_anahtarlar(", recv, ", ", kat(sayi_metin(ktag), ")"));
        }
        if (metin_esit(name, "sil"))
        {
            return kat(kat5("(bool)upp_harita_sil(&(", recv, "), ", sayi_metin(ktag), ", "), kat(uret_harita_key(ktur, liste_al(args, 0LL)), ")"));
        }
        if (metin_esit(name, "temizle"))
        {
            return kat3("upp_harita_temizle(&(", recv, "))");
        }
        return "0";
    }
}

char* uret_uye(long long e) {
    {
        long long obj_t = tip_ifade(dugum_a(e));
        char* on = dugum_ad(obj_t);
        if (((metin_uzun(on) >= 8LL) && metin_esit(intern_dilim(on, 0LL, 8LL), "secenek.")))
        {
            return kat3(nokta_altcizgi(intern_dilim(on, 8LL, metin_uzun(on))), "_", dugum_ad(e));
        }
        if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "modul.")))
        {
            return c_sembol(dugum_ad(e), intern_dilim(on, 6LL, metin_uzun(on)));
        }
        if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "sinif.")))
        {
            char* key = intern_dilim(on, 6LL, metin_uzun(on));
            long long cls = tablo_bul(0LL, key);
            if ((cls == 0LL))
            {
                return "0";
            }
            return kat3(dugum_c_ad(cls), "_", dugum_ad(e));
        }
        if ((liste_tur_mi(on) || harita_tur_mi(on)))
        {
            hata_ver(dugum_satir(e), dugum_sutun(e), "Bu üye bir metottur; çağrı '...' ile yazın.");
            return "0";
        }
        char* ok = upp_metin_kopya(".");
        if ((dugum_ptrd(obj_t) > 0LL))
        {
            (ok = "->");
        }
        return kat3(uret_ifade(dugum_a(e)), ok, dugum_ad(e));
    }
}

char* uret_upp_cagri(char* name, long long args, long long satir, long long sutun) {
    {
        if ((metin_esit(name, "yaz") || metin_esit(name, "satir_yaz")))
        {
            if ((liste_uzunluk(args) == 0LL))
            {
                if (metin_esit(name, "satir_yaz"))
                {
                    return "printf(\"\\n\")";
                }
                return "printf(\"\")";
            }
            long long first = liste_al(args, 0LL);
            if (((dugum_tur(first) == DugumTur_D_SABIT) && (dugum_ptrd(first) == 2LL)))
            {
                if ((liste_uzunluk(args) > 1LL))
                {
                    return kat3("printf(", uret_args(args), ")");
                }
                return uret_interp_printf(first, metin_esit(name, "satir_yaz"), "");
            }
            if ((liste_uzunluk(args) > 1LL))
            {
                return kat3("printf(", uret_args(args), ")");
            }
            char* c = uret_ifade(first);
            char* safe = kat5("((", c, ") ? (", c, ") : \"\")");
            if (metin_esit(name, "satir_yaz"))
            {
                return kat3("printf(\"%s\\n\", ", safe, ")");
            }
            return kat3("printf(\"%s\", ", safe, ")");
        }
        if ((metin_esit(name, "hata_yaz") || metin_esit(name, "hata_satir_yaz")))
        {
            bool nl = metin_esit(name, "hata_satir_yaz");
            if ((liste_uzunluk(args) == 0LL))
            {
                if (nl)
                {
                    return "fprintf(stderr, \"\\n\")";
                }
                return "(void)0";
            }
            long long first = liste_al(args, 0LL);
            if (((dugum_tur(first) == DugumTur_D_SABIT) && (dugum_ptrd(first) == 2LL)))
            {
                if ((liste_uzunluk(args) > 1LL))
                {
                    return kat3("fprintf(stderr, ", uret_args(args), ")");
                }
                return uret_interp_printf(first, nl, "stderr");
            }
            if ((liste_uzunluk(args) > 1LL))
            {
                return kat3("fprintf(stderr, ", uret_args(args), ")");
            }
            char* c = uret_ifade(first);
            char* safe = kat5("((", c, ") ? (", c, ") : \"\")");
            if (nl)
            {
                return kat3("fprintf(stderr, \"%s\\n\", ", safe, ")");
            }
            return kat3("fprintf(stderr, \"%s\", ", safe, ")");
        }
        if (metin_esit(name, "bekle"))
        {
            return kat3("upp_arkaplan_bekle_deger(", uret_ifade(liste_al(args, 0LL)), ")");
        }
        char* cn = name;
        if (metin_esit(name, "mesaj"))
        {
            (cn = "upp_mesaj");
        }
        else
            if (metin_esit(name, "hata"))
            {
                (cn = "upp_hata");
            }
            else
                if (metin_esit(name, "ses_cal"))
                {
                    (cn = "upp_ses_cal");
                }
                else
                    if (metin_esit(name, "uyut"))
                    {
                        (cn = "upp_uyut");
                    }
                    else
                        if (metin_esit(name, "rastgele"))
                        {
                            (cn = "upp_rastgele");
                        }
                        else
                            if (metin_esit(name, "dosya_oku"))
                            {
                                (cn = "upp_dosya_oku");
                            }
                            else
                                if (metin_esit(name, "dosya_yaz"))
                                {
                                    (cn = "upp_dosya_yaz");
                                }
                                else
                                    if (metin_esit(name, "uzunluk"))
                                    {
                                        (cn = "upp_uzunluk");
                                    }
                                    else
                                        if (metin_esit(name, "giris"))
                                        {
                                            (cn = "upp_giris");
                                        }
                                        else
                                            if (metin_esit(name, "min"))
                                            {
                                                (cn = "upp_min");
                                            }
                                            else
                                                if (metin_esit(name, "max"))
                                                {
                                                    (cn = "upp_max");
                                                }
                                                else
                                                    if (metin_esit(name, "mutlak"))
                                                    {
                                                        (cn = "upp_mutlak");
                                                    }
                                                    else
                                                        if (metin_esit(name, "metin_bosalt"))
                                                        {
                                                            (cn = "upp_metin_bosalt");
                                                        }
                                                        else
                                                            if (metin_esit(name, "arguman_sayisi"))
                                                            {
                                                                (cn = "upp_arguman_sayisi");
                                                            }
                                                            else
                                                                if (metin_esit(name, "arguman"))
                                                                {
                                                                    (cn = "upp_arguman");
                                                                }
                                                                else
                                                                    if (metin_esit(name, "dosya_var_mi"))
                                                                    {
                                                                        (cn = "upp_dosya_var_mi");
                                                                    }
                                                                    else
                                                                        if (metin_esit(name, "dosya_sil"))
                                                                        {
                                                                            (cn = "upp_dosya_sil");
                                                                        }
                                                                        else
                                                                            if (metin_esit(name, "zaman"))
                                                                            {
                                                                                (cn = "upp_zaman");
                                                                            }
                                                                            else
                                                                                if (metin_esit(name, "metinden_sayiya"))
                                                                                {
                                                                                    (cn = "upp_metinden_sayiya");
                                                                                }
                                                                                else
                                                                                    if (metin_esit(name, "sayidan_metin"))
                                                                                    {
                                                                                        (cn = "upp_sayidan_metin");
                                                                                    }
                                                                                    else
                                                                                        if (metin_esit(name, "ondalikdan_metin"))
                                                                                        {
                                                                                            (cn = "upp_ondalikdan_metin");
                                                                                        }
                                                                                        else
                                                                                            if (metin_esit(name, "metinden_ondalik"))
                                                                                            {
                                                                                                (cn = "upp_metinden_ondalik");
                                                                                            }
                                                                                            else
                                                                                                if (metin_esit(name, "dosya_ekle"))
                                                                                                {
                                                                                                    (cn = "upp_dosya_ekle");
                                                                                                }
                                                                                                else
                                                                                                    if (metin_esit(name, "dosya_boyut"))
                                                                                                    {
                                                                                                        (cn = "upp_dosya_boyut");
                                                                                                    }
                                                                                                    else
                                                                                                        if (metin_esit(name, "ortam_al"))
                                                                                                        {
                                                                                                            (cn = "upp_ortam_al");
                                                                                                        }
                                                                                                        else
                                                                                                            if (metin_esit(name, "ortam_yaz"))
                                                                                                            {
                                                                                                                (cn = "upp_ortam_yaz");
                                                                                                            }
                                                                                                            else
                                                                                                                if (metin_esit(name, "platform"))
                                                                                                                {
                                                                                                                    (cn = "upp_platform");
                                                                                                                }
        if ((metin_esit(name, "uzunluk") && (liste_uzunluk(args) == 1LL)))
        {
            long long a0 = liste_al(args, 0LL);
            long long at = tip_ifade(a0);
            if ((dugum_dims(at) > 0LL))
            {
                if ((dugum_tur(a0) == DugumTur_D_DIZI_SABIT))
                {
                    return sayi_metin(liste_uzunluk(dugum_liste(a0)));
                }
                char* c = uret_ifade(a0);
                return kat5("(long long)(sizeof(", c, ") / sizeof((", c, ")[0]))");
            }
            if (liste_tur_mi(dugum_ad(at)))
            {
                return kat3("upp_liste_uzunluk(", uret_ifade(a0), ")");
            }
            if (harita_tur_mi(dugum_ad(at)))
            {
                return kat3("upp_harita_uzunluk(", uret_ifade(a0), ")");
            }
        }
        return kat4(cn, "(", uret_args(args), ")");
    }
}

char* uret_dizi_ptr_say(long long e, char* elem_c) {
    {
        char* c = upp_metin_kopya("");
        long long L = 0LL;
        if ((e == 0LL))
        {
            return kat5("((", elem_c, "*)0), 0", "", "");
        }
        if ((dugum_tur(e) == DugumTur_D_DIZI_SABIT))
        {
            (L = dugum_liste(e));
            if ((liste_uzunluk(L) == 0LL))
            {
                return kat3("((", elem_c, "*)0), 0");
            }
            return kat(kat5("(", elem_c, "[]){", uret_args(L), "}, "), sayi_metin(liste_uzunluk(L)));
        }
        (c = uret_ifade(e));
        return kat(kat5(c, ", (long long)(sizeof(", c, ") / sizeof((", c), ")[0]))");
    }
}

char* uret_ns_cagri(char* ns, char* name, long long args) {
    {
        if (metin_esit(ns, "matematik"))
        {
            char* cn = kat("upp_mat_", name);
            if (metin_esit(name, "mesafe"))
            {
                (cn = "upp_mat_mesafe_v3");
            }
            else
                if (metin_esit(name, "mesafe_3d"))
                {
                    (cn = "upp_mat_mesafe_3d");
                }
                else
                    if (metin_esit(name, "aci_hesapla"))
                    {
                        (cn = "upp_mat_aci_hesapla");
                    }
                    else
                        if (metin_esit(name, "dunya_ekran"))
                        {
                            return kat(kat(kat5("upp_mat_dunya_ekran(", uret_ifade(liste_al(args, 0LL)), ", ", uret_ifade(liste_al(args, 1LL)), ", "), kat5(uret_ifade(liste_al(args, 2LL)), ", ", uret_dizi_ptr_say(liste_al(args, 3LL), "double"), ", ", uret_ifade(liste_al(args, 4LL)))), kat3(", ", uret_ifade(liste_al(args, 5LL)), ")"));
                        }
            return kat4(cn, "(", uret_args(args), ")");
        }
        if (((metin_esit(ns, "windows.bellek") || metin_esit(ns, "bellek")) && metin_esit(name, "zincir_oku")))
        {
            return kat(kat3("upp_windows_bellek_zincir_oku(", uret_ifade(liste_al(args, 0LL)), ", "), kat(uret_dizi_ptr_say(liste_al(args, 1LL), "long long"), ")"));
        }
        char* pref = kat3("upp_", nokta_altcizgi(ns), "_");
        if (metin_esit(ns, "metin"))
        {
            (pref = "upp_metin_");
        }
        else
            if (metin_esit(ns, "girdi"))
            {
                (pref = "upp_girdi_");
            }
            else
                if (metin_esit(ns, "bellek"))
                {
                    (pref = "upp_bellek_");
                }
                else
                    if (metin_esit(ns, "cizim"))
                    {
                        (pref = "upp_cizim_");
                    }
                    else
                        if (metin_esit(ns, "json"))
                        {
                            (pref = "upp_json_");
                        }
                        else
                            if (metin_esit(ns, "stdio"))
                            {
                                (pref = "upp_stdio_");
                            }
                            else
                                if (metin_esit(ns, "sistem"))
                                {
                                    (pref = "upp_sistem_");
                                }
                                else
                                    if (metin_esit(ns, "yol"))
                                    {
                                        (pref = "upp_yol_");
                                    }
                                    else
                                        if (metin_esit(ns, "dosya"))
                                        {
                                            (pref = "upp_dosya_");
                                        }
        if ((metin_esit(ns, "metin") && metin_esit(name, "birlestir")))
        {
            return kat4("upp_metin_liste_birlestir(", uret_args(args), "", ")");
        }
        return kat4(pref, name, "(", kat(uret_args(args), ")"));
    }
}

char* uret_metot(long long expr) {
    {
        long long member = dugum_a(expr);
        long long obj_t = tip_ifade(dugum_a(member));
        char* on = dugum_ad(obj_t);
        if (metin_esit(on, "upp"))
        {
            return uret_upp_cagri(dugum_ad(member), dugum_liste(expr), dugum_satir(expr), dugum_sutun(expr));
        }
        if (((metin_uzun(on) >= 4LL) && metin_esit(intern_dilim(on, 0LL, 4LL), "upp.")))
        {
            return uret_ns_cagri(intern_dilim(on, 4LL, metin_uzun(on)), dugum_ad(member), dugum_liste(expr));
        }
        if ((metin_esit(on, "ArkaplanIs") && metin_esit(dugum_ad(member), "bekle")))
        {
            return kat3("upp_arkaplan_bekle(&(", uret_ifade(dugum_a(member)), "))");
        }
        if ((metin_esit(on, "Kilit") && (metin_esit(dugum_ad(member), "kilitle") || metin_esit(dugum_ad(member), "birak"))))
        {
            return kat(kat5("upp_kilit_", dugum_ad(member), "(&(", uret_ifade(dugum_a(member)), "))"), "");
        }
        if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "modul.")))
        {
            char* alias = intern_dilim(on, 6LL, metin_uzun(on));
            char* name = dugum_ad(member);
            long long fn = tablo_bul(1LL, c_sembol(name, alias));
            if ((fn != 0LL))
            {
                return kat4(dugum_c_ad(fn), "(", uret_args_fn(fn, dugum_liste(expr)), ")");
            }
            long long kcls = tablo_bul(0LL, intern(kat3(alias, ".", name)));
            if ((kcls != 0LL))
            {
                return uret_kurucu(kcls, dugum_liste(expr));
            }
            return "0";
        }
        if (((metin_uzun(on) >= 6LL) && metin_esit(intern_dilim(on, 0LL, 6LL), "sinif.")))
        {
            char* key = intern_dilim(on, 6LL, metin_uzun(on));
            long long kcls = tablo_bul(0LL, key);
            if ((kcls != 0LL))
            {
                return uret_kurucu(kcls, dugum_liste(expr));
            }
            return "0";
        }
        if (liste_tur_mi(on))
        {
            return uret_liste_cagri(expr, on);
        }
        if (harita_tur_mi(on))
        {
            return uret_harita_cagri(expr, on);
        }
        long long cls = tablo_bul(0LL, on);
        char* cn = nokta_altcizgi(on);
        if ((cls != 0LL))
        {
            (cn = dugum_c_ad(cls));
        }
        long long method = 0LL;
        if ((cls != 0LL))
        {
            long long M = dugum_liste2(cls);
            long long mi = 0LL;
            while ((mi < liste_uzunluk(M)))
            {
                if (metin_esit(dugum_ad(liste_al(M, mi)), dugum_ad(member)))
                {
                    (method = liste_al(M, mi));
                }
                (mi = (mi + 1LL));
            }
        }
        char* s = kat5(cn, "_", dugum_ad(member), "(&(", uret_ifade(dugum_a(member)));
        (s = kat(s, ")"));
        if ((method != 0LL))
        {
            if ((liste_uzunluk(dugum_liste(method)) > 0LL))
            {
                (s = kat3(s, ", ", uret_args_fn(method, dugum_liste(expr))));
            }
        }
        else
            if ((liste_uzunluk(dugum_liste(expr)) > 0LL))
            {
                (s = kat3(s, ", ", uret_args(dugum_liste(expr))));
            }
        return kat(s, ")");
    }
}

char* uret_dizi_idx(long long arr_t, long long idx_e) {
    {
        char* idx = uret_ifade(idx_e);
        if ((((arr_t != 0LL) && (dugum_dims(arr_t) > 0LL)) && (dugum_sayi(arr_t) > 0LL)))
        {
            return kat4("upp_dizi_idx(", idx, ", ", kat(sayi_metin(dugum_sayi(arr_t)), ")"));
        }
        return idx;
    }
}

char* uret_ifade(long long e) {
    {
        if ((e == 0LL))
        {
            return "";
        }
        long long t = dugum_tur(e);
        if ((t == DugumTur_D_SABIT))
        {
            return uret_sabit(e);
        }
        if ((t == DugumTur_D_KIMLIK))
        {
            return dugum_ad(e);
        }
        if ((t == DugumTur_D_ATAMA))
        {
            char* op = dugum_ad(e);
            if (metin_esit(op, ""))
            {
                (op = "=");
            }
            if ((dugum_tur(dugum_a(e)) == DugumTur_D_INDIS))
            {
                long long arr_t = tip_ifade(dugum_a(dugum_a(e)));
                if (liste_tur_mi(dugum_ad(arr_t)))
                {
                    char* elem = liste_elem_ad(dugum_ad(arr_t));
                    char* arr = uret_ifade(dugum_a(dugum_a(e)));
                    char* idx = uret_ifade(dugum_b(dugum_a(e)));
                    char* rhs = uret_ifade(dugum_b(e));
                    if ((!metin_esit(op, "=")))
                    {
                        char* al = uret_liste_al(elem, arr, idx);
                        (rhs = kat5("(", al, kat3(" ", intern_dilim(op, 0LL, 1LL), " "), rhs, ")"));
                    }
                    return kat(kat5("upp_liste_yaz(&(", arr, "), ", sayi_metin(koleksiyon_etiket(elem)), ", "), kat(kat3(idx, ", ", uret_kol_pack(elem, rhs)), ")"));
                }
            }
            if (metin_esit(op, "+="))
            {
                long long tt = tip_ifade(dugum_a(e));
                if ((tip_metin_mi(tt) && (dugum_tur(dugum_a(e)) != DugumTur_D_INDIS)))
                {
                    return kat4("upp_metin_ekle(&(", uret_ifade(dugum_a(e)), "), ", kat(uret_ifade(dugum_b(e)), ")"));
                }
            }
            return kat5("(", uret_ifade(dugum_a(e)), kat3(" ", op, " "), uret_ifade(dugum_b(e)), ")");
        }
        if ((t == DugumTur_D_UCLU))
        {
            return kat(kat5("(", uret_ifade(dugum_a(e)), " ? ", uret_ifade(dugum_b(e)), " : "), kat(uret_ifade(dugum_c(e)), ")"));
        }
        if ((t == DugumTur_D_IKIL))
        {
            long long lt = tip_ifade(dugum_a(e));
            long long rt = tip_ifade(dugum_b(e));
            if (((metin_esit(dugum_ad(e), "+") && tip_metin_mi(lt)) && tip_metin_mi(rt)))
            {
                return kat4("upp_metin_birlestir(", uret_ifade(dugum_a(e)), ", ", kat(uret_ifade(dugum_b(e)), ")"));
            }
            if ((((metin_esit(dugum_ad(e), "==") || metin_esit(dugum_ad(e), "!=")) && tip_metin_mi(lt)) && tip_metin_mi(rt)))
            {
                char* eq = kat4("upp_metin_esit(", uret_ifade(dugum_a(e)), ", ", kat(uret_ifade(dugum_b(e)), ")"));
                if (metin_esit(dugum_ad(e), "!="))
                {
                    return kat3("(!", eq, ")");
                }
                return eq;
            }
            return kat5("(", uret_ifade(dugum_a(e)), kat3(" ", dugum_ad(e), " "), uret_ifade(dugum_b(e)), ")");
        }
        if ((t == DugumTur_D_TEKIL))
        {
            return kat4("(", dugum_ad(e), uret_ifade(dugum_a(e)), ")");
        }
        if ((t == DugumTur_D_CAGRI))
        {
            if ((dugum_tur(dugum_a(e)) == DugumTur_D_UYE))
            {
                return uret_metot(e);
            }
            if ((dugum_tur(dugum_a(e)) == DugumTur_D_KIMLIK))
            {
                char* name = dugum_ad(dugum_a(e));
                long long cls_id = tablo_bul(0LL, name);
                if (((cls_id != 0LL) && (!yerlesik_sinif_mi(name))))
                {
                    return uret_kurucu(cls_id, dugum_liste(e));
                }
                long long fn = tablo_bul(1LL, name);
                if ((fn != 0LL))
                {
                    return kat4(dugum_c_ad(fn), "(", uret_args_fn(fn, dugum_liste(e)), ")");
                }
            }
            return kat4(uret_ifade(dugum_a(e)), "(", uret_args(dugum_liste(e)), ")");
        }
        if ((t == DugumTur_D_UYE))
        {
            return uret_uye(e);
        }
        if ((t == DugumTur_D_INDIS))
        {
            long long arr_t = tip_ifade(dugum_a(e));
            if (liste_tur_mi(dugum_ad(arr_t)))
            {
                return uret_liste_al(liste_elem_ad(dugum_ad(arr_t)), uret_ifade(dugum_a(e)), uret_ifade(dugum_b(e)));
            }
            if (tip_metin_mi(arr_t))
            {
                return kat4("upp_metin_kod(", uret_ifade(dugum_a(e)), ", ", kat(uret_ifade(dugum_b(e)), ")"));
            }
            return kat4(uret_ifade(dugum_a(e)), "[", uret_dizi_idx(arr_t, dugum_b(e)), "]");
        }
        if ((t == DugumTur_D_DIZI_SABIT))
        {
            return kat3("{", uret_args(dugum_liste(e)), "}");
        }
        if ((t == DugumTur_D_ARKAPLAN_IFADE))
        {
            return uret_bg_ifade(e);
        }
        return "0";
    }
}

char* uret_bg_sifir(long long t) {
    {
        if ((t == 0LL))
        {
            return "0";
        }
        if (((dugum_ptrd(t) > 0LL) || tip_metin_mi(t)))
        {
            return "NULL";
        }
        if (metin_esit(dugum_ad(t), "mantik"))
        {
            return "false";
        }
        if (metin_esit(dugum_ad(t), "ondalik"))
        {
            return "0.0";
        }
        if ((liste_tur_mi(dugum_ad(t)) || harita_tur_mi(dugum_ad(t))))
        {
            return "{0}";
        }
        if ((tablo_bul(0LL, dugum_ad(t)) != 0LL))
        {
            return "{0}";
        }
        return "0";
    }
}

char* uret_bg_ctx_ad(long long id) {
    {
        return kat("_upp_bgctx_", sayi_metin(id));
    }
}

char* uret_bg_fn_ad(long long id) {
    {
        return kat("_upp_bg_", sayi_metin(id));
    }
}

char* uret_bg_sp_ad(long long id) {
    {
        return kat("_upp_spawn_", sayi_metin(id));
    }
}

void uret_bg_kopya_alan(char* hedef, long long cap) {
    {
        char* ad = dugum_ad(cap);
        if (tip_metin_mi(dugum_a(cap)))
        {
            emit(kat(kat3(hedef, ad, " = upp_metin_kopya("), kat(ad, ");")));
        }
        else
        {
            emit(kat(kat3(hedef, ad, " = "), kat(ad, ";")));
        }
    }
}

void uret_bg_metin_ctx_birak(long long caps, char* p) {
    {
        long long i = 0LL;
        while ((i < liste_uzunluk(caps)))
        {
            long long cap = liste_al(caps, i);
            if (tip_metin_mi(dugum_a(cap)))
            {
                emit(kat(kat(kat5("if (", p, "->", dugum_ad(cap), ") free("), kat3(p, "->", dugum_ad(cap))), ");"));
            }
            (i = (i + 1LL));
        }
    }
}

char* uret_bg_kopya_hata_kosul(char* hedef, long long caps) {
    {
        char* s = upp_metin_kopya("");
        long long i = 0LL;
        while ((i < liste_uzunluk(caps)))
        {
            long long cap = liste_al(caps, i);
            if (tip_metin_mi(dugum_a(cap)))
            {
                char* ad = dugum_ad(cap);
                char* parca = kat(kat5("(", ad, " && !", hedef, ad), ")");
                if (metin_esit(s, ""))
                {
                    (s = parca);
                }
                else
                {
                    (s = kat3(s, " || ", parca));
                }
            }
            (i = (i + 1LL));
        }
        return s;
    }
}

char* uret_bg_param_listesi(long long caps) {
    {
        char* s = upp_metin_kopya("");
        long long i = 0LL;
        if ((liste_uzunluk(caps) == 0LL))
        {
            return "void";
        }
        while ((i < liste_uzunluk(caps)))
        {
            long long cap = liste_al(caps, i);
            if ((i > 0LL))
            {
                (s = kat(s, ", "));
            }
            (s = kat(s, uret_ctur(dugum_a(cap))));
            (s = kat(s, " "));
            (s = kat(s, dugum_ad(cap)));
            (i = (i + 1LL));
        }
        return s;
    }
}

char* uret_bg_arg_listesi(long long caps) {
    {
        char* s = upp_metin_kopya("");
        long long i = 0LL;
        while ((i < liste_uzunluk(caps)))
        {
            if ((i > 0LL))
            {
                (s = kat(s, ", "));
            }
            (s = kat(s, dugum_ad(liste_al(caps, i))));
            (i = (i + 1LL));
        }
        return s;
    }
}

char* uret_bg_ifade(long long e) {
    {
        long long id = dugum_sayi(e);
        long long caps = dugum_liste2(e);
        return kat4(uret_bg_sp_ad(id), "(", uret_bg_arg_listesi(caps), ")");
    }
}

bool linux_mi(void) {
    {
        return (os_hedef_al() != 0LL);
    }
}

void uret_bg_isci(long long node) {
    {
        long long id = dugum_sayi(node);
        char* fn = uret_bg_fn_ad(id);
        char* sp = uret_bg_sp_ad(id);
        char* ctx = uret_bg_ctx_ad(id);
        long long caps = dugum_liste2(node);
        long long eski = indent_al();
        long long i = 0LL;
        long long cap = 0LL;
        long long body = 0LL;
        indent_kur(0LL);
        if ((liste_uzunluk(caps) > 0LL))
        {
            emit(kat3("struct ", ctx, " {"));
            indent_ekle(1LL);
            (i = 0LL);
            while ((i < liste_uzunluk(caps)))
            {
                (cap = liste_al(caps, i));
                emit(kat(kat3(uret_ctur(dugum_a(cap)), " ", dugum_ad(cap)), ";"));
                (i = (i + 1LL));
            }
            indent_ekle((-1LL));
            emit("};");
        }
        if (linux_mi())
        {
            emit(kat3("static void* ", fn, "(void* _p) {"));
        }
        else
        {
            emit(kat3("static DWORD WINAPI ", fn, "(LPVOID _p) {"));
        }
        indent_ekle(1LL);
        if ((liste_uzunluk(caps) > 0LL))
        {
            emit(kat3("struct ", ctx, " _c;"));
            (i = 0LL);
            while ((i < liste_uzunluk(caps)))
            {
                (cap = liste_al(caps, i));
                emit(kat(kat5(uret_ctur(dugum_a(cap)), " ", dugum_ad(cap), " = ", uret_bg_sifir(dugum_a(cap))), ";"));
                (i = (i + 1LL));
            }
            emit("if (_p) {");
            indent_ekle(1LL);
            emit(kat3("_c = *(struct ", ctx, "*)_p;"));
            emit("free(_p);");
            (i = 0LL);
            while ((i < liste_uzunluk(caps)))
            {
                (cap = liste_al(caps, i));
                emit(kat(kat3(dugum_ad(cap), " = _c.", dugum_ad(cap)), ";"));
                (i = (i + 1LL));
            }
            indent_ekle((-1LL));
            emit("}");
        }
        else
        {
            emit("(void)_p;");
        }
        kapsam_sifirla();
        kapsam_ac();
        (i = 0LL);
        while ((i < liste_uzunluk(caps)))
        {
            (cap = liste_al(caps, i));
            kapsam_bagla(dugum_ad(cap), dugum_a(cap));
            (i = (i + 1LL));
        }
        bg_yaz((bg_al() + 1LL));
        emit("{");
        indent_ekle(1LL);
        (body = dugum_a(node));
        if (((body != 0LL) && (dugum_tur(body) == DugumTur_D_BLOK)))
        {
            long long L = dugum_liste(body);
            (i = 0LL);
            while ((i < liste_uzunluk(L)))
            {
                uret_cumle(liste_al(L, i));
                (i = (i + 1LL));
            }
        }
        else
        {
            uret_cumle(body);
        }
        indent_ekle((-1LL));
        emit("}");
        bg_yaz((bg_al() - 1LL));
        kapsam_kapat();
        (i = 0LL);
        while ((i < liste_uzunluk(caps)))
        {
            (cap = liste_al(caps, i));
            if (tip_metin_mi(dugum_a(cap)))
            {
                emit(kat(kat3("if (", dugum_ad(cap), ") { free("), kat(dugum_ad(cap), "); }")));
            }
            (i = (i + 1LL));
        }
        emit("return 0;");
        indent_ekle((-1LL));
        emit("}");
        if ((dugum_tur(node) == DugumTur_D_ARKAPLAN_IFADE))
        {
            emit(kat4("static ArkaplanIs ", sp, "(", kat(uret_bg_param_listesi(caps), ") {")));
            indent_ekle(1LL);
            emit("ArkaplanIs _j;");
            emit("_j.handle = 0;");
            if ((liste_uzunluk(caps) > 0LL))
            {
                emit(kat(kat5("struct ", ctx, "* _ctx = (struct ", ctx, "*)malloc(sizeof(struct "), kat(ctx, "));")));
                emit("if (!_ctx) {");
                indent_ekle(1LL);
                emit("fprintf(stderr, \"[u++] arkaplan: bellek yetersiz.\\n\");");
                emit("return _j;");
                indent_ekle((-1LL));
                emit("}");
                (i = 0LL);
                while ((i < liste_uzunluk(caps)))
                {
                    uret_bg_kopya_alan("_ctx->", liste_al(caps, i));
                    (i = (i + 1LL));
                }
                char* kosul = uret_bg_kopya_hata_kosul("_ctx->", caps);
                if ((!metin_esit(kosul, "")))
                {
                    emit(kat3("if (", kosul, ") {"));
                    indent_ekle(1LL);
                    emit("fprintf(stderr, \"[u++] arkaplan: bellek yetersiz.\\n\");");
                    uret_bg_metin_ctx_birak(caps, "_ctx");
                    emit("free(_ctx);");
                    emit("return _j;");
                    indent_ekle((-1LL));
                    emit("}");
                }
                if (linux_mi())
                {
                    emit(kat3("{ pthread_t _th; if (pthread_create(&_th, NULL, ", fn, ", _ctx) == 0) { _j.handle = (long long)(uintptr_t)_th; upp_arkaplan_kaydet(_j.handle); } else {"));
                }
                else
                {
                    emit(kat3("{ HANDLE _th = CreateThread(NULL, 0, ", fn, ", _ctx, 0, NULL);"));
                    emit("  if (_th) { _j.handle = (long long)(intptr_t)_th; upp_arkaplan_kaydet(_j.handle); } else {");
                }
                indent_ekle(1LL);
                emit("fprintf(stderr, \"[u++] arkaplan: is parcacigi baslatilamadi.\\n\");");
                uret_bg_metin_ctx_birak(caps, "_ctx");
                emit("free(_ctx);");
                indent_ekle((-1LL));
                emit("} }");
            }
            else
            {
                if (linux_mi())
                {
                    emit(kat3("{ pthread_t _th; if (pthread_create(&_th, NULL, ", fn, ", NULL) == 0) { _j.handle = (long long)(uintptr_t)_th; upp_arkaplan_kaydet(_j.handle); } else {"));
                }
                else
                {
                    emit(kat3("{ HANDLE _th = CreateThread(NULL, 0, ", fn, ", NULL, 0, NULL);"));
                    emit("  if (_th) { _j.handle = (long long)(intptr_t)_th; upp_arkaplan_kaydet(_j.handle); } else {");
                }
                emit("    fprintf(stderr, \"[u++] arkaplan: is parcacigi baslatilamadi.\\n\");");
                emit("} }");
            }
            emit("return _j;");
            indent_ekle((-1LL));
            emit("}");
        }
        emit("");
        indent_kur(eski);
    }
}

void uret_bg_deyim(long long stmt) {
    {
        long long id = dugum_sayi(stmt);
        char* fn = uret_bg_fn_ad(id);
        char* ctx = uret_bg_ctx_ad(id);
        long long caps = dugum_liste2(stmt);
        long long i = 0LL;
        emit("{");
        indent_ekle(1LL);
        if ((liste_uzunluk(caps) > 0LL))
        {
            emit(kat(kat5("struct ", ctx, "* _ctx = (struct ", ctx, "*)malloc(sizeof(struct "), kat(ctx, "));")));
            emit("if (_ctx) {");
            indent_ekle(1LL);
            (i = 0LL);
            while ((i < liste_uzunluk(caps)))
            {
                uret_bg_kopya_alan("_ctx->", liste_al(caps, i));
                (i = (i + 1LL));
            }
            char* kosul = uret_bg_kopya_hata_kosul("_ctx->", caps);
            if ((!metin_esit(kosul, "")))
            {
                emit(kat3("if (", kosul, ") {"));
                indent_ekle(1LL);
                emit("fprintf(stderr, \"[u++] arkaplan: bellek yetersiz.\\n\");");
                uret_bg_metin_ctx_birak(caps, "_ctx");
                emit("free(_ctx);");
                indent_ekle((-1LL));
                emit("} else {");
                indent_ekle(1LL);
            }
            if (linux_mi())
            {
                emit(kat3("pthread_t _th; if (pthread_create(&_th, NULL, ", fn, ", _ctx) == 0) { pthread_detach(_th); } else {"));
            }
            else
            {
                emit(kat3("HANDLE _th = CreateThread(NULL, 0, ", fn, ", _ctx, 0, NULL);"));
                emit("if (_th) { CloseHandle(_th); } else {");
            }
            indent_ekle(1LL);
            emit("fprintf(stderr, \"[u++] arkaplan: is parcacigi baslatilamadi.\\n\");");
            uret_bg_metin_ctx_birak(caps, "_ctx");
            emit("free(_ctx);");
            indent_ekle((-1LL));
            emit("}");
            if ((!metin_esit(kosul, "")))
            {
                indent_ekle((-1LL));
                emit("}");
            }
            indent_ekle((-1LL));
            emit("} else {");
            indent_ekle(1LL);
            emit("fprintf(stderr, \"[u++] arkaplan: bellek yetersiz.\\n\");");
            indent_ekle((-1LL));
            emit("}");
        }
        else
        {
            if (linux_mi())
            {
                emit(kat3("pthread_t _th; if (pthread_create(&_th, NULL, ", fn, ", NULL) == 0) { pthread_detach(_th); } else {"));
            }
            else
            {
                emit(kat3("HANDLE _th = CreateThread(NULL, 0, ", fn, ", NULL, 0, NULL);"));
                emit("if (_th) { CloseHandle(_th); } else {");
            }
            indent_ekle(1LL);
            emit("fprintf(stderr, \"[u++] arkaplan: is parcacigi baslatilamadi.\\n\");");
            indent_ekle((-1LL));
            emit("}");
        }
        indent_ekle((-1LL));
        emit("}");
    }
}

void bg_yuru(long long n, long long kip) {
    {
        if ((n == 0LL))
        {
            return;
        }
        bg_yuru(dugum_a(n), kip);
        bg_yuru(dugum_b(n), kip);
        bg_yuru(dugum_c(n), kip);
        bg_yuru(dugum_d(n), kip);
        long long L = dugum_liste(n);
        long long i = 0LL;
        while ((i < liste_uzunluk(L)))
        {
            bg_yuru(liste_al(L, i), kip);
            (i = (i + 1LL));
        }
        (L = dugum_liste2(n));
        (i = 0LL);
        while ((i < liste_uzunluk(L)))
        {
            bg_yuru(liste_al(L, i), kip);
            (i = (i + 1LL));
        }
        long long t = dugum_tur(n);
        if (((t == DugumTur_D_ARKAPLAN) || (t == DugumTur_D_ARKAPLAN_IFADE)))
        {
            if ((kip == 0LL))
            {
                if ((dugum_sayi(n) == 0LL))
                {
                    dugum_yaz_sayi(n, bgid_yeni());
                }
            }
            else
            {
                uret_bg_isci(n);
            }
        }
    }
}

void uret_govde(long long stmt) {
    {
        if ((stmt == 0LL))
        {
            emit(";");
            return;
        }
        long long t = dugum_tur(stmt);
        if ((((((t == DugumTur_D_BLOK) || (t == DugumTur_D_GUVENSIZ)) || (t == DugumTur_D_GUVENLI)) || (t == DugumTur_D_SECIM)) || (t == DugumTur_D_HER)))
        {
            uret_cumle(stmt);
            return;
        }
        indent_ekle(1LL);
        uret_cumle(stmt);
        indent_ekle((-1LL));
    }
}

bool kilit_hazir_gerekli(long long d) {
    {
        long long ty = 0LL;
        if ((d == 0LL))
        {
            return false;
        }
        (ty = dugum_a(d));
        if ((ty == 0LL))
        {
            return false;
        }
        if ((!metin_esit(dugum_ad(ty), "Kilit")))
        {
            return false;
        }
        if ((dugum_ptrd(ty) != 0LL))
        {
            return false;
        }
        if ((dugum_c(d) != 0LL))
        {
            return false;
        }
        if ((dugum_b(d) != 0LL))
        {
            return false;
        }
        if (((dugum_bayrak(d) & 16LL) != 0LL))
        {
            return false;
        }
        return true;
    }
}

void uret_var(long long d, bool kuresel) {
    {
        long long ty = dugum_a(d);
        char* s = uret_ctur(ty);
        if (((dugum_bayrak(d) & 16LL) != 0LL))
        {
            (s = kat("const ", s));
        }
        (s = kat(s, " "));
        char* ad = dugum_ad(d);
        if (kuresel)
        {
            (ad = dugum_c_ad(d));
        }
        (s = kat(s, ad));
        if ((dugum_b(d) != 0LL))
        {
            (s = kat(s, "["));
            (s = kat(s, uret_ifade(dugum_b(d))));
            (s = kat(s, "]"));
        }
        else
            if ((((dugum_bayrak(d) & 8LL) != 0LL) || (dugum_dims(ty) > 0LL)))
            {
                (s = kat(s, "[]"));
            }
        bool metin_kopya = (((metin_esit(dugum_ad(ty), "metin") && (dugum_ptrd(ty) == 0LL)) && ((dugum_bayrak(d) & 16LL) == 0LL)) && (dugum_c(d) != 0LL));
        if ((kuresel && metin_kopya))
        {
            (s = kat(s, " = NULL"));
        }
        else
            if ((dugum_c(d) != 0LL))
            {
                if (((metin_kopya && (dugum_tur(dugum_c(d)) == DugumTur_D_SABIT)) && (dugum_ptrd(dugum_c(d)) == 2LL)))
                {
                    (s = kat(s, " = upp_metin_kopya("));
                    (s = kat(s, uret_ifade(dugum_c(d))));
                    (s = kat(s, ")"));
                }
                else
                {
                    (s = kat(s, " = "));
                    (s = kat(s, uret_ifade(dugum_c(d))));
                }
            }
            else
                if (((dugum_ptrd(ty) > 0LL) || metin_esit(dugum_ad(ty), "metin")))
                {
                    (s = kat(s, " = NULL"));
                }
                else
                    if ((liste_tur_mi(dugum_ad(ty)) || harita_tur_mi(dugum_ad(ty))))
                    {
                        (s = kat(s, " = {0}"));
                    }
                    else
                        if (((tablo_bul(0LL, dugum_ad(ty)) != 0LL) && (dugum_ptrd(ty) == 0LL)))
                        {
                            (s = kat(s, " = {0}"));
                        }
                        else
                            if ((tablo_bul(2LL, dugum_ad(ty)) != 0LL))
                            {
                                (s = kat(s, " = 0"));
                            }
        emit(kat(s, ";"));
        kapsam_bagla(ad, ty);
        if ((kuresel && (!metin_esit(ad, dugum_ad(d)))))
        {
            kapsam_bagla(dugum_ad(d), ty);
        }
        if (((!kuresel) && kilit_hazir_gerekli(d)))
        {
            emit(kat3("upp_kilit_hazir(&", ad, ");"));
        }
    }
}

void uret_cumle(long long stmt) {
    {
        if ((stmt == 0LL))
        {
            return;
        }
        long long t = dugum_tur(stmt);
        if ((t == DugumTur_D_BOS_CUMLE))
        {
            emit(";");
            return;
        }
        if ((t == DugumTur_D_BLOK))
        {
            kapsam_ac();
            emit("{");
            indent_ekle(1LL);
            long long L = dugum_liste(stmt);
            long long i = 0LL;
            while ((i < liste_uzunluk(L)))
            {
                uret_cumle(liste_al(L, i));
                (i = (i + 1LL));
            }
            indent_ekle((-1LL));
            emit("}");
            kapsam_kapat();
            return;
        }
        if ((t == DugumTur_D_GUVENSIZ))
        {
            emit("/* guvensiz */ {");
            indent_ekle(1LL);
            uret_cumle(dugum_a(stmt));
            indent_ekle((-1LL));
            emit("}");
            return;
        }
        if ((t == DugumTur_D_GUVENLI))
        {
            emit("/* guvenli */ {");
            indent_ekle(1LL);
            uret_cumle(dugum_a(stmt));
            indent_ekle((-1LL));
            emit("}");
            return;
        }
        if ((t == DugumTur_D_C_GOMMU))
        {
            emit("/* c_kod */");
            emit(dugum_ad(stmt));
            return;
        }
        if ((t == DugumTur_D_ASM_GOMMU))
        {
            emit("/* asm_kod */");
            emit("__asm__ __volatile__(\"\");");
            return;
        }
        if ((t == DugumTur_D_DEGISKEN))
        {
            uret_var(stmt, false);
            return;
        }
        if ((t == DugumTur_D_EGER))
        {
            emit(kat3("if (", uret_ifade(dugum_a(stmt)), ")"));
            uret_govde(dugum_b(stmt));
            if ((dugum_c(stmt) != 0LL))
            {
                emit("else");
                uret_govde(dugum_c(stmt));
            }
            return;
        }
        if ((t == DugumTur_D_IKEN))
        {
            emit(kat3("while (", uret_ifade(dugum_a(stmt)), ")"));
            uret_govde(dugum_b(stmt));
            return;
        }
        if ((t == DugumTur_D_DONGU))
        {
            kapsam_ac();
            if (((dugum_a(stmt) != 0LL) && (dugum_tur(dugum_a(stmt)) == DugumTur_D_DEGISKEN)))
            {
                kapsam_bagla(dugum_ad(dugum_a(stmt)), dugum_a(dugum_a(stmt)));
            }
            char* s = upp_metin_kopya("for (");
            if (((dugum_a(stmt) != 0LL) && (dugum_tur(dugum_a(stmt)) == DugumTur_D_DEGISKEN)))
            {
                (s = kat(s, uret_ctur(dugum_a(dugum_a(stmt)))));
                (s = kat(s, " "));
                (s = kat(s, dugum_ad(dugum_a(stmt))));
                if ((dugum_c(dugum_a(stmt)) != 0LL))
                {
                    (s = kat(s, " = "));
                    (s = kat(s, uret_ifade(dugum_c(dugum_a(stmt)))));
                }
            }
            else
                if (((dugum_a(stmt) != 0LL) && (dugum_tur(dugum_a(stmt)) == DugumTur_D_IFADE_CUMLE)))
                {
                    (s = kat(s, uret_ifade(dugum_a(dugum_a(stmt)))));
                }
            (s = kat(s, "; "));
            (s = kat(s, uret_ifade(dugum_b(stmt))));
            (s = kat(s, "; "));
            (s = kat(s, uret_ifade(dugum_c(stmt))));
            (s = kat(s, ")"));
            emit(s);
            uret_govde(dugum_d(stmt));
            kapsam_kapat();
            return;
        }
        if ((t == DugumTur_D_SECIM))
        {
            long long sw_t = tip_ifade(dugum_a(stmt));
            if (tip_metin_mi(sw_t))
            {
                long long swid = tmp_yeni();
                char* swv = kat("_upp_sw", sayi_metin(swid));
                emit("{");
                indent_ekle(1LL);
                emit(kat(kat3("char* ", swv, " = "), kat(uret_ifade(dugum_a(stmt)), ";")));
                long long L = dugum_liste(stmt);
                long long i = 0LL;
                bool first = true;
                while ((i < liste_uzunluk(L)))
                {
                    long long c = liste_al(L, i);
                    if ((dugum_a(c) == 0LL))
                    {
                        emit("else");
                    }
                    else
                    {
                        char* cond = kat4("upp_metin_esit(", swv, ", ", kat(uret_ifade(dugum_a(c)), ")"));
                        if (first)
                        {
                            emit(kat3("if (", cond, ")"));
                        }
                        else
                        {
                            emit(kat3("else if (", cond, ")"));
                        }
                        (first = false);
                    }
                    emit("{");
                    indent_ekle(1LL);
                    long long sl = dugum_liste(c);
                    long long j = 0LL;
                    while ((j < liste_uzunluk(sl)))
                    {
                        long long s = liste_al(sl, j);
                        if ((dugum_tur(s) != DugumTur_D_DUR))
                        {
                            uret_cumle(s);
                        }
                        (j = (j + 1LL));
                    }
                    indent_ekle((-1LL));
                    emit("}");
                    (i = (i + 1LL));
                }
                indent_ekle((-1LL));
                emit("}");
                return;
            }
            emit(kat3("switch (", uret_ifade(dugum_a(stmt)), ") {"));
            indent_ekle(1LL);
            long long L = dugum_liste(stmt);
            long long i = 0LL;
            while ((i < liste_uzunluk(L)))
            {
                long long c = liste_al(L, i);
                if ((dugum_a(c) == 0LL))
                {
                    emit("default:");
                }
                else
                {
                    emit(kat3("case ", uret_ifade(dugum_a(c)), ":"));
                }
                indent_ekle(1LL);
                long long sl = dugum_liste(c);
                long long j = 0LL;
                while ((j < liste_uzunluk(sl)))
                {
                    uret_cumle(liste_al(sl, j));
                    (j = (j + 1LL));
                }
                indent_ekle((-1LL));
                (i = (i + 1LL));
            }
            indent_ekle((-1LL));
            emit("}");
            return;
        }
        if ((t == DugumTur_D_HER))
        {
            long long idx = tmp_yeni();
            char* ix = kat("_upp_i", sayi_metin(idx));
            char* arr = uret_ifade(dugum_b(stmt));
            long long it_t = tip_ifade(dugum_b(stmt));
            kapsam_ac();
            kapsam_bagla(dugum_ad(stmt), dugum_a(stmt));
            emit("{");
            indent_ekle(1LL);
            emit(kat3("long long ", ix, ";"));
            char* s = upp_metin_kopya("");
            if (liste_tur_mi(dugum_ad(it_t)))
            {
                (s = "for (");
                (s = kat(s, ix));
                (s = kat(s, " = 0; "));
                (s = kat(s, ix));
                (s = kat(s, " < upp_liste_uzunluk("));
                (s = kat(s, arr));
                (s = kat(s, "); "));
                (s = kat(s, ix));
                (s = kat(s, "++) {"));
                emit(s);
                indent_ekle(1LL);
                (s = uret_ctur(dugum_a(stmt)));
                (s = kat(s, " "));
                (s = kat(s, dugum_ad(stmt)));
                (s = kat(s, " = "));
                (s = kat(s, uret_liste_al(liste_elem_ad(dugum_ad(it_t)), arr, ix)));
                (s = kat(s, ";"));
                emit(s);
            }
            else
                if (harita_tur_mi(dugum_ad(it_t)))
                {
                    char* ktur = harita_anahtar_ad(dugum_ad(it_t));
                    long long ktag = koleksiyon_etiket(ktur);
                    long long kid = tmp_yeni();
                    char* keys = kat("_upp_hk", sayi_metin(kid));
                    emit(kat(kat5("UppKolListe ", keys, " = upp_harita_anahtarlar(", arr, ", "), kat(sayi_metin(ktag), ");")));
                    (s = "for (");
                    (s = kat(s, ix));
                    (s = kat(s, " = 0; "));
                    (s = kat(s, ix));
                    (s = kat(s, " < upp_liste_uzunluk("));
                    (s = kat(s, keys));
                    (s = kat(s, "); "));
                    (s = kat(s, ix));
                    (s = kat(s, "++) {"));
                    emit(s);
                    indent_ekle(1LL);
                    (s = uret_ctur(dugum_a(stmt)));
                    (s = kat(s, " "));
                    (s = kat(s, dugum_ad(stmt)));
                    (s = kat(s, " = "));
                    (s = kat(s, uret_liste_al(ktur, keys, ix)));
                    (s = kat(s, ";"));
                    emit(s);
                    uret_govde(dugum_c(stmt));
                    indent_ekle((-1LL));
                    emit("}");
                    emit(kat3("upp_liste_bosalt(&(", keys, "));"));
                    indent_ekle((-1LL));
                    emit("}");
                    kapsam_kapat();
                    return;
                }
                else
                {
                    (s = "for (");
                    (s = kat(s, ix));
                    (s = kat(s, " = 0; "));
                    (s = kat(s, ix));
                    (s = kat(s, " < (long long)(sizeof("));
                    (s = kat(s, arr));
                    (s = kat(s, ") / sizeof(("));
                    (s = kat(s, arr));
                    (s = kat(s, ")[0])); "));
                    (s = kat(s, ix));
                    (s = kat(s, "++) {"));
                    emit(s);
                    indent_ekle(1LL);
                    (s = uret_ctur(dugum_a(stmt)));
                    (s = kat(s, " "));
                    (s = kat(s, dugum_ad(stmt)));
                    (s = kat(s, " = "));
                    (s = kat(s, arr));
                    (s = kat(s, "["));
                    (s = kat(s, ix));
                    (s = kat(s, "];"));
                    emit(s);
                }
            uret_govde(dugum_c(stmt));
            indent_ekle((-1LL));
            emit("}");
            indent_ekle((-1LL));
            emit("}");
            kapsam_kapat();
            return;
        }
        if ((t == DugumTur_D_DON))
        {
            if ((bg_al() > 0LL))
            {
                emit("return 0;");
                return;
            }
            long long fn = cur_fn_al();
            bool ana = (((fn != 0LL) && metin_esit(dugum_ad(fn), "ana")) && (dugum_c(fn) == 0LL));
            if ((dugum_a(stmt) == 0LL))
            {
                if (ana)
                {
                    emit("return 0;");
                }
                else
                {
                    emit("return;");
                }
            }
            else
                if (ana)
                {
                    emit(kat3("return (int)(", uret_ifade(dugum_a(stmt)), ");"));
                }
                else
                {
                    emit(kat3("return ", uret_ifade(dugum_a(stmt)), ";"));
                }
            return;
        }
        if ((t == DugumTur_D_DUR))
        {
            emit("break;");
            return;
        }
        if ((t == DugumTur_D_DEVAM))
        {
            emit("continue;");
            return;
        }
        if ((t == DugumTur_D_IFADE_CUMLE))
        {
            emit(kat(uret_ifade(dugum_a(stmt)), ";"));
            return;
        }
        if ((t == DugumTur_D_ARKAPLAN))
        {
            uret_bg_deyim(stmt);
            return;
        }
    }
}

char* uret_prototip(long long decl) {
    {
        if ((dugum_c(decl) != 0LL))
        {
            char* cn = metot_sinif_c_ad(decl);
            char* s = uret_ctur(dugum_a(decl));
            (s = kat(s, " "));
            (s = kat(s, cn));
            (s = kat(s, "_"));
            (s = kat(s, dugum_ad(decl)));
            (s = kat(s, "("));
            (s = kat(s, cn));
            (s = kat(s, "* kendi"));
            long long P = dugum_liste(decl);
            long long i = 0LL;
            while ((i < liste_uzunluk(P)))
            {
                long long p = liste_al(P, i);
                (s = kat(s, ", "));
                (s = kat(s, uret_ctur(dugum_a(p))));
                (s = kat(s, " "));
                (s = kat(s, dugum_ad(p)));
                (i = (i + 1LL));
            }
            return kat(s, ")");
        }
        if ((metin_esit(dugum_ad(decl), "ana") && metin_esit(dugum_alias_ad(decl), "")))
        {
            return "int main(int argc, char **argv)";
        }
        char* s = uret_ctur(dugum_a(decl));
        (s = kat(s, " "));
        (s = kat(s, dugum_c_ad(decl)));
        (s = kat(s, "("));
        long long P = dugum_liste(decl);
        if ((liste_uzunluk(P) == 0LL))
        {
            (s = kat(s, "void"));
        }
        else
        {
            long long i = 0LL;
            while ((i < liste_uzunluk(P)))
            {
                if ((i > 0LL))
                {
                    (s = kat(s, ", "));
                }
                long long p = liste_al(P, i);
                (s = kat(s, uret_ctur(dugum_a(p))));
                (s = kat(s, " "));
                (s = kat(s, dugum_ad(p)));
                (i = (i + 1LL));
            }
        }
        return kat(s, ")");
    }
}

void uret_fonksiyon(long long decl) {
    {
        cur_fn_yaz(decl);
        kapsam_sifirla();
        kapsam_ac();
        if ((dugum_c(decl) != 0LL))
        {
            char* stad = metot_sinif_tur_ad(decl);
            cur_class_yaz(stad);
            kapsam_bagla("kendi", tur_dugum(dugum_satir(decl), dugum_sutun(decl), stad, 1LL, 0LL));
        }
        else
        {
            cur_class_yaz("");
        }
        long long P = dugum_liste(decl);
        long long pi = 0LL;
        while ((pi < liste_uzunluk(P)))
        {
            long long p = liste_al(P, pi);
            kapsam_bagla(dugum_ad(p), dugum_a(p));
            (pi = (pi + 1LL));
        }
        emit(kat(uret_prototip(decl), " {"));
        indent_ekle(1LL);
        if ((metin_esit(dugum_ad(decl), "ana") && (dugum_c(decl) == 0LL)))
        {
            emit("upp_argv_ayarla(argc, argv);");
            emit("_upp_kuresel_hazir();");
            emit("#if defined(_WIN32)");
            emit("SetConsoleOutputCP(CP_UTF8);");
            emit("SetConsoleCP(CP_UTF8);");
            emit("#endif");
        }
        uret_cumle(dugum_b(decl));
        indent_ekle((-1LL));
        emit("}");
        emit("");
        cur_fn_yaz(0LL);
        cur_class_yaz("");
    }
}

void uret_program(long long program) {
    {
        tampon_sifirla();
        tmp_sifirla();
        if (linux_mi())
        {
            emit("#define UPP_HEDEF_LINUX 1");
            emit("#ifndef _FILE_OFFSET_BITS");
            emit("#define _FILE_OFFSET_BITS 64");
            emit("#endif");
            emit("#ifndef _DEFAULT_SOURCE");
            emit("#define _DEFAULT_SOURCE 1");
            emit("#endif");
            emit("#ifndef _POSIX_C_SOURCE");
            emit("#define _POSIX_C_SOURCE 200809L");
            emit("#endif");
        }
        else
        {
            emit("#define UPP_HEDEF_WINDOWS 1");
        }
        emit("#include <stdio.h>");
        emit("#include <stdlib.h>");
        emit("#include <stdbool.h>");
        emit("#include <string.h>");
        emit("#include <limits.h>");
        emit("#include <stdint.h>");
        emit("#include <math.h>");
        if (linux_mi())
        {
            emit("#include <pthread.h>");
            emit("#include <unistd.h>");
            emit("#include <time.h>");
            emit("#include <fcntl.h>");
            emit("#include <errno.h>");
            emit("#include <sys/types.h>");
            emit("#include <sys/stat.h>");
        }
        else
        {
            emit("#include <windows.h>");
            emit("#include <mmsystem.h>");
            emit("#include <tlhelp32.h>");
            emit("#include <io.h>");
            emit("#include <sys/stat.h>");
        }
        long long decls = dugum_liste(program);
        long long i = 0LL;
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_CEKLE))
            {
                char* spec = dugum_ad(d);
                hata_fmt_basla();
                hata_fmt_yaz("#include ");
                if (((metin_cp(spec, 0LL) == 60LL) || (metin_cp(spec, 0LL) == 34LL)))
                {
                    hata_fmt_yaz(spec);
                }
                else
                {
                    hata_fmt_yaz("\"");
                    hata_fmt_yaz(spec);
                    hata_fmt_yaz("\"");
                }
                emit(intern(hata_fmt_al()));
            }
            (i = (i + 1LL));
        }
        emit("");
        char* runtime = os_runtime_oku();
        if ((!metin_esit(runtime, "")))
        {
            emit(runtime);
        }
        else
        {
            os_runtime_gomulu_yay();
        }
        emit("");
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_SECENEK))
            {
                hata_fmt_basla();
                hata_fmt_yaz("typedef enum ");
                hata_fmt_yaz(dugum_c_ad(d));
                hata_fmt_yaz(" {");
                emit(intern(hata_fmt_al()));
                indent_ekle(1LL);
                long long L = dugum_liste(d);
                long long j = 0LL;
                while ((j < liste_uzunluk(L)))
                {
                    long long v = liste_al(L, j);
                    char* ev = kat5(dugum_c_ad(d), "_", dugum_ad(v), " = ", sayi_metin(dugum_sayi(v)));
                    if ((j < (liste_uzunluk(L) - 1LL)))
                    {
                        (ev = kat(ev, ","));
                    }
                    emit(ev);
                    (j = (j + 1LL));
                }
                indent_ekle((-1LL));
                hata_fmt_basla();
                hata_fmt_yaz("} ");
                hata_fmt_yaz(dugum_c_ad(d));
                hata_fmt_yaz(";");
                emit(intern(hata_fmt_al()));
                emit("");
            }
            (i = (i + 1LL));
        }
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_SINIF))
            {
                hata_fmt_basla();
                hata_fmt_yaz("typedef struct ");
                hata_fmt_yaz(dugum_c_ad(d));
                hata_fmt_yaz(" ");
                hata_fmt_yaz(dugum_c_ad(d));
                hata_fmt_yaz(";");
                emit(intern(hata_fmt_al()));
            }
            (i = (i + 1LL));
        }
        emit("");
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_SINIF))
            {
                hata_fmt_basla();
                hata_fmt_yaz("struct ");
                hata_fmt_yaz(dugum_c_ad(d));
                hata_fmt_yaz(" {");
                emit(intern(hata_fmt_al()));
                indent_ekle(1LL);
                long long F = dugum_liste(d);
                if ((liste_uzunluk(F) == 0LL))
                {
                    emit("char _u_bos;");
                }
                else
                {
                    long long j = 0LL;
                    while ((j < liste_uzunluk(F)))
                    {
                        long long f = liste_al(F, j);
                        char* fs = uret_ctur(dugum_a(f));
                        (fs = kat(fs, " "));
                        (fs = kat(fs, dugum_ad(f)));
                        if ((dugum_b(f) != 0LL))
                        {
                            (fs = kat(fs, "["));
                            (fs = kat(fs, uret_ifade(dugum_b(f))));
                            (fs = kat(fs, "]"));
                        }
                        emit(kat(fs, ";"));
                        (j = (j + 1LL));
                    }
                }
                indent_ekle((-1LL));
                emit("};");
                emit("");
            }
            (i = (i + 1LL));
        }
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_SINIF))
            {
                long long holder = dugum_c(d);
                if ((holder != 0LL))
                {
                    long long CL = dugum_liste(holder);
                    long long j = 0LL;
                    while ((j < liste_uzunluk(CL)))
                    {
                        long long c = liste_al(CL, j);
                        char* cs = upp_metin_kopya("static const ");
                        (cs = kat(cs, uret_ctur(dugum_a(c))));
                        (cs = kat(cs, " "));
                        (cs = kat(cs, dugum_c_ad(d)));
                        (cs = kat(cs, "_"));
                        (cs = kat(cs, dugum_ad(c)));
                        if ((dugum_c(c) != 0LL))
                        {
                            (cs = kat(cs, " = "));
                            (cs = kat(cs, uret_ifade(dugum_c(c))));
                        }
                        emit(kat(cs, ";"));
                        (j = (j + 1LL));
                    }
                }
                long long M = dugum_liste2(d);
                long long mj = 0LL;
                while ((mj < liste_uzunluk(M)))
                {
                    emit(kat(uret_prototip(liste_al(M, mj)), ";"));
                    (mj = (mj + 1LL));
                }
            }
            (i = (i + 1LL));
        }
        emit("/* ---- prototipler ---- */");
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_FONK))
            {
                emit(kat(uret_prototip(d), ";"));
            }
            (i = (i + 1LL));
        }
        emit("");
        emit("/* ---- u++ üst düzey değişkenler ---- */");
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_DEGISKEN))
            {
                emit("static");
                uret_var(d, true);
            }
            (i = (i + 1LL));
        }
        emit("static void _upp_kuresel_hazir(void) {");
        indent_ekle(1LL);
        long long hazir = 0LL;
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_DEGISKEN))
            {
                long long ty = dugum_a(d);
                if ((((metin_esit(dugum_ad(ty), "metin") && (dugum_ptrd(ty) == 0LL)) && ((dugum_bayrak(d) & 16LL) == 0LL)) && (dugum_c(d) != 0LL)))
                {
                    emit(kat(kat3(dugum_c_ad(d), " = upp_metin_kopya(", uret_ifade(dugum_c(d))), ");"));
                    (hazir = (hazir + 1LL));
                }
                if (kilit_hazir_gerekli(d))
                {
                    emit(kat3("upp_kilit_hazir(&", dugum_c_ad(d), ");"));
                    (hazir = (hazir + 1LL));
                }
            }
            (i = (i + 1LL));
        }
        if ((hazir == 0LL))
        {
            emit("(void)0;");
        }
        indent_ekle((-1LL));
        emit("}");
        emit("");
        bg_yuru(program, 0LL);
        bg_yigin_sifirla();
        long long kayit_indent = indent_al();
        indent_kur(0LL);
        emit_hedef_yaz(1LL);
        bg_yuru(program, 1LL);
        emit_hedef_yaz(0LL);
        indent_kur(kayit_indent);
        if ((metin_uzun(bg_yigin_al()) > 0LL))
        {
            emit("/* ---- u++ arkaplan ---- */");
            tampon_yaz(bg_yigin_al());
            tampon_yaz("\n");
        }
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_SINIF))
            {
                long long M = dugum_liste2(d);
                long long j = 0LL;
                while ((j < liste_uzunluk(M)))
                {
                    uret_fonksiyon(liste_al(M, j));
                    (j = (j + 1LL));
                }
            }
            (i = (i + 1LL));
        }
        (i = 0LL);
        while ((i < liste_uzunluk(decls)))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_FONK))
            {
                uret_fonksiyon(d);
            }
            (i = (i + 1LL));
        }
    }
}

void os_runtime_gomulu_yay(void) {
    {
        tampon_yaz(upp_metin_hex_coz("2f2a203d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d0d0a202a20752b2b2076322e3820c3a7616cc4b1c59f6d61207a616d616ec4b120e28094207570702e2a207374646c69620d0a202a0d0a202a205550505f48454445465f4c494e5558202f205550505f48454445465f57494e444f5753206465726c657969636920287570706329207461726166c4b16e64616e20626173c4b16cc4b1722e0d0a202a20486f7374206d616b726f205f57494e3332206865646566692079616e73c4b1746d617a3a204d696e475720696c65202d2d6c696e7578206465726c656e696e63650d0a202a2057696e646f777320415049276c6572692077696e646f77732e68206f6c6d6164616e20c3a761c49f72c4b16cc4b1796f7264752e0d0a202a203d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d3d202a2f0d0a0d0a23696620646566696e6564285550505f48454445465f4c494e5558290d0a23646566696e65205550505f57494e20300d0a23656c696620646566696e6564285550505f48454445465f57494e444f5753290d0a23646566696e65205550505f57494e20310d0a23656c696620646566696e6564285f57494e3332290d0a23646566696e65205550505f57494e20310d0a23656c73650d0a23646566696e65205550505f57494e20300d0a23656e6469660d0a0d0a23696620215550505f57494e0d0a2369666e646566205f46494c455f4f46465345545f424954530d0a23646566696e65205f46494c455f4f46465345545f424954532036340d0a23656e6469660d0a23656e6469660d0a0d0a23696e636c756465203c737464696e742e683e0d0a23696e636c756465203c737464626f6f6c2e683e0d0a23696e636c756465203c6c696d6974732e683e0d0a23696e636c756465203c737464696f2e683e0d0a23696e636c756465203c7374646c69622e683e0d0a747970656465662075696e74385f7420626179743b0d0a0d0a23696620646566696e6564285f4d53435f564552290d0a23646566696e65205550505f544c53205f5f6465636c7370656328746872656164290d0a23656c73650d0a23646566696e65205550505f544c53205f5f7468726561640d0a23656e6469660d0a0d0a73746174696320766f6964207570705f70616e696b28636f6e737420636861722a206d6573616a29207b0d0a20202020667072696e7466287374646572722c20225b752b2b20484154415d2025735c6e222c206d6573616a203f206d6573616a203a202269c3a720686174612229"));
        tampon_yaz(upp_metin_hex_coz("3b0d0a2020202066666c75736828737464657272293b0d0a2020202061626f727428293b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f64697a695f696478286c6f6e67206c6f6e6720692c206c6f6e67206c6f6e67206e29207b0d0a202020206966202869203c2030207c7c2069203e3d206e29207b0d0a2020202020202020667072696e7466287374646572722c20225b752b2b20484154415d20696e64656b7320256c6c642c2064697a6920626f7975747520256c6c642779692061c59fc4b1796f725c6e222c20692c206e293b0d0a202020202020202066666c75736828737464657272293b0d0a202020202020202061626f727428293b0d0a202020207d0d0a2020202072657475726e20693b0d0a7d0d0a0d0a23696e636c756465203c6572726e6f2e683e0d0a23696e636c756465203c737472696e672e683e0d0a23696e636c756465203c6d6174682e683e0d0a236966205550505f57494e0d0a23696e636c756465203c77696e646f77732e683e0d0a23696e636c756465203c6d6d73797374656d2e683e0d0a23696e636c756465203c746c68656c7033322e683e0d0a23696e636c756465203c696f2e683e0d0a23696e636c756465203c7379732f737461742e683e0d0a23656c73650d0a23696e636c756465203c756e697374642e683e0d0a23696e636c756465203c7379732f737461742e683e0d0a23696e636c756465203c7379732f74797065732e683e0d0a23696e636c756465203c646972656e742e683e0d0a23696e636c756465203c6c696d6974732e683e0d0a23696e636c756465203c707468726561642e683e0d0a23696e636c756465203c74696d652e683e0d0a23696e636c756465203c66636e746c2e683e0d0a23696620646566696e6564285f5f6861735f696e636c756465290d0a2320206966205f5f6861735f696e636c756465283c7379732f776169742e683e290d0a2320202020696e636c756465203c7379732f776169742e683e0d0a232020656e6469660d0a23656c73650d0a232020696e636c756465203c7379732f776169742e683e0d0a23656e6469660d0a23656e6469660d0a0d0a73746174696320696e74205f7570705f72616e645f68617a6972203d20303b0d0a0d0a236966205550505f57494e0d0a73746174696320766f6964205f7570705f72616e645f6261736c617428766f696429207b0d0a20202020756e7369676e656420736565643b0d0a202020204c415247455f494e5445474552207170633b0d0a20202020696620285f7570705f72616e645f68617a697229207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020202f2a204765745469636b436f756e74203130e280933136206d733b20504944202b205150432069"));
        tampon_yaz(upp_metin_hex_coz("6c652061796ec4b12074696b746520696b692073c3bc726563696e2061796ec4b120746f68756d7520616c6d6173c4b1207a6f726c61c59fc4b1722e202a2f0d0a2020202073656564203d2028756e7369676e6564294765745469636b436f756e7428293b0d0a2020202073656564205e3d2028756e7369676e65642947657443757272656e7450726f6365737349642829202a2032363534343335373631753b0d0a20202020696620285175657279506572666f726d616e6365436f756e74657228267170632929207b0d0a202020202020202073656564205e3d2028756e7369676e6564297170632e4c6f77506172743b0d0a202020202020202073656564205e3d2028756e7369676e6564297170632e48696768506172743b0d0a202020207d0d0a202020207372616e642873656564203f2073656564203a203175293b0d0a202020205f7570705f72616e645f68617a6972203d20313b0d0a7d0d0a23656c73650d0a73746174696320766f6964205f7570705f72616e645f6261736c617428766f696429207b0d0a20202020756e7369676e656420736565643b0d0a202020207374727563742074696d65737065632074733b0d0a20202020696620285f7570705f72616e645f68617a697229207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202073656564203d2028756e7369676e65642974696d65284e554c4c293b0d0a2020202073656564205e3d2028756e7369676e6564296765747069642829202a2032363534343335373631753b0d0a2020202069662028636c6f636b5f67657474696d6528434c4f434b5f4d4f4e4f544f4e49432c2026747329203d3d203029207b0d0a202020202020202073656564205e3d2028756e7369676e65642974732e74765f6e7365633b0d0a202020202020202073656564205e3d2028756e7369676e6564292874732e74765f736563202620307866666666293b0d0a202020207d0d0a202020207372616e642873656564203f2073656564203a203175293b0d0a202020205f7570705f72616e645f68617a6972203d20313b0d0a7d0d0a23656e6469660d0a0d0a2f2a2043207374616e64617264c4b12052414e445f4d4158203e3d2033323736372028313520626974292e2057696e646f7773204d696e47572f4d5356432764652074616d2033323736372764c4b1723a0d0a202a2074656b2072616e64282920696c65207570702e7261737467656c6528302c20313030303030292061736c61203332373637276e696e20c3bc7374c3bc6e6520c3a7c4b16b616d617a64c4b12e0d0a202a203135206269746c696b20706172c3a7616c6172c4b1206b617964c4b172c4b1702036342062697420646f6c64757275796f72757a2e202a2f0d0a73746174696320"));
        tampon_yaz(upp_metin_hex_coz("756e7369676e6564206c6f6e67206c6f6e67205f7570705f72616e645f75363428766f696429207b0d0a20202020756e7369676e6564206c6f6e67206c6f6e672072203d20303b0d0a20202020696e742066696c6c6564203d20303b0d0a202020207768696c65202866696c6c6564203c20363429207b0d0a2020202020202020756e7369676e6564206c6f6e67206c6f6e67206368756e6b203d2028756e7369676e6564206c6f6e67206c6f6e672972616e642829202620307837464646756c6c3b0d0a202020202020202072207c3d206368756e6b203c3c2066696c6c65643b0d0a202020202020202066696c6c6564202b3d2031353b0d0a202020207d0d0a2020202072657475726e20723b0d0a7d0d0a0d0a236966205550505f57494e0d0a73746174696320766f6964207570705f6d6573616a28636f6e737420636861722a206261736c696b2c20636f6e737420636861722a206d6574696e29207b0d0a202020204d657373616765426f7841284e554c4c2c206d6574696e203f206d6574696e203a2022222c206261736c696b203f206261736c696b203a2022222c204d425f4f4b207c204d425f49434f4e494e464f524d4154494f4e293b0d0a7d0d0a0d0a73746174696320766f6964207570705f6861746128636f6e737420636861722a206261736c696b2c20636f6e737420636861722a206d6574696e29207b0d0a202020204d657373616765426f7841284e554c4c2c206d6574696e203f206d6574696e203a2022222c206261736c696b203f206261736c696b203a2022222c204d425f4f4b207c204d425f49434f4e4552524f52293b0d0a7d0d0a0d0a73746174696320696e74207570705f7365735f63616c28636f6e737420636861722a20646f7379615f796f6c7529207b0d0a202020206966202821646f7379615f796f6c7529207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e20506c6179536f756e644128646f7379615f796f6c752c204e554c4c2c20534e445f46494c454e414d45207c20534e445f4153594e43207c20534e445f4e4f44454641554c5429203f2031203a20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f75797574286c6f6e67206c6f6e67206d7329207b0d0a202020202f2a20536c656570283078464646464646464629203d20494e46494e4954453b2044574f5244206b65736d65736920736f6e73757a2062656b6c656d6579652064c3b66ec3bcc59f6d6573696e2e202a2f0d0a20202020696620286d73203c203029207b0d0a20202020202020206d73203d20303b0d0a202020207d0d0a202020206966202828756e7369676e6564206c6f6e67206c6f6e67296d73203e3d2030786666666666666665756c6c2920"));
        tampon_yaz(upp_metin_hex_coz("7b0d0a2020202020202020536c6565702830786666666666666665756c293b0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020536c656570282844574f5244296d73293b0d0a7d0d0a23656c73650d0a73746174696320766f6964207570705f6d6573616a28636f6e737420636861722a206261736c696b2c20636f6e737420636861722a206d6574696e29207b0d0a20202020667072696e7466287374646572722c20225b752b2b204d4553414a5d2025733a2025735c6e222c206261736c696b203f206261736c696b203a2022222c206d6574696e203f206d6574696e203a202222293b0d0a7d0d0a0d0a73746174696320766f6964207570705f6861746128636f6e737420636861722a206261736c696b2c20636f6e737420636861722a206d6574696e29207b0d0a20202020667072696e7466287374646572722c20225b752b2b20484154415d2025733a2025735c6e222c206261736c696b203f206261736c696b203a2022222c206d6574696e203f206d6574696e203a202222293b0d0a7d0d0a0d0a73746174696320696e74207570705f7365735f63616c28636f6e737420636861722a20646f7379615f796f6c7529207b0d0a202020202f2a204c696e75783a2073657320c3a7616c6d6120616c74796170c4b173c4b120796f6b3b2073657373697a6365207975742e202a2f0d0a2020202028766f696429646f7379615f796f6c753b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f75797574286c6f6e67206c6f6e67206d7329207b0d0a202020207374727563742074696d65737065632074733b0d0a20202020696620286d73203c203029207b0d0a20202020202020206d73203d20303b0d0a202020207d0d0a20202020696620286d73203e20313030303030303030304c4c29207b0d0a20202020202020206d73203d20313030303030303030304c4c3b0d0a202020207d0d0a2020202074732e74765f736563203d202874696d655f7429286d73202f2031303030293b0d0a2020202074732e74765f6e736563203d20286c6f6e672928286d732025203130303029202a20313030303030304c293b0d0a202020207768696c6520286e616e6f736c656570282674732c202674732920213d203029207b0d0a2020202020202020696620286572726e6f20213d2045494e545229207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020207d0d0a7d0d0a23656e6469660d0a0d0a737461746963206c6f6e67206c6f6e67207570705f7261737467656c65286c6f6e67206c6f6e67206d696e692c206c6f6e67206c6f6e67206d61786929207b0d0a20202020756e7369676e6564206c6f6e67206c6f6e6720"));
        tampon_yaz(upp_metin_hex_coz("756d696e2c20756d61782c207370616e2c207069636b2c206c696d69743b0d0a202020205f7570705f72616e645f6261736c617428293b0d0a20202020696620286d617869203c206d696e6929207b0d0a20202020202020206c6f6e67206c6f6e672074203d206d696e693b0d0a20202020202020206d696e69203d206d6178693b0d0a20202020202020206d617869203d20743b0d0a202020207d0d0a202020202f2a20c4b06d7a6173c4b17a206661726b3a20286d617869202d206d696e6929207369676e6564207461c59f6d6173c4b16ec4b120c3b66e6c65722e202a2f0d0a20202020756d696e203d2028756e7369676e6564206c6f6e67206c6f6e67296d696e693b0d0a20202020756d6178203d2028756e7369676e6564206c6f6e67206c6f6e67296d6178693b0d0a202020207370616e203d20756d6178202d20756d696e202b2031756c6c3b0d0a20202020696620287370616e203d3d2030756c6c29207b0d0a20202020202020202f2a205b4c4c4f4e475f4d494e2c204c4c4f4e475f4d41585d2074616d20325e3634206465c49f65722e202a2f0d0a202020202020202072657475726e20286c6f6e67206c6f6e67295f7570705f72616e645f75363428293b0d0a202020207d0d0a202020202f2a20325e363427c3bc207370616e27652062c3b66cc3bc6e6d6579656e20617274c4b16b2c20616c74207461726166612079c4b1c49fc4b16c6d6173c4b16e2064697965207265646465742e202a2f0d0a202020206c696d6974203d2028307866666666666666666666666666666666756c6c202f207370616e29202a207370616e3b0d0a20202020646f207b0d0a20202020202020207069636b203d205f7570705f72616e645f75363428293b0d0a202020207d207768696c6520287069636b203e3d206c696d6974293b0d0a202020207069636b20253d207370616e3b0d0a2020202072657475726e20286c6f6e67206c6f6e672928756d696e202b207069636b293b0d0a7d0d0a0d0a73746174696320636861722a207570705f646f7379615f6f6b7528636f6e737420636861722a20796f6c29207b0d0a2020202046494c452a20663b0d0a202020206c6f6e67206c6f6e672073697a653b0d0a20202020636861722a206275663b0d0a2020202073697a655f74206e3b0d0a202020206966202821796f6c29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202066203d20666f70656e28796f6c2c2022726222293b0d0a2020202069662028216629207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a236966205550505f57494e0d0a20202020696620285f667365656b69363428662c20302c205345454b5f454e442920213d203029207b0d"));
        tampon_yaz(upp_metin_hex_coz("0a202020202020202066636c6f73652866293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202073697a65203d205f6674656c6c6936342866293b0d0a23656c73650d0a2020202069662028667365656b6f28662c20302c205345454b5f454e442920213d203029207b0d0a202020202020202066636c6f73652866293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202073697a65203d20286c6f6e67206c6f6e67296674656c6c6f2866293b0d0a23656e6469660d0a202020206966202873697a65203c203029207b0d0a202020202020202066636c6f73652866293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020726577696e642866293b0d0a20202020627566203d2028636861722a296d616c6c6f63282873697a655f742973697a65202b203175293b0d0a20202020696620282162756629207b0d0a202020202020202066636c6f73652866293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206e203d206672656164286275662c20312c202873697a655f742973697a652c2066293b0d0a202020206275665b6e5d203d20275c30273b0d0a2020202066636c6f73652866293b0d0a2020202072657475726e206275663b202f2a207570702e6d6574696e5f626f73616c74202f2062656c6c656b5f626f73616c7420696c652062c4b172616bc4b16e202a2f0d0a7d0d0a0d0a236966205550505f57494e0d0a73746174696320626f6f6c207570705f646f7379615f7661725f6d6928636f6e737420636861722a20796f6c29207b0d0a2020202044574f524420617474723b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a202020202020202072657475726e2066616c73653b0d0a202020207d0d0a2020202061747472203d2047657446696c65417474726962757465734128796f6c293b0d0a2020202072657475726e206174747220213d20494e56414c49445f46494c455f415454524942555445533b0d0a7d0d0a23656c73650d0a73746174696320626f6f6c207570705f646f7379615f7661725f6d6928636f6e737420636861722a20796f6c29207b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a202020202020202072657475726e2066616c73653b0d0a202020207d0d0a2020202072657475726e2061636365737328796f6c2c20465f4f4b29203d3d20303b0d0a7d0d0a23656e6469660d0a0d0a73746174696320626f6f6c207570705f646f7379615f73696c28636f6e737420636861722a20796f6c29207b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a2020"));
        tampon_yaz(upp_metin_hex_coz("20202020202072657475726e2066616c73653b0d0a202020207d0d0a2020202072657475726e2072656d6f766528796f6c29203d3d20303b0d0a7d0d0a0d0a73746174696320696e74205f7570705f61726763203d20303b0d0a73746174696320636861722a2a205f7570705f61726776203d204e554c4c3b0d0a0d0a73746174696320766f6964207570705f617267765f617961726c6128696e7420617267632c20636861722a2a206172677629207b0d0a202020205f7570705f61726763203d20617267633b0d0a202020205f7570705f61726776203d20617267763b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f617267756d616e5f73617969736928766f696429207b0d0a2020202072657475726e20286c6f6e67206c6f6e67295f7570705f617267633b0d0a7d0d0a0d0a73746174696320636861722a207570705f617267756d616e286c6f6e67206c6f6e67206929207b0d0a202020206966202869203c2030207c7c2069203e3d20286c6f6e67206c6f6e67295f7570705f61726763207c7c20215f7570705f6172677629207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475726e205f7570705f617267765b28696e7429695d3b0d0a7d0d0a0d0a236966205550505f57494e0d0a737461746963206c6f6e67206c6f6e67207570705f7a616d616e28766f696429207b0d0a2020202072657475726e20286c6f6e67206c6f6e67294765745469636b436f756e74363428293b0d0a7d0d0a23656c73650d0a737461746963206c6f6e67206c6f6e67207570705f7a616d616e28766f696429207b0d0a202020207374727563742074696d65737065632074733b0d0a2020202069662028636c6f636b5f67657474696d6528434c4f434b5f4d4f4e4f544f4e49432c202674732920213d203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e20286c6f6e67206c6f6e672974732e74765f736563202a20313030304c4c202b20286c6f6e67206c6f6e67292874732e74765f6e736563202f20313030303030304c293b0d0a7d0d0a23656e6469660d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6d6574696e64656e5f73617969796128636f6e737420636861722a207329207b0d0a2020202069662028217329207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e2061746f6c6c2873293b0d0a7d0d0a0d0a73746174696320696e74207570705f646f7379615f79617a28636f6e737420636861722a20796f6c2c20636f6e737420636861722a2069636572696b29207b0d0a2020202046494c452a20663b0d0a2020202069662028"));
        tampon_yaz(upp_metin_hex_coz("21796f6c29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202066203d20666f70656e28796f6c2c2022776222293b0d0a2020202069662028216629207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206966202869636572696b29207b0d0a202020202020202066707574732869636572696b2c2066293b0d0a202020207d0d0a2020202066636c6f73652866293b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320696e74207570705f646f7379615f656b6c6528636f6e737420636861722a20796f6c2c20636f6e737420636861722a2069636572696b29207b0d0a2020202046494c452a20663b0d0a202020206966202821796f6c29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202066203d20666f70656e28796f6c2c2022616222293b0d0a2020202069662028216629207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206966202869636572696b29207b0d0a202020202020202066707574732869636572696b2c2066293b0d0a202020207d0d0a2020202066636c6f73652866293b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320636861722a207570705f6f7274616d5f616c28636f6e737420636861722a20616429207b0d0a20202020636f6e737420636861722a20763b0d0a20202020636861722a20703b0d0a2020202073697a655f74206e3b0d0a202020206966202821616429207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202076203d20676574656e76286164293b0d0a2020202069662028217629207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206e203d207374726c656e2876293b0d0a2020202070203d2028636861722a296d616c6c6f63286e202b203175293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206d656d63707928702c20762c206e202b203175293b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320636861722a207570705f7361796964616e5f6d6574696e286c6f6e67206c6f6e67206e29207b0d0a2020202063686172206275665b33325d3b0d0a20202020636861722a20703b0d0a2020202073697a655f74206c656e3b0d0a20202020736e7072696e7466286275662c2073697a656f6628627566292c2022256c6c64222c206e293b0d0a202020206c656e203d207374726c656e28627566293b0d0a2020202070203d2028636861722a296d616c6c6f63286c656e202b203175293b0d0a202020206966"));
        tampon_yaz(upp_metin_hex_coz("2028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206d656d63707928702c206275662c206c656e202b203175293b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320636861722a207570705f6f6e64616c696b64616e5f6d6574696e28646f75626c65207829207b0d0a2020202063686172206275665b36345d3b0d0a20202020636861722a20703b0d0a2020202073697a655f74206c656e3b0d0a20202020736e7072696e7466286275662c2073697a656f6628627566292c2022252e313767222c2078293b0d0a202020206c656e203d207374726c656e28627566293b0d0a2020202070203d2028636861722a296d616c6c6f63286c656e202b203175293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206d656d63707928702c206275662c206c656e202b203175293b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f6d6574696e64656e5f6f6e64616c696b28636f6e737420636861722a207329207b0d0a2020202069662028217329207b0d0a202020202020202072657475726e20302e303b0d0a202020207d0d0a2020202072657475726e2061746f662873293b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f757466385f61647628636f6e737420636861722a20732c2073697a655f74206e2c2073697a655f74206929207b0d0a20202020756e7369676e6564206368617220633b0d0a202020206966202869203e3d206e29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202063203d2028756e7369676e6564206368617229735b695d3b0d0a202020206966202863203c203078383029207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a202020206966202828632026203078453029203d3d20307843302026262069202b2031203c206e29207b0d0a202020202020202072657475726e20323b0d0a202020207d0d0a202020206966202828632026203078463029203d3d20307845302026262069202b2032203c206e29207b0d0a202020202020202072657475726e20333b0d0a202020207d0d0a202020206966202828632026203078463829203d3d20307846302026262069202b2033203c206e29207b0d0a202020202020202072657475726e20343b0d0a202020207d0d0a2020202072657475726e20313b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f757466385f73617928636f6e737420636861722a20732c2073697a655f74206e29207b0d0a2020202073697a655f742069203d2030"));
        tampon_yaz(upp_metin_hex_coz("3b0d0a202020206c6f6e67206c6f6e67206b203d20303b0d0a202020207768696c65202869203c206e29207b0d0a20202020202020206c6f6e67206c6f6e672061203d205f7570705f757466385f61647628732c206e2c2069293b0d0a20202020202020206966202861203c3d203029207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020202020202069202b3d202873697a655f7429613b0d0a20202020202020206b2b2b3b0d0a202020207d0d0a2020202072657475726e206b3b0d0a7d0d0a0d0a7374617469632073697a655f74205f7570705f757466385f6261797428636f6e737420636861722a20732c2073697a655f74206e2c206c6f6e67206c6f6e67206b616329207b0d0a2020202073697a655f742069203d20303b0d0a202020206c6f6e67206c6f6e67206b203d20303b0d0a20202020696620286b6163203c3d203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020207768696c65202869203c206e202626206b203c206b616329207b0d0a20202020202020206c6f6e67206c6f6e672061203d205f7570705f757466385f61647628732c206e2c2069293b0d0a20202020202020206966202861203c3d203029207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020202020202069202b3d202873697a655f7429613b0d0a20202020202020206b2b2b3b0d0a202020207d0d0a2020202072657475726e20693b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f757466385f63705f617428636f6e737420636861722a20732c2073697a655f74206e2c2073697a655f74206929207b0d0a20202020756e7369676e6564206368617220633b0d0a20202020696620282173207c7c2069203e3d206e29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202063203d2028756e7369676e6564206368617229735b695d3b0d0a202020206966202863203c203078383029207b0d0a202020202020202072657475726e20286c6f6e67206c6f6e6729633b0d0a202020207d0d0a202020206966202828632026203078453029203d3d20307843302026262069202b2031203c206e29207b0d0a202020202020202072657475726e2028286c6f6e67206c6f6e672928632026203078314629203c3c203629207c202828756e7369676e6564206368617229735b69202b20315d20262030783346293b0d0a202020207d0d0a202020206966202828632026203078463029203d3d20307845302026262069202b2032203c206e29207b0d0a202020202020202072657475726e2028286c6f6e67206c6f6e672928632026203078304629203c3c20313229"));
        tampon_yaz(upp_metin_hex_coz("0d0a2020202020202020202020207c2028286c6f6e67206c6f6e67292828756e7369676e6564206368617229735b69202b20315d2026203078334629203c3c2036290d0a2020202020202020202020207c202828756e7369676e6564206368617229735b69202b20325d20262030783346293b0d0a202020207d0d0a202020206966202828632026203078463829203d3d20307846302026262069202b2033203c206e29207b0d0a202020202020202072657475726e2028286c6f6e67206c6f6e672928632026203078303729203c3c203138290d0a2020202020202020202020207c2028286c6f6e67206c6f6e67292828756e7369676e6564206368617229735b69202b20315d2026203078334629203c3c203132290d0a2020202020202020202020207c2028286c6f6e67206c6f6e67292828756e7369676e6564206368617229735b69202b20325d2026203078334629203c3c2036290d0a2020202020202020202020207c202828756e7369676e6564206368617229735b69202b20335d20262030783346293b0d0a202020207d0d0a2020202072657475726e20286c6f6e67206c6f6e6729633b0d0a7d0d0a0d0a2f2a206b6f642f757a756e6c756b2f6b65732068657220c3a761c49f72c4b16461206261c59f74616e20746172617273612062c3bc79c3bc6b206b61796e616b204f286ec2b229206f6c75722e202a2f0d0a7479706564656620737472756374207b0d0a20202020636f6e737420636861722a20733b0d0a2020202073697a655f74206e62797465733b0d0a202020206c6f6e67206c6f6e67206e63703b0d0a2020202073697a655f742a206f66663b0d0a20202020756e7369676e6564206368617220736967303b0d0a20202020756e7369676e6564206368617220736967313b0d0a20202020756e7369676e6564207665723b0d0a7d20557070557466385461626c6f3b0d0a0d0a737461746963205550505f544c5320557070557466385461626c6f205f7538743b0d0a737461746963205550505f544c5320636861722a205f7570705f67656f5f70203d204e554c4c3b0d0a737461746963205550505f544c532073697a655f74205f7570705f67656f5f636170203d20303b0d0a737461746963205550505f544c5320756e7369676e6564205f7570705f67656f5f766572203d20303b0d0a0d0a73746174696320696e74205f7538745f65736c657328636f6e737420636861722a20732c2073697a655f74206e29207b0d0a20202020696620285f7538742e7320213d2073207c7c205f7538742e6e627974657320213d206e207c7c205f7538742e6e6370203c2030207c7c20215f7538742e6f666629207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620287320262620"));
        tampon_yaz(upp_metin_hex_coz("73203d3d205f7570705f67656f5f70202626205f7538742e76657220213d205f7570705f67656f5f76657229207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620286e203d3d203029207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a2020202069662028217329207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206966202828756e7369676e6564206368617229735b305d20213d205f7538742e7369673029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206966202828756e7369676e6564206368617229735b6e202d20315d20213d205f7538742e7369673129207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320696e74205f7538745f68617a697228636f6e737420636861722a207329207b0d0a2020202073697a655f74206e3b0d0a2020202073697a655f74206361703b0d0a2020202073697a655f7420693b0d0a202020206c6f6e67206c6f6e67206b3b0d0a2020202073697a655f742a206f66663b0d0a202020202f2a2041796ec4b12069c59f61726574c3a7693a207374726c656e20796f6b2e20416b73692068616c646520686572206c6578206164c4b16dc4b120353431204b42206b61796e616b7461204f286ec2b2292e202a2f0d0a202020206966202873202626205f7538742e73203d3d2073202626205f7538742e6e6370203e3d2030202626205f7538742e6f666629207b0d0a202020202020202069662028212873203d3d205f7570705f67656f5f70202626205f7538742e76657220213d205f7570705f67656f5f7665722929207b0d0a20202020202020202020202072657475726e20313b0d0a20202020202020207d0d0a202020207d0d0a202020206e203d2073203f207374726c656e287329203a20303b0d0a20202020696620285f7538745f65736c657328732c206e2929207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a2020202066726565285f7538742e6f6666293b0d0a202020205f7538742e6f6666203d204e554c4c3b0d0a202020205f7538742e73203d20733b0d0a202020205f7538742e6e6279746573203d206e3b0d0a202020205f7538742e6e6370203d20303b0d0a202020205f7538742e73696730203d20286e202626207329203f2028756e7369676e6564206368617229735b305d203a20303b0d0a202020205f7538742e73696731203d20286e202626207329203f2028756e7369676e6564206368617229735b6e202d20315d203a20303b0d0a20202020636170203d206e202b2032753b0d0a2020202069662028"));
        tampon_yaz(upp_metin_hex_coz("636170203c20387529207b0d0a2020202020202020636170203d2038753b0d0a202020207d0d0a202020206f6666203d202873697a655f742a296d616c6c6f6328636170202a2073697a656f662873697a655f7429293b0d0a2020202069662028216f666629207b0d0a20202020202020205f7538742e6e6370203d202d313b0d0a20202020202020205f7538742e73203d204e554c4c3b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206f66665b305d203d20303b0d0a2020202069203d20303b0d0a202020206b203d20303b0d0a202020207768696c65202869203c206e29207b0d0a20202020202020206c6f6e67206c6f6e672061203d205f7570705f757466385f6164762873203f2073203a2022222c206e2c2069293b0d0a20202020202020206966202861203c3d203029207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020202020202069202b3d202873697a655f7429613b0d0a20202020202020206b2b2b3b0d0a2020202020202020696620282873697a655f74296b202b203175203e3d2063617029207b0d0a20202020202020202020202073697a655f74206e636170203d20636170202a2032753b0d0a20202020202020202020202073697a655f742a20713b0d0a202020202020202020202020696620286e636170202f20327520213d2063617029207b0d0a202020202020202020202020202020206e636170203d202873697a655f74296b202b2032753b0d0a2020202020202020202020207d0d0a20202020202020202020202071203d202873697a655f742a297265616c6c6f63286f66662c206e636170202a2073697a656f662873697a655f7429293b0d0a20202020202020202020202069662028217129207b0d0a2020202020202020202020202020202066726565286f6666293b0d0a202020202020202020202020202020205f7538742e6f6666203d204e554c4c3b0d0a202020202020202020202020202020205f7538742e6e6370203d202d313b0d0a202020202020202020202020202020205f7538742e73203d204e554c4c3b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a2020202020202020202020206f6666203d20713b0d0a202020202020202020202020636170203d206e6361703b0d0a20202020202020207d0d0a20202020202020206f66665b6b5d203d20693b0d0a202020207d0d0a202020205f7538742e6f6666203d206f66663b0d0a202020205f7538742e6e6370203d206b3b0d0a202020205f7538742e766572203d2028732026262073203d3d205f7570705f67656f5f7029203f205f7570705f67656f5f766572203a20303b0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("2072657475726e20313b0d0a7d0d0a0d0a7374617469632073697a655f74205f7538745f6261797428636f6e737420636861722a20732c206c6f6e67206c6f6e6720637029207b0d0a20202020696620286370203c3d203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028215f7538745f68617a6972287329207c7c20215f7538742e6f666629207b0d0a202020202020202073697a655f74206e203d2073203f207374726c656e287329203a20303b0d0a202020202020202072657475726e205f7570705f757466385f626179742873203f2073203a2022222c206e2c206370293b0d0a202020207d0d0a20202020696620286370203e3d205f7538742e6e637029207b0d0a202020202020202072657475726e205f7538742e6e62797465733b0d0a202020207d0d0a2020202072657475726e205f7538742e6f66665b63705d3b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6d6574696e5f6b6f6428636f6e737420636861722a20732c206c6f6e67206c6f6e67206929207b0d0a2020202073697a655f7420623b0d0a2020202073697a655f74206e3b0d0a202020206966202869203c203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028215f7538745f68617a6972287329207c7c20215f7538742e6f666629207b0d0a20202020202020206e203d2073203f207374726c656e287329203a20303b0d0a202020202020202062203d205f7570705f757466385f626179742873203f2073203a2022222c206e2c2069293b0d0a20202020202020206966202862203e3d206e29207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202072657475726e205f7570705f757466385f63705f617428732c206e2c2062293b0d0a202020207d0d0a202020206966202869203e3d205f7538742e6e637029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202062203d205f7538742e6f66665b695d3b0d0a2020202072657475726e205f7570705f757466385f63705f617428732c205f7538742e6e62797465732c2062293b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6d6574696e5f696c65726928636f6e737420636861722a20732c206c6f6e67206c6f6e67206929207b0d0a202020206966202869203c203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028215f7538745f68617a6972287329207c7c20215f7538742e6f666629207b0d0a202020202020202073697a655f74206e203d2073203f207374726c656e287329203a20303b0d"));
        tampon_yaz(upp_metin_hex_coz("0a202020202020202073697a655f742062203d205f7570705f757466385f626179742873203f2073203a2022222c206e2c2069293b0d0a20202020202020206966202862203e3d206e29207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202072657475726e205f7570705f757466385f61647628732c206e2c2062293b0d0a202020207d0d0a202020206966202869203e3d205f7538742e6e637029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e20286c6f6e67206c6f6e6729285f7538742e6f66665b69202b20315d202d205f7538742e6f66665b695d293b0d0a7d0d0a0d0a73746174696320766f6964207570705f686174615f79617a28636f6e737420636861722a207329207b0d0a2020202066707574732873203f2073203a2022222c20737464657272293b0d0a2020202066666c75736828737464657272293b0d0a7d0d0a0d0a73746174696320766f6964207570705f686174615f73617469725f79617a28636f6e737420636861722a207329207b0d0a2020202066707574732873203f2073203a2022222c20737464657272293b0d0a20202020667075746328275c6e272c20737464657272293b0d0a2020202066666c75736828737464657272293b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f757a756e6c756b28636f6e737420636861722a207329207b0d0a2020202069662028215f7538745f68617a697228732929207b0d0a202020202020202073697a655f74206e203d2073203f207374726c656e287329203a20303b0d0a202020202020202072657475726e205f7570705f757466385f7361792873203f2073203a2022222c206e293b0d0a202020207d0d0a2020202072657475726e205f7538742e6e63703b0d0a7d0d0a0d0a73746174696320636861722a207570705f676972697328766f696429207b0d0a2020202073697a655f7420636170203d203235363b0d0a2020202073697a655f74206e203d20303b0d0a20202020696e742063203d20303b0d0a20202020696e7420736177203d20303b0d0a20202020636861722a2070203d2028636861722a296d616c6c6f6328636170293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020207768696c6520282863203d20666765746328737464696e292920213d20454f4629207b0d0a2020202020202020736177203d20313b0d0a20202020202020206966202863203d3d20275c6e2729207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a20202020202020206966202863203d3d20275c722729"));
        tampon_yaz(upp_metin_hex_coz("207b0d0a202020202020202020202020696e74206e7874203d20666765746328737464696e293b0d0a202020202020202020202020696620286e787420213d20275c6e27202626206e787420213d20454f4629207b0d0a20202020202020202020202020202020756e67657463286e78742c20737464696e293b0d0a2020202020202020202020207d0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a2020202020202020696620286e202b2031203e3d2063617029207b0d0a20202020202020202020202073697a655f74206e63203d20636170202a2032753b0d0a202020202020202020202020636861722a2071203d2028636861722a297265616c6c6f6328702c206e63293b0d0a20202020202020202020202069662028217129207b0d0a20202020202020202020202020202020667265652870293b0d0a2020202020202020202020202020202072657475726e204e554c4c3b0d0a2020202020202020202020207d0d0a20202020202020202020202070203d20713b0d0a202020202020202020202020636170203d206e633b0d0a20202020202020207d0d0a2020202020202020705b6e2b2b5d203d20286368617229633b0d0a202020207d0d0a2020202069662028217361772026262063203d3d20454f4629207b0d0a2020202020202020667265652870293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020705b6e5d203d20275c30273b0d0a2020202072657475726e20703b202f2a207570702e6d6574696e5f626f73616c74202f2062656c6c656b5f626f73616c7420696c652062c4b172616bc4b16e202a2f0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6d696e286c6f6e67206c6f6e6720612c206c6f6e67206c6f6e67206229207b0d0a2020202072657475726e2061203c2062203f2061203a20623b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6d6178286c6f6e67206c6f6e6720612c206c6f6e67206c6f6e67206229207b0d0a2020202072657475726e2061203e2062203f2061203a20623b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6d75746c616b286c6f6e67206c6f6e67206129207b0d0a20202020756e7369676e6564206c6f6e67206c6f6e6720753b0d0a202020206966202861203e3d203029207b0d0a202020202020202072657475726e20613b0d0a202020207d0d0a202020202f2a204c4c4f4e475f4d494e2069c3a7696e202d61207369676e6564207461c59f6d6173c4b12028554229206f6c6d6173c4b16e206469796520696d7a6173c4b17a20796f6c2e202a2f0d0a2020202075203d2030756c6c202d2028756e7369676e6564206c6f"));
        tampon_yaz(upp_metin_hex_coz("6e67206c6f6e6729613b0d0a202020206966202875203e2028756e7369676e6564206c6f6e67206c6f6e67294c4c4f4e475f4d415829207b0d0a202020202020202072657475726e204c4c4f4e475f4d41583b0d0a202020207d0d0a2020202072657475726e20286c6f6e67206c6f6e6729753b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f6269726c657374697228636f6e737420636861722a20612c20636f6e737420636861722a206229207b0d0a2020202073697a655f74206e61203d2061203f207374726c656e286129203a2030753b0d0a2020202073697a655f74206e62203d2062203f207374726c656e286229203a2030753b0d0a20202020636861722a20703b0d0a20202020696620286e62203e2053495a455f4d4158202d206e61202d20317529207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202070203d2028636861722a296d616c6c6f63286e61202b206e62202b203175293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020696620286e6129207b0d0a20202020202020206d656d63707928702c20612c206e61293b0d0a202020207d0d0a20202020696620286e6229207b0d0a20202020202020206d656d6370792870202b206e612c20622c206e62293b0d0a202020207d0d0a20202020705b6e61202b206e625d203d20275c30273b0d0a2020202072657475726e20703b202f2a207570702e6d6574696e5f626f73616c74202f2062656c6c656b5f626f73616c7420696c652062c4b172616bc4b16e202a2f0d0a7d0d0a0d0a73746174696320696e74207570705f6d6574696e5f6573697428636f6e737420636861722a20612c20636f6e737420636861722a206229207b0d0a202020206966202861203d3d206229207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a20202020696620282161207c7c20216229207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e20737472636d7028612c206229203d3d20303b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f67656f5f626972616b28636f6e737420636861722a207029207b0d0a2020202069662028702026262070203d3d205f7570705f67656f5f7029207b0d0a20202020202020205f7570705f67656f5f70203d204e554c4c3b0d0a20202020202020205f7570705f67656f5f636170203d20303b0d0a20202020202020205f7570705f67656f5f7665722b2b3b0d0a202020207d0d0a7d0d0a0d0a73746174696320766f6964205f7538745f626972616b5f6567657228636f6e737420636861722a20732920"));
        tampon_yaz(upp_metin_hex_coz("7b0d0a202020206966202873202626205f7538742e73203d3d207329207b0d0a202020202020202066726565285f7538742e6f6666293b0d0a20202020202020205f7538742e6f6666203d204e554c4c3b0d0a20202020202020205f7538742e73203d204e554c4c3b0d0a20202020202020205f7538742e6e6279746573203d20303b0d0a20202020202020205f7538742e6e6370203d202d313b0d0a20202020202020205f7538742e73696730203d20303b0d0a20202020202020205f7538742e73696731203d20303b0d0a20202020202020205f7538742e766572203d20303b0d0a202020207d0d0a7d0d0a0d0a73746174696320766f6964207570705f6d6574696e5f626f73616c7428636861722a207329207b0d0a202020205f7570705f67656f5f626972616b2873293b0d0a202020205f7538745f626972616b5f656765722873293b0d0a20202020667265652873293b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f6b6f70796128636f6e737420636861722a207329207b0d0a2020202073697a655f74206e3b0d0a20202020636861722a20703b0d0a2020202069662028217329207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206e203d207374726c656e2873293b0d0a2020202070203d2028636861722a296d616c6c6f63286e202b203175293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206d656d63707928702c20732c206e202b203175293b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320766f6964207570705f6d6574696e5f656b6c6528636861722a2a20732c20636f6e737420636861722a207829207b0d0a20202020636861722a20613b0d0a20202020636f6e737420636861722a20623b0d0a20202020636861722a206b6f7079613b0d0a20202020636861722a20703b0d0a2020202073697a655f74206e613b0d0a2020202073697a655f74206e623b0d0a2020202073697a655f74206e6565643b0d0a2020202073697a655f74206361703b0d0a2020202069662028217329207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202061203d202a733b0d0a2020202062203d2078203f2078203a2022223b0d0a202020206b6f707961203d204e554c4c3b0d0a20202020696620286120262620622026262061203d3d206229207b0d0a20202020202020206b6f707961203d207570705f6d6574696e5f6b6f7079612862293b0d0a202020202020202062203d206b6f707961203f206b6f707961203a2022223b0d0a202020207d0d0a202020206e61203d2061203f207374726c656e286129203a2030"));
        tampon_yaz(upp_metin_hex_coz("753b0d0a202020206e62203d207374726c656e2862293b0d0a20202020696620286e62203e2053495a455f4d4158202d206e61202d20317529207b0d0a2020202020202020696620286b6f70796129207b0d0a20202020202020202020202066726565286b6f707961293b0d0a20202020202020207d0d0a20202020202020207570705f70616e696b28226d6574696e20656b6c656d65207461c59f6d6173c4b122293b0d0a202020207d0d0a202020206e656564203d206e61202b206e62202b2031753b0d0a2020202069662028612026262061203d3d205f7570705f67656f5f70202626205f7570705f67656f5f636170203e3d206e65656429207b0d0a20202020202020206d656d6370792861202b206e612c20622c206e62202b203175293b0d0a20202020202020205f7570705f67656f5f7665722b2b3b0d0a2020202020202020696620286b6f70796129207b0d0a20202020202020202020202066726565286b6f707961293b0d0a20202020202020207d0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202069662028612026262061203d3d205f7570705f67656f5f70202626205f7570705f67656f5f636170203e203029207b0d0a2020202020202020636170203d205f7570705f67656f5f6361703b0d0a20202020202020207768696c652028636170203c206e65656429207b0d0a20202020202020202020202069662028636170203e2053495a455f4d4158202f20327529207b0d0a20202020202020202020202020202020636170203d206e6565643b0d0a20202020202020202020202020202020627265616b3b0d0a2020202020202020202020207d0d0a202020202020202020202020636170202a3d2032753b0d0a20202020202020207d0d0a202020202020202070203d2028636861722a297265616c6c6f6328612c20636170293b0d0a202020202020202069662028217029207b0d0a202020202020202020202020696620286b6f70796129207b0d0a2020202020202020202020202020202066726565286b6f707961293b0d0a2020202020202020202020207d0d0a2020202020202020202020207570705f70616e696b28226d6574696e2062656c6c656b20796574657273697a22293b0d0a20202020202020207d0d0a20202020202020205f7538745f626972616b5f656765722861293b0d0a20202020202020206d656d6370792870202b206e612c20622c206e62202b203175293b0d0a20202020202020202a73203d20703b0d0a20202020202020205f7570705f67656f5f70203d20703b0d0a20202020202020205f7570705f67656f5f636170203d206361703b0d0a20202020202020205f7570705f67656f5f7665722b2b3b0d0a2020202020202020696620286b6f7079612920"));
        tampon_yaz(upp_metin_hex_coz("7b0d0a20202020202020202020202066726565286b6f707961293b0d0a20202020202020207d0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020636170203d206e656564203c20333275203f20363475203a206e656564202a2032753b0d0a2020202069662028636170203c206e65656429207b0d0a2020202020202020636170203d206e6565643b0d0a202020207d0d0a2020202070203d2028636861722a296d616c6c6f6328636170293b0d0a2020202069662028217029207b0d0a2020202020202020696620286b6f70796129207b0d0a20202020202020202020202066726565286b6f707961293b0d0a20202020202020207d0d0a20202020202020207570705f70616e696b28226d6574696e2062656c6c656b20796574657273697a22293b0d0a202020207d0d0a20202020696620286e61202626206129207b0d0a20202020202020206d656d63707928702c20612c206e61293b0d0a202020207d0d0a202020206d656d6370792870202b206e612c20622c206e62202b203175293b0d0a202020205f7570705f67656f5f626972616b2861293b0d0a202020205f7538745f626972616b5f656765722861293b0d0a20202020667265652861293b0d0a202020202a73203d20703b0d0a202020205f7570705f67656f5f70203d20703b0d0a202020205f7570705f67656f5f636170203d206361703b0d0a202020205f7570705f67656f5f7665722b2b3b0d0a20202020696620286b6f70796129207b0d0a202020202020202066726565286b6f707961293b0d0a202020207d0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f6b657328636f6e737420636861722a20732c206c6f6e67206c6f6e67206261732c206c6f6e67206c6f6e6720757a756e6c756b29207b0d0a2020202073697a655f74206e2c2073746172742c20656e642c2074616b653b0d0a20202020636861722a20703b0d0a202020206e203d2073203f207374726c656e287329203a2030753b0d0a2020202069662028626173203c20302920626173203d20303b0d0a2020202069662028757a756e6c756b203c20302920757a756e6c756b203d20303b0d0a202020207374617274203d205f7538745f6261797428732c20626173293b0d0a20202020656e64203d205f7538745f6261797428732c20626173202b20757a756e6c756b293b0d0a2020202069662028656e64203c20737461727429207b0d0a2020202020202020656e64203d2073746172743b0d0a202020207d0d0a2020202074616b65203d20656e64202d2073746172743b0d0a2020202070203d2028636861722a296d616c6c6f632874616b65202b203175293b0d0a20202020696620282170292072657475726e204e554c4c3b0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("206966202874616b6529206d656d63707928702c202873203f2073203a20222229202b2073746172742c2074616b65293b0d0a20202020705b74616b655d203d20275c30273b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320696e74207570705f6d6574696e5f6261736c617228636f6e737420636861722a20732c20636f6e737420636861722a207072656629207b0d0a2020202073697a655f74206e732c206e703b0d0a20202020696620282173292073203d2022223b0d0a20202020696620282170726566292070726566203d2022223b0d0a202020206e73203d207374726c656e2873293b0d0a202020206e70203d207374726c656e2870726566293b0d0a20202020696620286e70203e206e73292072657475726e20303b0d0a2020202072657475726e206d656d636d7028732c20707265662c206e7029203d3d2030203f2031203a20303b0d0a7d0d0a0d0a73746174696320696e74207570705f6d6574696e5f626974657228636f6e737420636861722a20732c20636f6e737420636861722a2073756629207b0d0a2020202073697a655f74206e732c206e703b0d0a20202020696620282173292073203d2022223b0d0a2020202069662028217375662920737566203d2022223b0d0a202020206e73203d207374726c656e2873293b0d0a202020206e70203d207374726c656e28737566293b0d0a20202020696620286e70203e206e73292072657475726e20303b0d0a2020202072657475726e206d656d636d702873202b20286e73202d206e70292c207375662c206e7029203d3d2030203f2031203a20303b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f6b69727028636f6e737420636861722a207329207b0d0a2020202073697a655f74206e2c20692c206a3b0d0a20202020636861722a20703b0d0a20202020696620282173292073203d2022223b0d0a202020206e203d207374726c656e2873293b0d0a2020202069203d20303b0d0a202020207768696c65202869203c206e2026262028735b695d203d3d20272027207c7c20735b695d203d3d20275c7427207c7c20735b695d203d3d20275c7227207c7c20735b695d203d3d20275c6e272929207b0d0a2020202020202020692b2b3b0d0a202020207d0d0a202020206a203d206e3b0d0a202020207768696c6520286a203e20692026262028735b6a202d20315d203d3d20272027207c7c20735b6a202d20315d203d3d20275c7427207c7c20735b6a202d20315d203d3d20275c7227207c7c20735b6a202d20315d203d3d20275c6e272929207b0d0a20202020202020206a2d2d3b0d0a202020207d0d0a2020202070203d2028636861722a296d616c6c6f6328286a202d206929202b203175293b0d0a"));
        tampon_yaz(upp_metin_hex_coz("20202020696620282170292072657475726e204e554c4c3b0d0a20202020696620286a203e206929206d656d63707928702c2073202b20692c206a202d2069293b0d0a20202020705b6a202d20695d203d20275c30273b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f627579756b28636f6e737420636861722a207329207b0d0a2020202073697a655f74206e2c20693b0d0a20202020636861722a20703b0d0a20202020696620282173292073203d2022223b0d0a202020206e203d207374726c656e2873293b0d0a2020202070203d2028636861722a296d616c6c6f63286e202b203175293b0d0a20202020696620282170292072657475726e204e554c4c3b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020756e7369676e656420636861722063203d2028756e7369676e6564206368617229735b695d3b0d0a20202020202020206966202863203e3d202761272026262063203c3d20277a27292063203d2028756e7369676e65642063686172292863202d203332293b0d0a2020202020202020705b695d203d20286368617229633b0d0a202020207d0d0a20202020705b6e5d203d20275c30273b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f6b7563756b28636f6e737420636861722a207329207b0d0a2020202073697a655f74206e2c20693b0d0a20202020636861722a20703b0d0a20202020696620282173292073203d2022223b0d0a202020206e203d207374726c656e2873293b0d0a2020202070203d2028636861722a296d616c6c6f63286e202b203175293b0d0a20202020696620282170292072657475726e204e554c4c3b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020756e7369676e656420636861722063203d2028756e7369676e6564206368617229735b695d3b0d0a20202020202020206966202863203e3d202741272026262063203c3d20275a27292063203d2028756e7369676e65642063686172292863202b203332293b0d0a2020202020202020705b695d203d20286368617229633b0d0a202020207d0d0a20202020705b6e5d203d20275c30273b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f6865785f636f7a28636f6e737420636861722a2068657829207b0d0a2020202073697a655f74206c656e203d20686578203f207374726c656e2868657829203a20303b0d0a2020202073697a655f7420692c206a203d20303b0d0a20202020636861722a206275663b0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("20696620286c656e203d3d2030292072657475726e207570705f6d6574696e5f6b6f707961282222293b0d0a20202020627566203d2028636861722a296d616c6c6f63286c656e202f2032202b2031293b0d0a202020206966202821627566292072657475726e207570705f6d6574696e5f6b6f707961282222293b0d0a20202020666f72202869203d20303b2069202b2031203c206c656e3b2069202b3d203229207b0d0a202020202020202063686172206331203d206865785b695d2c206332203d206865785b69202b20315d3b0d0a2020202020202020696e74207631203d20286331203e3d20273027202626206331203c3d2027392729203f20286331202d2027302729203a2028286331203e3d20276127202626206331203c3d2027662729203f20286331202d20276127202b20313029203a2028286331203e3d20274127202626206331203c3d2027462729203f20286331202d20274127202b20313029203a203029293b0d0a2020202020202020696e74207632203d20286332203e3d20273027202626206332203c3d2027392729203f20286332202d2027302729203a2028286332203e3d20276127202626206332203c3d2027662729203f20286332202d20276127202b20313029203a2028286332203e3d20274127202626206332203c3d2027462729203f20286332202d20274127202b20313029203a203029293b0d0a20202020202020206275665b6a2b2b5d203d2028636861722928287631203c3c203429207c207632293b0d0a202020207d0d0a202020206275665b6a5d203d20303b0d0a2020202072657475726e206275663b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6d6574696e5f6963696e646528636f6e737420636861722a20732c20636f6e737420636861722a2061726129207b0d0a20202020636f6e737420636861722a20703b0d0a2020202073697a655f74206e3b0d0a20202020696620282173207c7c2021617261292072657475726e202d313b0d0a2020202070203d2073747273747228732c20617261293b0d0a20202020696620282170292072657475726e202d313b0d0a202020206e203d202873697a655f74292870202d2073293b0d0a2020202072657475726e205f7570705f757466385f73617928732c206e293b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f646567697374697228636f6e737420636861722a20732c20636f6e737420636861722a2065736b692c20636f6e737420636861722a2079656e6929207b0d0a2020202073697a655f74206e732c206e652c206e6e2c20636f756e742c2065787472612c206361703b0d0a20202020636f6e737420636861722a20703b0d0a20202020636861722a206f75743b0d0a"));
        tampon_yaz(upp_metin_hex_coz("20202020636861722a20773b0d0a20202020696620282173292073203d2022223b0d0a20202020696620282165736b69292065736b69203d2022223b0d0a20202020696620282179656e69292079656e69203d2022223b0d0a202020206e73203d207374726c656e2873293b0d0a202020206e65203d207374726c656e2865736b69293b0d0a202020206e6e203d207374726c656e2879656e69293b0d0a20202020696620286e65203d3d203029207b0d0a20202020202020206f7574203d2028636861722a296d616c6c6f63286e73202b203175293b0d0a202020202020202069662028216f7574292072657475726e204e554c4c3b0d0a20202020202020206d656d637079286f75742c20732c206e73202b203175293b0d0a202020202020202072657475726e206f75743b0d0a202020207d0d0a20202020636f756e74203d20303b0d0a2020202070203d20733b0d0a202020207768696c6520282870203d2073747273747228702c2065736b69292920213d204e554c4c29207b0d0a2020202020202020636f756e742b2b3b0d0a202020202020202070202b3d206e653b0d0a202020207d0d0a202020206578747261203d20286e6e203e206e6529203f20286e6e202d206e6529203a2030753b0d0a20202020696620286578747261203e20307520262620636f756e74203e202853495a455f4d4158202d206e73202d20317529202f20657874726129207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020636170203d206e73202b20636f756e74202a206578747261202b2031753b0d0a202020206f7574203d2028636861722a296d616c6c6f6328636170293b0d0a2020202069662028216f7574292072657475726e204e554c4c3b0d0a2020202077203d206f75743b0d0a2020202070203d20733b0d0a202020207768696c6520282a7029207b0d0a2020202020202020696620287374726e636d7028702c2065736b692c206e6529203d3d203029207b0d0a2020202020202020202020206d656d63707928772c2079656e692c206e6e293b0d0a20202020202020202020202077202b3d206e6e3b0d0a20202020202020202020202070202b3d206e653b0d0a20202020202020207d20656c7365207b0d0a2020202020202020202020202a772b2b203d202a702b2b3b0d0a20202020202020207d0d0a202020207d0d0a202020202a77203d20275c30273b0d0a2020202072657475726e206f75743b0d0a7d0d0a0d0a73746174696320766f69642a2062656c6c656b5f61796972286c6f6e67206c6f6e6720626f79757429207b0d0a2020202069662028626f797574203c3d203029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475"));
        tampon_yaz(upp_metin_hex_coz("726e206d616c6c6f63282873697a655f7429626f797574293b0d0a7d0d0a0d0a73746174696320766f69642062656c6c656b5f626f73616c7428766f69642a2070747229207b0d0a202020206672656528707472293b0d0a7d0d0a0d0a2f2a202d2d2d2d207965726c65c59f696b20796170c4b16c6172202d2d2d2d202a2f0d0a0d0a747970656465662073747275637420596f6e656c6d65207b0d0a20202020646f75626c652070697463683b0d0a20202020646f75626c65207961773b0d0a7d20596f6e656c6d653b0d0a0d0a747970656465662073747275637420456b72616e4e6f6b7461207b0d0a20202020646f75626c6520783b0d0a20202020646f75626c6520793b0d0a202020206c6f6e67206c6f6e6720676f72756e75723b0d0a7d20456b72616e4e6f6b74613b0d0a0d0a74797065646566207374727563742056656b746f7233207b0d0a20202020646f75626c6520783b0d0a20202020646f75626c6520793b0d0a20202020646f75626c65207a3b0d0a7d2056656b746f72333b0d0a0d0a74797065646566207374727563742041726b61706c616e4973207b0d0a202020206c6f6e67206c6f6e672068616e646c653b0d0a7d2041726b61706c616e49733b0d0a0d0a23646566696e65205550505f41524b5f4d4158203235360d0a737461746963206c6f6e67206c6f6e67205f7570705f61726b5f685b5550505f41524b5f4d41585d3b0d0a73746174696320696e74205f7570705f61726b5f6e203d20303b0d0a73746174696320696e74205f7570705f61726b5f6174657869745f6f6b203d20303b0d0a236966205550505f57494e0d0a73746174696320435249544943414c5f53454354494f4e205f7570705f61726b5f63733b0d0a73746174696320696e74205f7570705f61726b5f63735f6f6b203d20303b0d0a73746174696320766f6964207570705f61726b5f6b696c69745f68617a697228766f696429207b0d0a2020202069662028215f7570705f61726b5f63735f6f6b29207b0d0a2020202020202020496e697469616c697a65437269746963616c53656374696f6e28265f7570705f61726b5f6373293b0d0a20202020202020205f7570705f61726b5f63735f6f6b203d20313b0d0a202020207d0d0a7d0d0a73746174696320766f6964207570705f61726b5f6b696c69746c6528766f696429207b0d0a202020207570705f61726b5f6b696c69745f68617a697228293b0d0a20202020456e746572437269746963616c53656374696f6e28265f7570705f61726b5f6373293b0d0a7d0d0a73746174696320766f6964207570705f61726b5f626972616b28766f696429207b0d0a202020204c65617665437269746963616c53656374696f6e28265f7570705f61726b5f6373293b0d0a7d0d0a2365"));
        tampon_yaz(upp_metin_hex_coz("6c73650d0a73746174696320707468726561645f6d757465785f74205f7570705f61726b5f6d75203d20505448524541445f4d555445585f494e495449414c495a45523b0d0a73746174696320766f6964207570705f61726b5f6b696c69746c6528766f696429207b20707468726561645f6d757465785f6c6f636b28265f7570705f61726b5f6d75293b207d0d0a73746174696320766f6964207570705f61726b5f626972616b28766f696429207b20707468726561645f6d757465785f756e6c6f636b28265f7570705f61726b5f6d75293b207d0d0a23656e6469660d0a0d0a73746174696320766f6964207570705f61726b61706c616e5f61746578697428766f696429207b0d0a20202020696e7420693b0d0a202020207570705f61726b5f6b696c69746c6528293b0d0a20202020666f72202869203d20303b2069203c205f7570705f61726b5f6e3b20692b2b29207b0d0a202020202020202069662028215f7570705f61726b5f685b695d29207b0d0a202020202020202020202020636f6e74696e75653b0d0a20202020202020207d0d0a236966205550505f57494e0d0a2020202020202020436c6f736548616e646c65282848414e444c452928696e747074725f74295f7570705f61726b5f685b695d293b0d0a23656c73650d0a2020202020202020707468726561645f6465746163682828707468726561645f74292875696e747074725f74295f7570705f61726b5f685b695d293b0d0a23656e6469660d0a20202020202020205f7570705f61726b5f685b695d203d20303b0d0a202020207d0d0a202020205f7570705f61726b5f6e203d20303b0d0a202020207570705f61726b5f626972616b28293b0d0a7d0d0a0d0a73746174696320766f6964207570705f61726b61706c616e5f6b6179646574286c6f6e67206c6f6e67206829207b0d0a2020202069662028216829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020207570705f61726b5f6b696c69746c6528293b0d0a2020202069662028215f7570705f61726b5f6174657869745f6f6b29207b0d0a2020202020202020617465786974287570705f61726b61706c616e5f617465786974293b0d0a20202020202020205f7570705f61726b5f6174657869745f6f6b203d20313b0d0a202020207d0d0a20202020696620285f7570705f61726b5f6e203c205550505f41524b5f4d415829207b0d0a20202020202020205f7570705f61726b5f685b5f7570705f61726b5f6e2b2b5d203d20683b0d0a202020207d0d0a202020207570705f61726b5f626972616b28293b0d0a7d0d0a0d0a73746174696320766f6964207570705f61726b61706c616e5f63696b6172286c6f6e67206c6f6e67206829207b0d0a20202020696e7420693b"));
        tampon_yaz(upp_metin_hex_coz("0d0a2020202069662028216829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020207570705f61726b5f6b696c69746c6528293b0d0a20202020666f72202869203d20303b2069203c205f7570705f61726b5f6e3b20692b2b29207b0d0a2020202020202020696620285f7570705f61726b5f685b695d203d3d206829207b0d0a2020202020202020202020205f7570705f61726b5f685b695d203d205f7570705f61726b5f685b5f7570705f61726b5f6e202d20315d3b0d0a2020202020202020202020205f7570705f61726b5f6e2d2d3b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020207d0d0a202020207570705f61726b5f626972616b28293b0d0a7d0d0a0d0a7479706564656620737472756374204a534f4e4465676572207b0d0a202020206c6f6e67206c6f6e672069643b0d0a7d204a534f4e44656765723b0d0a0d0a747970656465662073747275637420537572656343696b7469207b0d0a20202020636861722a2063696b74693b0d0a202020206c6f6e67206c6f6e67206b6f643b0d0a20202020696e74206261736172696c693b0d0a7d20537572656343696b74693b0d0a0d0a7479706564656620737472756374204b696c6974207b0d0a202020206c6f6e67206c6f6e672069643b0d0a7d204b696c69743b0d0a0d0a7479706564656620737472756374205570704b6f6c4c69737465207b0d0a202020206c6f6e67206c6f6e672069643b0d0a7d205570704b6f6c4c697374653b0d0a0d0a7479706564656620737472756374205570704b6f6c486172697461207b0d0a202020206c6f6e67206c6f6e672069643b0d0a7d205570704b6f6c4861726974613b0d0a0d0a23646566696e65205550505f4b545f5341594920300d0a23646566696e65205550505f4b545f4f4e44414c494b20310d0a23646566696e65205550505f4b545f4d414e54494b20320d0a23646566696e65205550505f4b545f4d4554494e20330d0a23646566696e65205550505f4b545f4241595420340d0a23646566696e65205550505f4b4f4c4c454b5349594f4e5f4d415820313030303030304c4c0d0a0d0a7479706564656620737472756374207b0d0a20202020696e742063616e6c693b0d0a20202020696e74207461673b0d0a202020206c6f6e67206c6f6e67206e3b0d0a202020206c6f6e67206c6f6e67206361703b0d0a202020206c6f6e67206c6f6e672a20693b0d0a20202020646f75626c652a20643b0d0a20202020636861722a2a20733b0d0a7d205f5570704b6f6c4c69737465483b0d0a0d0a7479706564656620737472756374207b0d0a20202020696e742063616e6c693b0d0a20202020696e74206b7461673b0d0a20202020696e7420767461"));
        tampon_yaz(upp_metin_hex_coz("673b0d0a202020206c6f6e67206c6f6e67206e3b0d0a202020206c6f6e67206c6f6e67206361703b0d0a202020206c6f6e67206c6f6e672a206b5f693b0d0a20202020636861722a2a206b5f733b0d0a202020206c6f6e67206c6f6e672a20765f693b0d0a20202020646f75626c652a20765f643b0d0a20202020636861722a2a20765f733b0d0a202020206c6f6e67206c6f6e672a20686173683b0d0a202020206c6f6e67206c6f6e6720686173685f6361703b0d0a202020206c6f6e67206c6f6e6720686173685f746f6d623b0d0a7d205f5570704b6f6c486172697461483b0d0a0d0a236966205550505f57494e0d0a73746174696320766f6964207570705f61726b61706c616e5f62656b6c652841726b61706c616e49732a206a29207b0d0a2020202048414e444c4520683b0d0a2020202069662028216a207c7c20216a2d3e68616e646c65292072657475726e3b0d0a2020202068203d202848414e444c452928696e747074725f74296a2d3e68616e646c653b0d0a202020207570705f61726b61706c616e5f63696b6172286a2d3e68616e646c65293b0d0a2020202057616974466f7253696e676c654f626a65637428682c20494e46494e495445293b0d0a20202020436c6f736548616e646c652868293b0d0a202020206a2d3e68616e646c65203d20303b0d0a7d0d0a23656c73650d0a73746174696320766f6964207570705f61726b61706c616e5f62656b6c652841726b61706c616e49732a206a29207b0d0a20202020707468726561645f742074683b0d0a2020202069662028216a207c7c20216a2d3e68616e646c65292072657475726e3b0d0a202020207468203d2028707468726561645f74292875696e747074725f74296a2d3e68616e646c653b0d0a202020207570705f61726b61706c616e5f63696b6172286a2d3e68616e646c65293b0d0a20202020707468726561645f6a6f696e2874682c204e554c4c293b0d0a202020206a2d3e68616e646c65203d20303b0d0a7d0d0a23656e6469660d0a0d0a73746174696320766f6964207570705f61726b61706c616e5f62656b6c655f64656765722841726b61706c616e4973206a29207b0d0a202020207570705f61726b61706c616e5f62656b6c6528266a293b0d0a7d0d0a0d0a2f2a202d2d2d2d207570702e6d6174656d6174696b202d2d2d2d202a2f0d0a0d0a73746174696320646f75626c65207570705f6d61745f6d75746c616b28646f75626c65207829207b2072657475726e20666162732878293b207d0d0a73746174696320646f75626c65207570705f6d61745f6b6172656b6f6b28646f75626c65207829207b2072657475726e2078203c20302e30203f20302e30203a20737172742878293b207d0d0a73746174696320646f75626c65207570"));
        tampon_yaz(upp_metin_hex_coz("705f6d61745f757328646f75626c6520612c20646f75626c65206229207b2072657475726e20706f7728612c2062293b207d0d0a73746174696320646f75626c65207570705f6d61745f73696e28646f75626c65207829207b2072657475726e2073696e2878293b207d0d0a73746174696320646f75626c65207570705f6d61745f636f7328646f75626c65207829207b2072657475726e20636f732878293b207d0d0a73746174696320646f75626c65207570705f6d61745f74616e28646f75626c65207829207b2072657475726e2074616e2878293b207d0d0a73746174696320646f75626c65207570705f6d61745f6174616e3228646f75626c6520792c20646f75626c65207829207b2072657475726e206174616e3228792c2078293b207d0d0a73746174696320646f75626c65207570705f6d61745f746162616e28646f75626c65207829207b2072657475726e20666c6f6f722878293b207d0d0a73746174696320646f75626c65207570705f6d61745f746176616e28646f75626c65207829207b2072657475726e206365696c2878293b207d0d0a73746174696320646f75626c65207570705f6d61745f79757661726c6128646f75626c65207829207b2072657475726e20726f756e642878293b207d0d0a73746174696320646f75626c65207570705f6d61745f706928766f696429207b2072657475726e20332e31343135393236353335383937393332333834363b207d0d0a0d0a73746174696320646f75626c65207570705f6d61745f73696e69726c6128646f75626c6520782c20646f75626c65206c6f2c20646f75626c6520686929207b0d0a20202020696620286c6f203e20686929207b0d0a2020202020202020646f75626c652074203d206c6f3b0d0a20202020202020206c6f203d2068693b0d0a20202020202020206869203d20743b0d0a202020207d0d0a202020206966202878203c206c6f292072657475726e206c6f3b0d0a202020206966202878203e206869292072657475726e2068693b0d0a2020202072657475726e20783b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f6d61745f6d65736166655f336428646f75626c652078312c20646f75626c652079312c20646f75626c65207a312c20646f75626c652078322c20646f75626c652079322c20646f75626c65207a3229207b0d0a20202020646f75626c65206478203d207832202d2078312c206479203d207932202d2079312c20647a203d207a32202d207a313b0d0a2020202072657475726e2073717274286478202a206478202b206479202a206479202b20647a202a20647a293b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f6d61745f6d65736166655f76332856656b746f723320612c2056656b74"));
        tampon_yaz(upp_metin_hex_coz("6f7233206229207b0d0a2020202072657475726e207570705f6d61745f6d65736166655f336428612e782c20612e792c20612e7a2c20622e782c20622e792c20622e7a293b0d0a7d0d0a0d0a73746174696320596f6e656c6d65207570705f6d61745f6163695f68657361706c6128646f75626c652078312c20646f75626c652079312c20646f75626c65207a312c20646f75626c652078322c20646f75626c652079322c20646f75626c65207a3229207b0d0a20202020596f6e656c6d6520793b0d0a20202020646f75626c65206478203d207832202d2078312c206479203d207932202d2079312c20647a203d207a32202d207a313b0d0a20202020646f75626c6520687970203d2073717274286478202a206478202b206479202a206479293b0d0a20202020792e796177203d206174616e322864792c206478293b0d0a20202020792e7069746368203d206174616e3228647a2c20687970293b0d0a2020202072657475726e20793b0d0a7d0d0a0d0a73746174696320456b72616e4e6f6b7461207570705f6d61745f64756e79615f656b72616e28646f75626c652068782c20646f75626c652068792c20646f75626c6520687a2c20636f6e737420646f75626c652a206d2c206c6f6e67206c6f6e67206e2c20646f75626c652067656e69736c696b2c20646f75626c652079756b73656b6c696b29207b0d0a20202020456b72616e4e6f6b746120653b0d0a20202020646f75626c652063782c2063792c20637a2c2063772c206e64782c206e64793b0d0a20202020652e78203d20302e303b0d0a20202020652e79203d20302e303b0d0a20202020652e676f72756e7572203d20303b0d0a2020202069662028216d207c7c206e203c20313629207b0d0a202020202020202072657475726e20653b0d0a202020207d0d0a202020202f2a2073c3bc74756e2d6d616a6f722034783420566965772d50726f6a656374696f6e202a2f0d0a202020206378203d206d5b305d202a206878202b206d5b345d202a206879202b206d5b385d202a20687a202b206d5b31325d3b0d0a202020206379203d206d5b315d202a206878202b206d5b355d202a206879202b206d5b395d202a20687a202b206d5b31335d3b0d0a20202020637a203d206d5b325d202a206878202b206d5b365d202a206879202b206d5b31305d202a20687a202b206d5b31345d3b0d0a202020206377203d206d5b335d202a206878202b206d5b375d202a206879202b206d5b31315d202a20687a202b206d5b31355d3b0d0a2020202028766f696429637a3b0d0a20202020696620286377203e2031652d36207c7c206377203c202d31652d3629207b0d0a20202020202020206e6478203d206378202f2063773b0d0a20202020202020206e6479203d206379202f2063"));
        tampon_yaz(upp_metin_hex_coz("773b0d0a2020202020202020652e78203d20286e6478202b20312e3029202a20302e35202a2067656e69736c696b3b0d0a2020202020202020652e79203d2028312e30202d206e647929202a20302e35202a2079756b73656b6c696b3b0d0a2020202020202020652e676f72756e7572203d20286377203e20302e3031202626206e6478203e3d202d312e32202626206e6478203c3d20312e32202626206e6479203e3d202d312e32202626206e6479203c3d20312e3229203f2031203a20303b0d0a202020207d0d0a2020202072657475726e20653b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f6d61745f79756d7573617428646f75626c6520612c20646f75626c6520622c20646f75626c65207429207b0d0a202020206966202874203c20302e30292074203d20302e303b0d0a202020206966202874203e20312e30292074203d20312e303b0d0a2020202072657475726e2061202b202862202d206129202a20743b0d0a7d0d0a0d0a2f2a202d2d2d2d207570702e6769726469202d2d2d2d202a2f0d0a0d0a236966205550505f57494e0d0a73746174696320766f6964207570705f67697264695f666172655f74617369286c6f6e67206c6f6e6720782c206c6f6e67206c6f6e67207929207b0d0a20202020536574437572736f72506f732828696e7429782c2028696e742979293b0d0a7d0d0a0d0a73746174696320766f6964207570705f67697264695f666172655f74696b6c6128766f696429207b0d0a20202020494e50555420696e5b325d3b0d0a202020206d656d73657428696e2c20302c2073697a656f6628696e29293b0d0a20202020696e5b305d2e74797065203d20494e5055545f4d4f5553453b0d0a20202020696e5b305d2e6d692e6477466c616773203d204d4f5553454556454e54465f4c454654444f574e3b0d0a20202020696e5b315d2e74797065203d20494e5055545f4d4f5553453b0d0a20202020696e5b315d2e6d692e6477466c616773203d204d4f5553454556454e54465f4c45465455503b0d0a2020202053656e64496e70757428322c20696e2c2073697a656f6628494e50555429293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f766b28636f6e737420636861722a207329207b0d0a2020202063686172206275665b33325d3b0d0a2020202073697a655f7420692c206e3b0d0a20202020696620282173207c7c2021735b305d292072657475726e20303b0d0a202020206e203d207374726c656e2873293b0d0a20202020696620286e203e3d2073697a656f66286275662929206e203d2073697a656f662862756629202d20313b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020636861722063203d"));
        tampon_yaz(upp_metin_hex_coz("20735b695d3b0d0a20202020202020206966202863203e3d202761272026262063203c3d20277a27292063203d202863686172292863202d203332293b0d0a20202020202020206275665b695d203d20633b0d0a202020207d0d0a202020206275665b6e5d203d20275c30273b0d0a20202020696620286275665b305d203d3d20274627202626206275665b315d203e3d20273127202626206275665b315d203c3d2027392729207b0d0a2020202020202020696e7420666e203d2061746f6928627566202b2031293b0d0a202020202020202069662028666e203e3d203120262620666e203c3d203234292072657475726e20564b5f4631202b2028666e202d2031293b0d0a202020207d0d0a20202020696620286e203d3d203129207b0d0a2020202020202020696620286275665b305d203e3d20274127202626206275665b305d203c3d20275a27292072657475726e206275665b305d3b0d0a2020202020202020696620286275665b305d203e3d20273027202626206275665b305d203c3d20273927292072657475726e206275665b305d3b0d0a202020207d0d0a202020206966202821737472636d70286275662c2022454e5445522229207c7c2021737472636d70286275662c202252455455524e2229292072657475726e20564b5f52455455524e3b0d0a202020206966202821737472636d70286275662c20224553432229207c7c2021737472636d70286275662c20224553434150452229292072657475726e20564b5f4553434150453b0d0a202020206966202821737472636d70286275662c202253504143452229207c7c2021737472636d70286275662c2022424f534c554b2229292072657475726e20564b5f53504143453b0d0a202020206966202821737472636d70286275662c20225441422229292072657475726e20564b5f5441423b0d0a202020206966202821737472636d70286275662c202253484946542229292072657475726e20564b5f53484946543b0d0a202020206966202821737472636d70286275662c20224354524c2229207c7c2021737472636d70286275662c2022434f4e54524f4c2229292072657475726e20564b5f434f4e54524f4c3b0d0a202020206966202821737472636d70286275662c2022414c542229292072657475726e20564b5f4d454e553b0d0a202020206966202821737472636d70286275662c20224241434b53504143452229207c7c2021737472636d70286275662c20224241434b2229292072657475726e20564b5f4241434b3b0d0a202020206966202821737472636d70286275662c202244454c4554452229207c7c2021737472636d70286275662c202244454c2229292072657475726e20564b5f44454c4554453b0d0a202020206966202821737472636d7028"));
        tampon_yaz(upp_metin_hex_coz("6275662c2022494e534552542229207c7c2021737472636d70286275662c2022494e532229292072657475726e20564b5f494e534552543b0d0a202020206966202821737472636d70286275662c2022484f4d452229292072657475726e20564b5f484f4d453b0d0a202020206966202821737472636d70286275662c2022454e442229292072657475726e20564b5f454e443b0d0a202020206966202821737472636d70286275662c20224c4546542229207c7c2021737472636d70286275662c2022534f4c2229292072657475726e20564b5f4c4546543b0d0a202020206966202821737472636d70286275662c202252494748542229207c7c2021737472636d70286275662c20225341472229292072657475726e20564b5f52494748543b0d0a202020206966202821737472636d70286275662c202255502229207c7c2021737472636d70286275662c202259554b4152492229292072657475726e20564b5f55503b0d0a202020206966202821737472636d70286275662c2022444f574e2229207c7c2021737472636d70286275662c202241534147492229292072657475726e20564b5f444f574e3b0d0a202020206966202821737472636d70286275662c20225041474555502229207c7c2021737472636d70286275662c2022504755502229292072657475726e20564b5f5052494f523b0d0a202020206966202821737472636d70286275662c202250414745444f574e2229207c7c2021737472636d70286275662c20225047444e2229292072657475726e20564b5f4e4558543b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f67697264695f7475735f62617328636f6e737420636861722a2074757329207b0d0a20202020494e50555420696e5b325d3b0d0a20202020574f524420766b203d2028574f5244295f7570705f766b28747573293b0d0a202020206966202821766b29207b0d0a2020202020202020667072696e7466287374646572722c20225b752b2b5d2062696c696e6d6579656e207475733a2025735c6e222c20747573203f20747573203a202222293b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020206d656d73657428696e2c20302c2073697a656f6628696e29293b0d0a20202020696e5b305d2e74797065203d20494e5055545f4b4559424f4152443b0d0a20202020696e5b305d2e6b692e77566b203d20766b3b0d0a20202020696e5b315d2e74797065203d20494e5055545f4b4559424f4152443b0d0a20202020696e5b315d2e6b692e77566b203d20766b3b0d0a20202020696e5b315d2e6b692e6477466c616773203d204b45594556454e54465f4b455955503b0d0a2020202053656e64496e70757428322c2069"));
        tampon_yaz(upp_metin_hex_coz("6e2c2073697a656f6628494e50555429293b0d0a7d0d0a23656c73650d0a73746174696320766f6964207570705f67697264695f666172655f74617369286c6f6e67206c6f6e6720782c206c6f6e67206c6f6e67207929207b0d0a2020202028766f696429783b2028766f696429793b0d0a7d0d0a0d0a73746174696320766f6964207570705f67697264695f666172655f74696b6c6128766f696429207b0d0a7d0d0a0d0a73746174696320696e74205f7570705f766b28636f6e737420636861722a207329207b0d0a2020202063686172206275665b33325d3b0d0a2020202073697a655f7420692c206e3b0d0a20202020696620282173207c7c2021735b305d292072657475726e20303b0d0a202020206e203d207374726c656e2873293b0d0a20202020696620286e203e3d2073697a656f66286275662929206e203d2073697a656f662862756629202d20313b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020636861722063203d20735b695d3b0d0a20202020202020206966202863203e3d202761272026262063203c3d20277a27292063203d202863686172292863202d203332293b0d0a20202020202020206275665b695d203d20633b0d0a202020207d0d0a202020206275665b6e5d203d20275c30273b0d0a20202020696620286275665b305d203d3d20274627202626206275665b315d203e3d20273127202626206275665b315d203c3d2027392729207b0d0a2020202020202020696e7420666e203d2061746f6928627566202b2031293b0d0a202020202020202069662028666e203e3d203120262620666e203c3d203234292072657475726e20313b0d0a202020207d0d0a20202020696620286e203d3d203129207b0d0a2020202020202020696620286275665b305d203e3d20274127202626206275665b305d203c3d20275a27292072657475726e20313b0d0a2020202020202020696620286275665b305d203e3d20273027202626206275665b305d203c3d20273927292072657475726e20313b0d0a202020207d0d0a202020206966202821737472636d70286275662c2022454e5445522229207c7c2021737472636d70286275662c202252455455524e2229292072657475726e20313b0d0a202020206966202821737472636d70286275662c20224553432229207c7c2021737472636d70286275662c20224553434150452229292072657475726e20313b0d0a202020206966202821737472636d70286275662c202253504143452229207c7c2021737472636d70286275662c2022424f534c554b2229292072657475726e20313b0d0a202020206966202821737472636d70286275662c20225441422229292072657475726e20313b0d0a20202020"));
        tampon_yaz(upp_metin_hex_coz("6966202821737472636d70286275662c202253484946542229292072657475726e20313b0d0a202020206966202821737472636d70286275662c20224354524c2229207c7c2021737472636d70286275662c2022434f4e54524f4c2229292072657475726e20313b0d0a202020206966202821737472636d70286275662c2022414c542229292072657475726e20313b0d0a202020206966202821737472636d70286275662c20224241434b53504143452229207c7c2021737472636d70286275662c20224241434b2229292072657475726e20313b0d0a202020206966202821737472636d70286275662c202244454c4554452229207c7c2021737472636d70286275662c202244454c2229292072657475726e20313b0d0a202020206966202821737472636d70286275662c2022494e534552542229207c7c2021737472636d70286275662c2022494e532229292072657475726e20313b0d0a202020206966202821737472636d70286275662c2022484f4d452229292072657475726e20313b0d0a202020206966202821737472636d70286275662c2022454e442229292072657475726e20313b0d0a202020206966202821737472636d70286275662c20224c4546542229207c7c2021737472636d70286275662c2022534f4c2229292072657475726e20313b0d0a202020206966202821737472636d70286275662c202252494748542229207c7c2021737472636d70286275662c20225341472229292072657475726e20313b0d0a202020206966202821737472636d70286275662c202255502229207c7c2021737472636d70286275662c202259554b4152492229292072657475726e20313b0d0a202020206966202821737472636d70286275662c2022444f574e2229207c7c2021737472636d70286275662c202241534147492229292072657475726e20313b0d0a202020206966202821737472636d70286275662c20225041474555502229207c7c2021737472636d70286275662c2022504755502229292072657475726e20313b0d0a202020206966202821737472636d70286275662c202250414745444f574e2229207c7c2021737472636d70286275662c20225047444e2229292072657475726e20313b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f67697264695f7475735f62617328636f6e737420636861722a2074757329207b0d0a2020202069662028215f7570705f766b287475732929207b0d0a2020202020202020667072696e7466287374646572722c20225b752b2b5d2062696c696e6d6579656e207475733a2025735c6e222c20747573203f20747573203a202222293b0d0a202020207d0d0a7d0d0a23656e6469660d0a0d0a2f2a202d2d2d2d20757070"));
        tampon_yaz(upp_metin_hex_coz("2e62656c6c656b202d2d2d2d202a2f0d0a0d0a236966205550505f57494e0d0a7374617469632048414e444c45205f7570705f62656c6c656b5f68203d204e554c4c3b0d0a7374617469632044574f5244205f7570705f62656c6c656b5f706964203d20303b0d0a737461746963206c6f6e67206c6f6e67205f7570705f62656c6c656b5f68617461203d20303b0d0a73746174696320435249544943414c5f53454354494f4e205f7570705f62656c6c656b5f63733b0d0a73746174696320696e74205f7570705f62656c6c656b5f63735f6f6b203d20303b0d0a0d0a73746174696320766f6964205f7570705f62656c6c656b5f63735f68617a697228766f696429207b0d0a2020202069662028215f7570705f62656c6c656b5f63735f6f6b29207b0d0a2020202020202020496e697469616c697a65437269746963616c53656374696f6e28265f7570705f62656c6c656b5f6373293b0d0a20202020202020205f7570705f62656c6c656b5f63735f6f6b203d20313b0d0a202020207d0d0a7d0d0a0d0a73746174696320766f6964205f7570705f62656c6c656b5f6b696c69746c6528766f696429207b0d0a202020205f7570705f62656c6c656b5f63735f68617a697228293b0d0a20202020456e746572437269746963616c53656374696f6e28265f7570705f62656c6c656b5f6373293b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f62656c6c656b5f626972616b28766f696429207b0d0a202020204c65617665437269746963616c53656374696f6e28265f7570705f62656c6c656b5f6373293b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f62656c6c656b5f686174615f617961726c6128696e74206f6b29207b0d0a20202020696620286f6b29207b0d0a20202020202020205f7570705f62656c6c656b5f68617461203d20303b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020205f7570705f62656c6c656b5f68617461203d20286c6f6e67206c6f6e67294765744c6173744572726f7228293b0d0a20202020696620285f7570705f62656c6c656b5f68617461203d3d203029207b0d0a20202020202020205f7570705f62656c6c656b5f68617461203d20313b0d0a202020207d0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f736f6e5f6861746128766f696429207b0d0a202020206c6f6e67206c6f6e6720763b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202076203d205f7570705f62656c6c656b5f686174613b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e"));
        tampon_yaz(upp_metin_hex_coz("67207570705f62656c6c656b5f6f6b756e647528766f696429207b0d0a202020206c6f6e67206c6f6e6720763b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202076203d205f7570705f62656c6c656b5f68617461203d3d2030203f2031203a20303b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a73746174696320696e74205f7570705f61645f6573697428636f6e737420636861722a20612c20636f6e737420636861722a206229207b0d0a20202020696620282161207c7c202162292072657475726e20303b0d0a202020207768696c6520282a61202626202a6229207b0d0a202020202020202063686172206361203d202a612c206362203d202a623b0d0a2020202020202020696620286361203e3d20274127202626206361203c3d20275a2729206361203d20286368617229286361202d20274127202b20276127293b0d0a2020202020202020696620286362203e3d20274127202626206362203c3d20275a2729206362203d20286368617229286362202d20274127202b20276127293b0d0a202020202020202069662028636120213d206362292072657475726e20303b0d0a2020202020202020612b2b3b20622b2b3b0d0a202020207d0d0a2020202072657475726e202a61203d3d202a623b0d0a7d0d0a0d0a73746174696320696e74205f7570705f61645f65786528636f6e737420636861722a2077616e742c20636f6e737420636861722a206861766529207b0d0a2020202063686172206275665b4d41585f504154485d3b0d0a20202020696620285f7570705f61645f657369742877616e742c206861766529292072657475726e20313b0d0a20202020696620282177616e74292072657475726e20303b0d0a20202020736e7072696e7466286275662c2073697a656f6628627566292c202225732e657865222c2077616e74293b0d0a2020202072657475726e205f7570705f61645f65736974286275662c2068617665293b0d0a7d0d0a0d0a7374617469632044574f5244205f7570705f7069645f62756c28636f6e737420636861722a20616429207b0d0a2020202048414e444c4520736e61703b0d0a2020202050524f43455353454e54525933322070653b0d0a2020202044574f524420706964203d20303b0d0a2020202069662028216164292072657475726e20303b0d0a20202020736e6170203d20437265617465546f6f6c68656c703332536e617073686f74285448333243535f534e415050524f434553532c2030293b0d0a2020202069662028736e6170203d3d20494e56414c49445f48414e444c455f56414c5545292072657475726e20303b0d0a2020202070652e647753697a65"));
        tampon_yaz(upp_metin_hex_coz("203d2073697a656f66287065293b0d0a202020206966202850726f636573733332466972737428736e61702c202670652929207b0d0a2020202020202020646f207b0d0a202020202020202020202020696620285f7570705f61645f6578652861642c2070652e737a45786546696c652929207b0d0a20202020202020202020202020202020706964203d2070652e7468333250726f6365737349443b0d0a20202020202020202020202020202020627265616b3b0d0a2020202020202020202020207d0d0a20202020202020207d207768696c65202850726f6365737333324e65787428736e61702c2026706529293b0d0a202020207d0d0a20202020436c6f736548616e646c6528736e6170293b0d0a2020202072657475726e207069643b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6261676c616e28636f6e737420636861722a2069736c656d5f61646929207b0d0a2020202044574f524420706964203d205f7570705f7069645f62756c2869736c656d5f616469293b0d0a202020206c6f6e67206c6f6e67206f6b203d20303b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a20202020696620285f7570705f62656c6c656b5f6829207b0d0a2020202020202020436c6f736548616e646c65285f7570705f62656c6c656b5f68293b0d0a20202020202020205f7570705f62656c6c656b5f68203d204e554c4c3b0d0a20202020202020205f7570705f62656c6c656b5f706964203d20303b0d0a202020207d0d0a202020206966202870696429207b0d0a20202020202020205f7570705f62656c6c656b5f68203d204f70656e50726f63657373280d0a20202020202020202020202050524f434553535f564d5f52454144207c2050524f434553535f564d5f5752495445207c2050524f434553535f564d5f4f5045524154494f4e207c2050524f434553535f51554552595f494e464f524d4154494f4e2c0d0a20202020202020202020202046414c53452c20706964293b0d0a2020202020202020696620285f7570705f62656c6c656b5f6829207b0d0a2020202020202020202020205f7570705f62656c6c656b5f706964203d207069643b0d0a2020202020202020202020206f6b203d20313b0d0a20202020202020207d0d0a202020207d0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e206f6b3b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6d6f64756c5f62756c28636f6e737420636861722a20646c6c5f61646929207b0d0a2020202048414e444c4520736e61703b0d0a202020204d4f44554c45454e5452593332206d653b0d0a202020206c6f6e67"));
        tampon_yaz(upp_metin_hex_coz("206c6f6e672062617365203d20303b0d0a2020202044574f5244207069643b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a20202020706964203d205f7570705f62656c6c656b5f7069643b0d0a202020206966202821706964207c7c2021646c6c5f61646929207b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020736e6170203d20437265617465546f6f6c68656c703332536e617073686f74285448333243535f534e41504d4f44554c45207c205448333243535f534e41504d4f44554c4533322c20706964293b0d0a2020202069662028736e6170203d3d20494e56414c49445f48414e444c455f56414c554529207b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206d652e647753697a65203d2073697a656f66286d65293b0d0a20202020696620284d6f64756c653332466972737428736e61702c20266d652929207b0d0a2020202020202020646f207b0d0a202020202020202020202020696620285f7570705f61645f65786528646c6c5f6164692c206d652e737a4d6f64756c652929207b0d0a2020202020202020202020202020202062617365203d20286c6f6e67206c6f6e67292875696e747074725f74296d652e6d6f6442617365416464723b0d0a20202020202020202020202020202020627265616b3b0d0a2020202020202020202020207d0d0a20202020202020207d207768696c6520284d6f64756c6533324e65787428736e61702c20266d6529293b0d0a202020207d0d0a20202020436c6f736548616e646c6528736e6170293b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20626173653b0d0a7d0d0a0d0a73746174696320696e74205f7570705f72706d2875696e747074725f7420616464722c20766f69642a206275662c2073697a655f74206e29207b0d0a2020202053495a455f5420676f74203d20303b0d0a20202020696e74206f6b3b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202069662028215f7570705f62656c6c656b5f68207c7c202162756629207b0d0a20202020202020205f7570705f62656c6c656b5f686174615f617961726c612830293b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206f6b203d205265616450726f636573734d656d6f7279285f7570705f62656c6c656b5f682c20284c5043564f494429616464722c20627566"));
        tampon_yaz(upp_metin_hex_coz("2c206e2c2026676f742920262620676f74203d3d206e3b0d0a202020205f7570705f62656c6c656b5f686174615f617961726c61286f6b293b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e206f6b3b0d0a7d0d0a0d0a73746174696320696e74205f7570705f77706d2875696e747074725f7420616464722c20636f6e737420766f69642a206275662c2073697a655f74206e29207b0d0a2020202053495a455f5420676f74203d20303b0d0a20202020696e74206f6b3b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202069662028215f7570705f62656c6c656b5f68207c7c202162756629207b0d0a20202020202020205f7570705f62656c6c656b5f686174615f617961726c612830293b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206f6b203d20577269746550726f636573734d656d6f7279285f7570705f62656c6c656b5f682c20284c50564f494429616464722c206275662c206e2c2026676f742920262620676f74203d3d206e3b0d0a202020205f7570705f62656c6c656b5f686174615f617961726c61286f6b293b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e206f6b3b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f736179696f6b75286c6f6e67206c6f6e6720616472657329207b0d0a202020206c6f6e67206c6f6e672076203d20303b0d0a2020202069662028215f7570705f72706d282875696e747074725f742961647265732c2026762c2073697a656f6628762929292072657475726e20303b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f62656c6c656b5f6f6b755f6f6e64616c696b286c6f6e67206c6f6e6720616472657329207b0d0a20202020646f75626c652076203d20302e303b0d0a2020202069662028215f7570705f72706d282875696e747074725f742961647265732c2026762c2073697a656f6628762929292072657475726e20302e303b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a73746174696320636861722a207570705f62656c6c656b5f6f6b755f6d6574696e286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720757a756e6c756b29207b0d0a20202020636861722a20703b0d0a2020202069662028757a756e6c756b203c3d2030292072657475726e204e554c4c3b0d0a2020202069662028757a756e6c756b203e20313034383537362920757a756e6c756b203d20313034383537363b0d0a2020"));
        tampon_yaz(upp_metin_hex_coz("202070203d2028636861722a296d616c6c6f63282873697a655f7429757a756e6c756b202b203175293b0d0a20202020696620282170292072657475726e204e554c4c3b0d0a202020206d656d73657428702c20302c202873697a655f7429757a756e6c756b202b203175293b0d0a2020202069662028215f7570705f72706d282875696e747074725f742961647265732c20702c202873697a655f7429757a756e6c756b2929207b0d0a2020202020202020667265652870293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020705b757a756e6c756b5d203d20275c30273b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f7361796979617a286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720646567657229207b0d0a2020202072657475726e205f7570705f77706d282875696e747074725f742961647265732c202664656765722c2073697a656f662864656765722929203f2031203a20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f79617a5f6f6e64616c696b286c6f6e67206c6f6e672061647265732c20646f75626c6520646567657229207b0d0a2020202072657475726e205f7570705f77706d282875696e747074725f742961647265732c202664656765722c2073697a656f662864656765722929203f2031203a20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f7a696e6369725f6f6b75286c6f6e67206c6f6e6720746162616e2c20636f6e7374206c6f6e67206c6f6e672a206f6666732c206c6f6e67206c6f6e67206e29207b0d0a20202020756e7369676e6564206c6f6e67206c6f6e672061646472203d2028756e7369676e6564206c6f6e67206c6f6e6729746162616e3b0d0a202020206c6f6e67206c6f6e6720693b0d0a2020202069662028216f666673207c7c206e203c3d2030292072657475726e20303b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020756e7369676e6564206c6f6e67206c6f6e6720707472203d20303b0d0a202020202020202069662028215f7570705f72706d282875696e747074725f7429616464722c20267074722c2073697a656f66287074722929292072657475726e20303b0d0a202020202020202061646472203d20707472202b2028756e7369676e6564206c6f6e67206c6f6e67296f6666735b695d3b0d0a202020207d0d0a2020202072657475726e20286c6f6e67206c6f6e6729616464723b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c65"));
        tampon_yaz(upp_metin_hex_coz("6b5f79616d615f796170286c6f6e67206c6f6e672061647265732c20636f6e737420636861722a2068657829207b0d0a20202020756e7369676e65642063686172206275665b3235365d3b0d0a2020202073697a655f74206e62203d20303b0d0a20202020636f6e737420636861722a20703b0d0a202020206966202821686578292072657475726e20303b0d0a2020202070203d206865783b0d0a202020207768696c6520282a70202626206e62203c2073697a656f66286275662929207b0d0a2020202020202020756e7369676e656420696e742062203d20303b0d0a20202020202020207768696c6520282a70203d3d20272027207c7c202a70203d3d20275c7427207c7c202a70203d3d20272c272920702b2b3b0d0a202020202020202069662028212a702920627265616b3b0d0a202020202020202069662028737363616e6628702c2022253278222c2026622920213d2031292072657475726e20303b0d0a20202020202020206275665b6e622b2b5d203d2028756e7369676e6564206368617229623b0d0a202020202020202069662028705b305d20262620705b315d202626202828705b305d203e3d2027302720262620705b305d203c3d2027392729207c7c2028705b305d203e3d2027412720262620705b305d203c3d2027462729207c7c2028705b305d203e3d2027612720262620705b305d203c3d2027662729290d0a2020202020202020202020202626202828705b315d203e3d2027302720262620705b315d203c3d2027392729207c7c2028705b315d203e3d2027412720262620705b315d203c3d2027462729207c7c2028705b315d203e3d2027612720262620705b315d203c3d20276627292929207b0d0a20202020202020202020202070202b3d20323b0d0a20202020202020207d20656c7365207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020207d0d0a2020202069662028216e62292072657475726e20303b0d0a2020202072657475726e205f7570705f77706d282875696e747074725f742961647265732c206275662c206e6229203f2031203a20303b0d0a7d0d0a0d0a7374617469632044574f5244205f7570705f697a696e5f6b6f6428636f6e737420636861722a20697a696e2c206c6f6e67206c6f6e67207361796929207b0d0a2020202069662028697a696e29207b0d0a20202020202020206966202821737472636d7028697a696e2c20226f6b756d612229207c7c2021737472636d7028697a696e2c2022504147455f524541444f4e4c592229292072657475726e20504147455f524541444f4e4c593b0d0a20202020202020206966202821737472636d7028697a696e2c20226f6b7579617a2229207c7c2021737472636d7028697a69"));
        tampon_yaz(upp_metin_hex_coz("6e2c202279617a6d612229207c7c2021737472636d7028697a696e2c2022504147455f5245414457524954452229292072657475726e20504147455f5245414457524954453b0d0a20202020202020206966202821737472636d7028697a696e2c202263616c69737469722229207c7c2021737472636d7028697a696e2c2022504147455f455845435554455f524541442229292072657475726e20504147455f455845435554455f524541443b0d0a20202020202020206966202821737472636d7028697a696e2c20226f6b7579617a63616c69737469722229207c7c2021737472636d7028697a696e2c20227277782229207c7c2021737472636d7028697a696e2c2022504147455f455845435554455f5245414457524954452229290d0a20202020202020202020202072657475726e20504147455f455845435554455f5245414457524954453b0d0a20202020202020206966202821737472636d7028697a696e2c2022796f6b2229207c7c2021737472636d7028697a696e2c2022504147455f4e4f4143434553532229292072657475726e20504147455f4e4f4143434553533b0d0a20202020202020206966202821737472636d7028697a696e2c202263616c697374697279617a2229207c7c2021737472636d7028697a696e2c2022504147455f455845435554455f5245414457524954452229292072657475726e20504147455f455845435554455f5245414457524954453b0d0a20202020202020206966202821737472636d7028697a696e2c2022504147455f455845435554452229292072657475726e20504147455f455845435554453b0d0a202020207d0d0a20202020696620287361796920213d2030292072657475726e202844574f524429736179693b0d0a2020202072657475726e20504147455f455845435554455f5245414457524954453b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6b6f72756d615f6465676973746972286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720626f7975742c20636f6e737420636861722a20697a696e29207b0d0a2020202044574f52442065736b69203d20303b0d0a2020202044574f52442079656e693b0d0a202020206c6f6e67206c6f6e672072203d20303b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a20202020696620285f7570705f62656c6c656b5f6820262620626f797574203e203029207b0d0a202020202020202079656e69203d205f7570705f697a696e5f6b6f6428697a696e2c2030293b0d0a2020202020202020696620285669727475616c50726f746563744578285f7570705f62656c6c656b5f682c20284c50564f4944292875696e747074725f742961"));
        tampon_yaz(upp_metin_hex_coz("647265732c202853495a455f5429626f7975742c2079656e692c202665736b692929207b0d0a20202020202020202020202072203d20286c6f6e67206c6f6e672965736b693b0d0a20202020202020207d0d0a202020207d0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20723b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6b6f72756d615f64656769737469725f73617969286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720626f7975742c206c6f6e67206c6f6e6720697a696e29207b0d0a2020202044574f52442065736b69203d20303b0d0a202020206c6f6e67206c6f6e672072203d20303b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a20202020696620285f7570705f62656c6c656b5f6820262620626f797574203e203029207b0d0a2020202020202020696620285669727475616c50726f746563744578285f7570705f62656c6c656b5f682c20284c50564f4944292875696e747074725f742961647265732c202853495a455f5429626f7975742c202844574f524429697a696e2c202665736b692929207b0d0a20202020202020202020202072203d20286c6f6e67206c6f6e672965736b693b0d0a20202020202020207d0d0a202020207d0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20723b0d0a7d0d0a23656c73650d0a2f2a204c696e75783a2062656c6c656b206d6f64c3bc6cc3bc2079616c6ec4b17a63612047c39c4e43454c2073c3bc726563696e202873656c66292062656c6c65c49f696e646520c3a7616cc4b1c59fc4b1722e0d0a202a204261c59f6b612073c3bc7265c3a76c65726520657269c59f696d2070747261636520676572656b74697269723b206261676c616e206b656e64692073c3bc726563696d697a6c650d0a202a2065c59f6c65c59f69727365206261c59f6172c4b16cc4b1206f6c75722e202a2f0d0a73746174696320696e74205f7570705f62656c6c656b5f73656c66203d20303b0d0a737461746963206c6f6e67206c6f6e67205f7570705f62656c6c656b5f68617461203d20303b0d0a73746174696320707468726561645f6d757465785f74205f7570705f62656c6c656b5f6d78203d20505448524541445f4d555445585f494e495449414c495a45523b0d0a0d0a73746174696320766f6964205f7570705f62656c6c656b5f6b696c69746c6528766f696429207b0d0a20202020707468726561645f6d757465785f6c6f636b28265f7570705f62656c6c656b5f6d78293b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f62656c6c656b5f626972616b28766f69"));
        tampon_yaz(upp_metin_hex_coz("6429207b0d0a20202020707468726561645f6d757465785f756e6c6f636b28265f7570705f62656c6c656b5f6d78293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f61645f6573697428636f6e737420636861722a20612c20636f6e737420636861722a206229207b0d0a20202020696620282161207c7c202162292072657475726e20303b0d0a202020207768696c6520282a61202626202a6229207b0d0a202020202020202063686172206361203d202a612c206362203d202a623b0d0a2020202020202020696620286361203e3d20274127202626206361203c3d20275a2729206361203d20286368617229286361202d20274127202b20276127293b0d0a2020202020202020696620286362203e3d20274127202626206362203c3d20275a2729206362203d20286368617229286362202d20274127202b20276127293b0d0a202020202020202069662028636120213d206362292072657475726e20303b0d0a2020202020202020612b2b3b20622b2b3b0d0a202020207d0d0a2020202072657475726e202a61203d3d202a623b0d0a7d0d0a0d0a73746174696320636f6e737420636861722a205f7570705f796f6c5f616428636f6e737420636861722a207029207b0d0a20202020636f6e737420636861722a206c617374203d20703b0d0a2020202069662028217029207b0d0a202020202020202072657475726e2022223b0d0a202020207d0d0a202020207768696c6520282a7029207b0d0a2020202020202020696620282a70203d3d20272f27207c7c202a70203d3d20275c5c2729207b0d0a2020202020202020202020206c617374203d2070202b20313b0d0a20202020202020207d0d0a2020202020202020702b2b3b0d0a202020207d0d0a2020202072657475726e206c6173743b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f61645f6b6972705f65786528636861722a207329207b0d0a2020202073697a655f74206e3b0d0a2020202069662028217329207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020206e203d207374726c656e2873293b0d0a20202020696620286e203e203429207b0d0a2020202020202020636861722a2065203d2073202b206e202d20343b0d0a202020202020202069662028655b305d203d3d20272e272026262028655b315d203d3d20276527207c7c20655b315d203d3d20274527292026262028655b325d203d3d20277827207c7c20655b325d203d3d20275827292026262028655b335d203d3d20276527207c7c20655b335d203d3d202745272929207b0d0a2020202020202020202020202a65203d20275c30273b0d0a20202020202020207d0d0a202020207d0d0a7d0d0a0d0a73746174696320696e74205f7570705f61"));
        tampon_yaz(upp_metin_hex_coz("645f65786528636f6e737420636861722a2077616e742c20636f6e737420636861722a206861766529207b0d0a202020206368617220775b3235365d2c20685b3235365d3b0d0a20202020636f6e73742063686172202a77622c202a68623b0d0a2020202073697a655f7420693b0d0a20202020696620282177616e74207c7c20216861766529207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020207762203d205f7570705f796f6c5f61642877616e74293b0d0a202020206862203d205f7570705f796f6c5f61642868617665293b0d0a20202020666f72202869203d20303b2077625b695d2026262069202b2031203c2073697a656f662877293b20692b2b29207b0d0a2020202020202020775b695d203d2077625b695d3b0d0a202020207d0d0a20202020775b695d203d20275c30273b0d0a20202020666f72202869203d20303b2068625b695d2026262068625b695d20213d20275c72272026262068625b695d20213d20275c6e272026262069202b2031203c2073697a656f662868293b20692b2b29207b0d0a2020202020202020685b695d203d2068625b695d3b0d0a202020207d0d0a20202020685b695d203d20275c30273b0d0a20202020696620285f7570705f61645f6573697428772c20682929207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a202020205f7570705f61645f6b6972705f6578652877293b0d0a202020205f7570705f61645f6b6972705f6578652868293b0d0a2020202072657475726e20775b305d20262620685b305d202626205f7570705f61645f6573697428772c2068293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6b656e64695f616428636861722a206f75742c2073697a655f742063617029207b0d0a2020202073697a655f7420693b0d0a2020202069662028216f7574207c7c20636170203d3d203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a23696620646566696e6564285f5f6c696e75785f5f290d0a202020207b0d0a20202020202020206368617220706174685b343039365d3b0d0a20202020202020207373697a655f74206e3b0d0a2020202020202020636f6e737420636861722a2061643b0d0a202020202020202046494c452a20663b0d0a2020202020202020696e7420633b0d0a20202020202020206e203d20726561646c696e6b28222f70726f632f73656c662f657865222c20706174682c2073697a656f66287061746829202d2031293b0d0a2020202020202020696620286e203e203029207b0d0a202020202020202020202020706174685b6e5d203d20275c30273b0d0a2020202020202020202020206164203d205f7570705f796f6c5f616428706174"));
        tampon_yaz(upp_metin_hex_coz("68293b0d0a20202020202020202020202069203d20303b0d0a2020202020202020202020207768696c65202861645b695d2026262069202b2031203c2063617029207b0d0a202020202020202020202020202020206f75745b695d203d2061645b695d3b0d0a20202020202020202020202020202020692b2b3b0d0a2020202020202020202020207d0d0a2020202020202020202020206f75745b695d203d20275c30273b0d0a2020202020202020202020206966202869203e203029207b0d0a2020202020202020202020202020202072657475726e20313b0d0a2020202020202020202020207d0d0a20202020202020207d0d0a202020202020202066203d20666f70656e28222f70726f632f73656c662f636f6d6d222c20227222293b0d0a202020202020202069662028216629207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202069203d20303b0d0a20202020202020207768696c6520282863203d2066676574632866292920213d20454f462026262069202b2031203c2063617029207b0d0a2020202020202020202020206966202863203d3d20275c6e27207c7c2063203d3d20275c722729207b0d0a20202020202020202020202020202020627265616b3b0d0a2020202020202020202020207d0d0a2020202020202020202020206f75745b692b2b5d203d20286368617229633b0d0a20202020202020207d0d0a20202020202020206f75745b695d203d20275c30273b0d0a202020202020202066636c6f73652866293b0d0a202020202020202072657475726e2069203e20303b0d0a202020207d0d0a23656c73650d0a2020202028766f696429693b0d0a2020202072657475726e20303b0d0a23656e6469660d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6261676c616e28636f6e737420636861722a2069736c656d5f61646929207b0d0a20202020636861722073656c665b3235365d3b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a202020205f7570705f62656c6c656b5f73656c66203d20303b0d0a20202020696620282169736c656d5f616469207c7c202169736c656d5f6164695b305d29207b0d0a20202020202020205f7570705f62656c6c656b5f68617461203d20313b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620285f7570705f6b656e64695f61642873656c662c2073697a656f662873656c662929202626205f7570705f61645f6578652869736c656d5f6164692c2073656c662929207b0d0a20202020202020205f7570705f62656c6c65"));
        tampon_yaz(upp_metin_hex_coz("6b5f73656c66203d20313b0d0a20202020202020205f7570705f62656c6c656b5f68617461203d20303b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a20202020696620285f7570705f61726763203e2030202626205f7570705f61726776202626205f7570705f617267765b305d202626205f7570705f61645f6578652869736c656d5f6164692c205f7570705f617267765b305d2929207b0d0a20202020202020205f7570705f62656c6c656b5f73656c66203d20313b0d0a20202020202020205f7570705f62656c6c656b5f68617461203d20303b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a202020205f7570705f62656c6c656b5f68617461203d20313b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6d6f64756c5f62756c28636f6e737420636861722a20646c6c5f61646929207b0d0a2020202046494c452a20663b0d0a20202020636861722073617469725b3531325d3b0d0a202020206c6f6e67206c6f6e672062617365203d20303b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202069662028215f7570705f62656c6c656b5f73656c66207c7c2021646c6c5f61646929207b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202066203d20666f70656e28222f70726f632f73656c662f6d617073222c20227222293b0d0a2020202069662028216629207b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020207768696c65202866676574732873617469722c2073697a656f66287361746972292c20662929207b0d0a2020202020202020636861722a206e6c3b0d0a2020202020202020636861722a20703b0d0a2020202020202020756e7369676e6564206c6f6e67206c6f6e67206261732c20736f6e3b0d0a202020202020202073697a655f74206c6e3b0d0a20202020202020206e6c203d207374726368722873617469722c20275c6e27293b0d0a2020202020202020696620286e6c29207b0d0a2020202020202020202020202a6e6c203d20275c30273b0d0a20202020202020207d0d0a20202020202020206c6e203d207374726c656e287361746972293b0d0a2020202020202020696620286c6e203e2030"));
        tampon_yaz(upp_metin_hex_coz("2026262073617469725b6c6e202d20315d203d3d20275c722729207b0d0a20202020202020202020202073617469725b6c6e202d20315d203d20275c30273b0d0a20202020202020207d0d0a202020202020202069662028737363616e662873617469722c2022256c6c782d256c6c78222c20266261732c2026736f6e2920213d203229207b0d0a202020202020202020202020636f6e74696e75653b0d0a20202020202020207d0d0a202020202020202028766f696429736f6e3b0d0a202020202020202070203d207374726368722873617469722c20272f27293b0d0a202020202020202069662028217029207b0d0a202020202020202020202020636f6e74696e75653b0d0a20202020202020207d0d0a2020202020202020696620285f7570705f61645f65786528646c6c5f6164692c20702929207b0d0a20202020202020202020202062617365203d20286c6f6e67206c6f6e67296261733b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020207d0d0a2020202066636c6f73652866293b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20626173653b0d0a7d0d0a0d0a73746174696320696e74205f7570705f72706d2875696e747074725f7420616464722c20766f69642a206275662c2073697a655f74206e29207b0d0a2020202069662028215f7570705f62656c6c656b5f73656c66207c7c202162756629207b0d0a20202020202020205f7570705f62656c6c656b5f68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206d656d637079286275662c2028766f69642a29616464722c206e293b0d0a202020205f7570705f62656c6c656b5f68617461203d20303b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320696e74205f7570705f77706d2875696e747074725f7420616464722c20636f6e737420766f69642a206275662c2073697a655f74206e29207b0d0a2020202069662028215f7570705f62656c6c656b5f73656c66207c7c202162756629207b0d0a20202020202020205f7570705f62656c6c656b5f68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206d656d6370792828766f69642a29616464722c206275662c206e293b0d0a202020205f7570705f62656c6c656b5f68617461203d20303b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f736179696f6b75286c6f6e67206c6f6e6720616472657329207b0d0a202020206c6f6e67206c6f6e672076203d20303b0d0a202020205f7570705f62656c6c"));
        tampon_yaz(upp_metin_hex_coz("656b5f6b696c69746c6528293b0d0a2020202069662028215f7570705f72706d282875696e747074725f742961647265732c2026762c2073697a656f662876292929207b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f62656c6c656b5f6f6b755f6f6e64616c696b286c6f6e67206c6f6e6720616472657329207b0d0a20202020646f75626c652076203d20302e303b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202069662028215f7570705f72706d282875696e747074725f742961647265732c2026762c2073697a656f662876292929207b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20302e303b0d0a202020207d0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a73746174696320636861722a207570705f62656c6c656b5f6f6b755f6d6574696e286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720757a756e6c756b29207b0d0a20202020636861722a20703b0d0a20202020696e74206f6b3b0d0a2020202069662028757a756e6c756b203c3d203029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202069662028757a756e6c756b203e203130343835373629207b0d0a2020202020202020757a756e6c756b203d20313034383537363b0d0a202020207d0d0a2020202070203d2028636861722a296d616c6c6f63282873697a655f7429757a756e6c756b202b203175293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206d656d73657428702c20302c202873697a655f7429757a756e6c756b202b203175293b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a202020206f6b203d205f7570705f72706d282875696e747074725f742961647265732c20702c202873697a655f7429757a756e6c756b293b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202069662028216f6b29207b0d0a2020202020202020667265652870293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020705b757a756e6c756b5d203d20275c30273b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e672075"));
        tampon_yaz(upp_metin_hex_coz("70705f62656c6c656b5f7361796979617a286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720646567657229207b0d0a202020206c6f6e67206c6f6e6720723b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202072203d205f7570705f77706d282875696e747074725f742961647265732c202664656765722c2073697a656f662864656765722929203f2031203a20303b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20723b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f79617a5f6f6e64616c696b286c6f6e67206c6f6e672061647265732c20646f75626c6520646567657229207b0d0a202020206c6f6e67206c6f6e6720723b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202072203d205f7570705f77706d282875696e747074725f742961647265732c202664656765722c2073697a656f662864656765722929203f2031203a20303b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20723b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f7a696e6369725f6f6b75286c6f6e67206c6f6e6720746162616e2c20636f6e7374206c6f6e67206c6f6e672a206f6666732c206c6f6e67206c6f6e67206e29207b0d0a20202020756e7369676e6564206c6f6e67206c6f6e672061646472203d2028756e7369676e6564206c6f6e67206c6f6e6729746162616e3b0d0a202020206c6f6e67206c6f6e6720693b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202069662028216f666673207c7c206e203c3d203029207b0d0a20202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020756e7369676e6564206c6f6e67206c6f6e6720707472203d20303b0d0a202020202020202069662028215f7570705f72706d282875696e747074725f7429616464722c20267074722c2073697a656f6628707472292929207b0d0a2020202020202020202020205f7570705f62656c6c656b5f626972616b28293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202061646472203d20707472202b2028756e7369676e6564206c6f6e67206c6f6e67296f6666735b695d3b0d0a202020207d0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e"));
        tampon_yaz(upp_metin_hex_coz("20286c6f6e67206c6f6e6729616464723b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f79616d615f796170286c6f6e67206c6f6e672061647265732c20636f6e737420636861722a2068657829207b0d0a20202020756e7369676e65642063686172206275665b3235365d3b0d0a2020202073697a655f74206e62203d20303b0d0a20202020636f6e737420636861722a20703b0d0a202020206c6f6e67206c6f6e6720723b0d0a20202020696620282168657829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202070203d206865783b0d0a202020207768696c6520282a70202626206e62203c2073697a656f66286275662929207b0d0a2020202020202020756e7369676e656420696e742062203d20303b0d0a20202020202020207768696c6520282a70203d3d20272027207c7c202a70203d3d20275c7427207c7c202a70203d3d20272c2729207b0d0a202020202020202020202020702b2b3b0d0a20202020202020207d0d0a202020202020202069662028212a7029207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020202020202069662028737363616e6628702c2022253278222c2026622920213d203129207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020206275665b6e622b2b5d203d2028756e7369676e6564206368617229623b0d0a202020202020202069662028705b305d20262620705b315d202626202828705b305d203e3d2027302720262620705b305d203c3d2027392729207c7c2028705b305d203e3d2027412720262620705b305d203c3d2027462729207c7c2028705b305d203e3d2027612720262620705b305d203c3d2027662729290d0a2020202020202020202020202626202828705b315d203e3d2027302720262620705b315d203c3d2027392729207c7c2028705b315d203e3d2027412720262620705b315d203c3d2027462729207c7c2028705b315d203e3d2027612720262620705b315d203c3d20276627292929207b0d0a20202020202020202020202070202b3d20323b0d0a20202020202020207d20656c7365207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020207d0d0a2020202069662028216e6229207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202072203d205f7570705f77706d282875696e747074725f742961647265732c206275662c206e6229203f2031203a20303b0d0a202020205f7570705f62656c6c656b5f626972616b28"));
        tampon_yaz(upp_metin_hex_coz("293b0d0a2020202072657475726e20723b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6b6f72756d615f6465676973746972286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720626f7975742c20636f6e737420636861722a20697a696e29207b0d0a2020202028766f69642961647265733b0d0a2020202028766f696429626f7975743b0d0a2020202028766f696429697a696e3b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6b6f72756d615f64656769737469725f73617969286c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720626f7975742c206c6f6e67206c6f6e6720697a696e29207b0d0a2020202028766f69642961647265733b0d0a2020202028766f696429626f7975743b0d0a2020202028766f696429697a696e3b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f736f6e5f6861746128766f696429207b0d0a202020206c6f6e67206c6f6e6720763b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202076203d205f7570705f62656c6c656b5f686174613b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f62656c6c656b5f6f6b756e647528766f696429207b0d0a202020206c6f6e67206c6f6e6720763b0d0a202020205f7570705f62656c6c656b5f6b696c69746c6528293b0d0a2020202076203d205f7570705f62656c6c656b5f68617461203d3d2030203f2031203a20303b0d0a202020205f7570705f62656c6c656b5f626972616b28293b0d0a2020202072657475726e20763b0d0a7d0d0a23656e6469660d0a0d0a2f2a202d2d2d2d207570702e63697a696d20286b61746d616e6cc4b1206f7665726c617929202d2d2d2d202a2f0d0a0d0a236966205550505f57494e0d0a7479706564656620737472756374207b0d0a20202020696e74206b696e643b202f2a2030206b7574752c20312079617a69202a2f0d0a20202020696e7420782c20792c20772c20683b0d0a20202020434f4c4f525245462072656e6b3b0d0a20202020636861722079617a695b3139325d3b0d0a7d205f55707043697a696d4b6f6d75743b0d0a0d0a737461746963205f55707043697a696d4b6f6d75742a205f7570705f63697a696d5f6b203d204e554c4c3b0d0a73746174696320696e74205f7570705f63697a696d5f636170203d20303b0d0a73746174696320696e74205f7570705f63697a696d5f6e203d20303b"));
        tampon_yaz(upp_metin_hex_coz("0d0a7374617469632048574e44205f7570705f63697a696d5f68776e64203d204e554c4c3b0d0a7374617469632048574e44205f7570705f63697a696d5f6865646566203d204e554c4c3b0d0a7374617469632048414e444c45205f7570705f63697a696d5f7468203d204e554c4c3b0d0a73746174696320435249544943414c5f53454354494f4e205f7570705f63697a696d5f63733b0d0a73746174696320696e74205f7570705f63697a696d5f63735f6f6b203d20303b0d0a73746174696320636f6e737420434f4c4f52524546205f7570705f63697a696d5f616e6168746172203d20524742283235352c20302c20323535293b0d0a0d0a73746174696320434f4c4f52524546205f7570705f72656e6b28636f6e737420636861722a20616429207b0d0a2020202063686172206275665b33325d3b0d0a2020202073697a655f7420692c206e3b0d0a2020202069662028216164292072657475726e20524742283235352c20302c2030293b0d0a202020206e203d207374726c656e286164293b0d0a20202020696620286e203e3d2073697a656f66286275662929206e203d2073697a656f662862756629202d20313b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020636861722063203d2061645b695d3b0d0a20202020202020206966202863203e3d202761272026262063203c3d20277a27292063203d202863686172292863202d203332293b0d0a20202020202020206275665b695d203d20633b0d0a202020207d0d0a202020206275665b6e5d203d20275c30273b0d0a202020206966202821737472636d70286275662c20224b49524d495a492229207c7c2021737472636d70286275662c20225245442229292072657475726e20524742283235352c2034302c203430293b0d0a202020206966202821737472636d70286275662c2022594553494c2229207c7c2021737472636d70286275662c2022475245454e2229292072657475726e205247422834302c203232302c203430293b0d0a202020206966202821737472636d70286275662c20224d4156492229207c7c2021737472636d70286275662c2022424c55452229292072657475726e205247422834302c203132302c20323535293b0d0a202020206966202821737472636d70286275662c2022534152492229207c7c2021737472636d70286275662c202259454c4c4f572229292072657475726e20524742283235352c203232302c2030293b0d0a202020206966202821737472636d70286275662c2022424559415a2229207c7c2021737472636d70286275662c202257484954452229292072657475726e20524742283235352c203235352c20323535293b0d0a202020206966202821737472636d"));
        tampon_yaz(upp_metin_hex_coz("70286275662c202253495941482229207c7c2021737472636d70286275662c2022424c41434b2229292072657475726e205247422832302c2032302c203230293b0d0a202020206966202821737472636d70286275662c2022545552554e43552229207c7c2021737472636d70286275662c20224f52414e47452229292072657475726e20524742283235352c203134302c2030293b0d0a202020206966202821737472636d70286275662c202250454d42452229207c7c2021737472636d70286275662c202250494e4b2229292072657475726e20524742283235352c203130352c20313830293b0d0a202020206966202821737472636d70286275662c20224752492229207c7c2021737472636d70286275662c2022475241592229207c7c2021737472636d70286275662c2022475245592229292072657475726e20524742283138302c203138302c20313830293b0d0a202020206966202821737472636d70286275662c20224359414e2229207c7c2021737472636d70286275662c202243414d474f424547492229292072657475726e2052474228302c203232302c20323230293b0d0a2020202072657475726e20524742283235352c2034302c203430293b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f63697a696d5f63735f68617a697228766f696429207b0d0a2020202069662028215f7570705f63697a696d5f63735f6f6b29207b0d0a2020202020202020496e697469616c697a65437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a20202020202020205f7570705f63697a696d5f63735f6f6b203d20313b0d0a202020207d0d0a7d0d0a0d0a73746174696320696e74205f7570705f63697a696d5f656b6c6528636f6e7374205f55707043697a696d4b6f6d75742a206b29207b0d0a20202020696620285f7570705f63697a696d5f6e203e3d205f7570705f63697a696d5f63617029207b0d0a2020202020202020696e74206e636170203d205f7570705f63697a696d5f636170203f205f7570705f63697a696d5f636170202a2032203a203132383b0d0a20202020202020205f55707043697a696d4b6f6d75742a2070203d20285f55707043697a696d4b6f6d75742a297265616c6c6f63285f7570705f63697a696d5f6b2c202873697a655f74296e636170202a2073697a656f66282a7029293b0d0a202020202020202069662028217029207b0d0a202020202020202020202020667072696e7466287374646572722c20225b752b2b5d2063697a696d206b75797275677520646f6c75202862656c6c656b20796574657273697a295c6e22293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020205f75"));
        tampon_yaz(upp_metin_hex_coz("70705f63697a696d5f6b203d20703b0d0a20202020202020205f7570705f63697a696d5f636170203d206e6361703b0d0a202020207d0d0a202020205f7570705f63697a696d5f6b5b5f7570705f63697a696d5f6e2b2b5d203d202a6b3b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a7479706564656620737472756374207b2044574f5244207069643b2048574e442068776e643b20696e7420616c616e3b207d205f55707048776e644172613b0d0a0d0a73746174696320424f4f4c2043414c4c4241434b205f7570705f68776e645f63622848574e442068776e642c204c504152414d206c7029207b0d0a202020205f55707048776e644172612a2061203d20285f55707048776e644172612a296c703b0d0a2020202044574f524420706964203d20303b0d0a202020205245435420723b0d0a20202020696e7420616c616e3b0d0a2020202047657457696e646f7754687265616450726f6365737349642868776e642c2026706964293b0d0a202020206966202870696420213d20612d3e706964292072657475726e20545255453b0d0a202020206966202821497357696e646f7756697369626c652868776e6429292072657475726e20545255453b0d0a20202020696620282147657457696e646f77526563742868776e642c20267229292072657475726e20545255453b0d0a20202020616c616e203d2028722e7269676874202d20722e6c65667429202a2028722e626f74746f6d202d20722e746f70293b0d0a2020202069662028616c616e203e20612d3e616c616e29207b0d0a2020202020202020612d3e616c616e203d20616c616e3b0d0a2020202020202020612d3e68776e64203d2068776e643b0d0a202020207d0d0a2020202072657475726e20545255453b0d0a7d0d0a0d0a7374617469632048574e44205f7570705f70656e636572655f62756c2844574f52442070696429207b0d0a202020205f55707048776e6441726120613b0d0a20202020612e706964203d207069643b0d0a20202020612e68776e64203d204e554c4c3b0d0a20202020612e616c616e203d20303b0d0a20202020456e756d57696e646f7773285f7570705f68776e645f63622c20284c504152414d292661293b0d0a2020202072657475726e20612e68776e643b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f63697a696d5f626f79612848574e442068776e6429207b0d0a202020205041494e545354525543542070733b0d0a2020202048444320686463203d20426567696e5061696e742868776e642c20267073293b0d0a20202020524543542072633b0d0a202020204842525553482062727573683b0d0a20202020696e7420693b0d0a20202020476574436c69656e74526563742868776e642c20267263"));
        tampon_yaz(upp_metin_hex_coz("293b0d0a202020206272757368203d20437265617465536f6c69644272757368285f7570705f63697a696d5f616e6168746172293b0d0a2020202046696c6c52656374286864632c202672632c206272757368293b0d0a2020202044656c6574654f626a656374286272757368293b0d0a20202020536574426b4d6f6465286864632c205452414e53504152454e54293b0d0a20202020456e746572437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a20202020666f72202869203d20303b2069203c205f7570705f63697a696d5f6e3b20692b2b29207b0d0a20202020202020205f55707043697a696d4b6f6d75742a206b203d20265f7570705f63697a696d5f6b5b695d3b0d0a2020202020202020696620286b2d3e6b696e64203d3d203029207b0d0a2020202020202020202020204850454e2070656e203d2043726561746550656e2850535f534f4c49442c20322c206b2d3e72656e6b293b0d0a202020202020202020202020484744494f424a206f6c64203d2053656c6563744f626a656374286864632c2070656e293b0d0a202020202020202020202020484744494f424a206f6c6462203d2053656c6563744f626a656374286864632c2047657453746f636b4f626a656374284e554c4c5f425255534829293b0d0a20202020202020202020202052656374616e676c65286864632c206b2d3e782c206b2d3e792c206b2d3e78202b206b2d3e772c206b2d3e79202b206b2d3e68293b0d0a20202020202020202020202053656c6563744f626a656374286864632c206f6c6462293b0d0a20202020202020202020202053656c6563744f626a656374286864632c206f6c64293b0d0a20202020202020202020202044656c6574654f626a6563742870656e293b0d0a20202020202020207d20656c7365207b0d0a20202020202020202020202077636861725f7420776275665b3139325d3b0d0a202020202020202020202020696e74206e77203d204d756c746942797465546f57696465436861722843505f555446382c20302c206b2d3e79617a692c202d312c20776275662c20313932293b0d0a20202020202020202020202053657454657874436f6c6f72286864632c206b2d3e72656e6b293b0d0a202020202020202020202020696620286e77203e20302920546578744f757457286864632c206b2d3e782c206b2d3e792c20776275662c206e77202d2031293b0d0a202020202020202020202020656c736520546578744f757441286864632c206b2d3e782c206b2d3e792c206b2d3e79617a692c2028696e74297374726c656e286b2d3e79617a6929293b0d0a20202020202020207d0d0a202020207d0d0a202020204c65617665437269746963616c53656374696f6e28265f7570"));
        tampon_yaz(upp_metin_hex_coz("705f63697a696d5f6373293b0d0a20202020456e645061696e742868776e642c20267073293b0d0a7d0d0a0d0a737461746963204c524553554c542043414c4c4241434b205f7570705f63697a696d5f776e642848574e442068776e642c2055494e54206d73672c2057504152414d2077702c204c504152414d206c7029207b0d0a20202020696620286d7367203d3d20574d5f5041494e5429207b0d0a20202020202020205f7570705f63697a696d5f626f79612868776e64293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620286d7367203d3d20574d5f44455354524f5929207b0d0a2020202020202020506f7374517569744d6573736167652830293b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e2044656657696e646f7750726f63412868776e642c206d73672c2077702c206c70293b0d0a7d0d0a0d0a7374617469632044574f52442057494e415049205f7570705f63697a696d5f646f6e6775284c50564f494420706172616d29207b0d0a20202020574e44434c415353412077633b0d0a2020202048574e44206865646566203d202848574e4429706172616d3b0d0a202020205245435420723b0d0a202020204d5347206d73673b0d0a202020206d656d736574282677632c20302c2073697a656f6628776329293b0d0a2020202077632e6c70666e576e6450726f63203d205f7570705f63697a696d5f776e643b0d0a2020202077632e68496e7374616e6365203d204765744d6f64756c6548616e646c6541284e554c4c293b0d0a2020202077632e6c70737a436c6173734e616d65203d202255707043697a696d4f7665726c6179223b0d0a2020202077632e68437572736f72203d204c6f6164437572736f72284e554c4c2c204944435f4152524f57293b0d0a202020205265676973746572436c6173734128267763293b0d0a2020202047657457696e646f77526563742868656465662c202672293b0d0a202020205f7570705f63697a696d5f68776e64203d2043726561746557696e646f77457841280d0a202020202020202057535f45585f4c415945524544207c2057535f45585f5452414e53504152454e54207c2057535f45585f544f504d4f5354207c2057535f45585f544f4f4c57494e444f572c0d0a20202020202020202255707043697a696d4f7665726c6179222c2022222c2057535f504f5055502c0d0a2020202020202020722e6c6566742c20722e746f702c20722e7269676874202d20722e6c6566742c20722e626f74746f6d202d20722e746f702c0d0a20202020202020204e554c4c2c204e554c4c2c2077632e68496e7374616e63652c204e554c4c293b0d0a20202020696620285f7570705f6369"));
        tampon_yaz(upp_metin_hex_coz("7a696d5f68776e6429207b0d0a20202020202020205365744c61796572656457696e646f7741747472696275746573285f7570705f63697a696d5f68776e642c205f7570705f63697a696d5f616e61687461722c20302c204c57415f434f4c4f524b4559293b0d0a202020202020202053686f7757696e646f77285f7570705f63697a696d5f68776e642c2053575f53484f574e4f4143544956415445293b0d0a202020202020202053657454696d6572285f7570705f63697a696d5f68776e642c20312c2031362c204e554c4c293b0d0a202020207d0d0a202020207768696c6520284765744d6573736167654128266d73672c204e554c4c2c20302c203029203e203029207b0d0a2020202020202020696620286d73672e6d657373616765203d3d20574d5f54494d4552202626205f7570705f63697a696d5f686564656620262620497357696e646f77285f7570705f63697a696d5f686564656629202626205f7570705f63697a696d5f68776e6429207b0d0a20202020202020202020202052454354206e723b0d0a2020202020202020202020206966202847657457696e646f7752656374285f7570705f63697a696d5f68656465662c20266e722929207b0d0a202020202020202020202020202020204d6f766557696e646f77285f7570705f63697a696d5f68776e642c206e722e6c6566742c206e722e746f702c206e722e7269676874202d206e722e6c6566742c206e722e626f74746f6d202d206e722e746f702c2054525545293b0d0a2020202020202020202020207d0d0a20202020202020207d0d0a20202020202020205472616e736c6174654d65737361676528266d7367293b0d0a202020202020202044697370617463684d6573736167654128266d7367293b0d0a202020207d0d0a202020205f7570705f63697a696d5f68776e64203d204e554c4c3b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f63697a696d5f6261736c617428636f6e737420636861722a20737572656329207b0d0a2020202044574f5244207069643b0d0a2020202048574e442068656465663b0d0a202020205f7570705f63697a696d5f63735f68617a697228293b0d0a20202020706964203d205f7570705f7069645f62756c287375726563293b0d0a202020206966202821706964292072657475726e20303b0d0a202020206865646566203d205f7570705f70656e636572655f62756c28706964293b0d0a2020202069662028216865646566292072657475726e20303b0d0a202020205f7570705f63697a696d5f6865646566203d2068656465663b0d0a202020202f2a206f7665726c61792069c59f20706172c3a76163c4b1c49fc4b120626974746979736520286877"));
        tampon_yaz(upp_metin_hex_coz("6e6420796f6b292079656e6964656e206261c59f6c61743b207961c59fc4b1796f727361206865646566692067c3bc6e63656c6c65202a2f0d0a20202020696620285f7570705f63697a696d5f746820262620215f7570705f63697a696d5f68776e6429207b0d0a2020202020202020436c6f736548616e646c65285f7570705f63697a696d5f7468293b0d0a20202020202020205f7570705f63697a696d5f7468203d204e554c4c3b0d0a202020207d0d0a2020202069662028215f7570705f63697a696d5f746829207b0d0a20202020202020205f7570705f63697a696d5f7468203d20437265617465546872656164284e554c4c2c20302c205f7570705f63697a696d5f646f6e67752c2068656465662c20302c204e554c4c293b0d0a202020202020202069662028215f7570705f63697a696d5f7468292072657475726e20303b0d0a20202020202020202f2a2070656e63657265206f6c75c59f616e61206b61646172206bc4b173612062656b6c65202a2f0d0a20202020202020207b0d0a202020202020202020202020696e7420693b0d0a202020202020202020202020666f72202869203d20303b2069203c20353020262620215f7570705f63697a696d5f68776e643b20692b2b2920536c656570283130293b0d0a20202020202020207d0d0a202020207d0d0a2020202072657475726e205f7570705f63697a696d5f68776e64203f2031203a20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f63697a696d5f74656d697a6c6528766f696429207b0d0a202020205f7570705f63697a696d5f63735f68617a697228293b0d0a20202020456e746572437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a202020205f7570705f63697a696d5f6e203d20303b0d0a202020204c65617665437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a20202020696620285f7570705f63697a696d5f68776e642920496e76616c696461746552656374285f7570705f63697a696d5f68776e642c204e554c4c2c2054525545293b0d0a7d0d0a0d0a73746174696320766f6964207570705f63697a696d5f6b757475286c6f6e67206c6f6e6720782c206c6f6e67206c6f6e6720792c206c6f6e67206c6f6e6720772c206c6f6e67206c6f6e6720682c20636f6e737420636861722a2072656e6b29207b0d0a202020205f55707043697a696d4b6f6d7574206b3b0d0a202020205f7570705f63697a696d5f63735f68617a697228293b0d0a202020206d656d73657428266b2c20302c2073697a656f66286b29293b0d0a202020206b2e6b696e64203d20303b0d0a202020206b2e78203d2028696e7429783b206b2e79203d2028696e7429793b206b2e7720"));
        tampon_yaz(upp_metin_hex_coz("3d2028696e7429773b206b2e68203d2028696e7429683b0d0a202020206b2e72656e6b203d205f7570705f72656e6b2872656e6b293b0d0a20202020456e746572437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a202020205f7570705f63697a696d5f656b6c6528266b293b0d0a202020204c65617665437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a20202020696620285f7570705f63697a696d5f68776e642920496e76616c696461746552656374285f7570705f63697a696d5f68776e642c204e554c4c2c2054525545293b0d0a7d0d0a0d0a73746174696320766f6964207570705f63697a696d5f79617a69286c6f6e67206c6f6e6720782c206c6f6e67206c6f6e6720792c20636f6e737420636861722a206d6574696e2c20636f6e737420636861722a2072656e6b29207b0d0a202020205f55707043697a696d4b6f6d7574206b3b0d0a202020205f7570705f63697a696d5f63735f68617a697228293b0d0a202020206d656d73657428266b2c20302c2073697a656f66286b29293b0d0a202020206b2e6b696e64203d20313b0d0a202020206b2e78203d2028696e7429783b206b2e79203d2028696e7429793b0d0a202020206b2e72656e6b203d205f7570705f72656e6b2872656e6b293b0d0a20202020696620286d6574696e29207b0d0a20202020202020207374726e637079286b2e79617a692c206d6574696e2c2073697a656f66286b2e79617a6929202d2031293b0d0a202020207d0d0a20202020456e746572437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a202020205f7570705f63697a696d5f656b6c6528266b293b0d0a202020204c65617665437269746963616c53656374696f6e28265f7570705f63697a696d5f6373293b0d0a20202020696620285f7570705f63697a696d5f68776e642920496e76616c696461746552656374285f7570705f63697a696d5f68776e642c204e554c4c2c2054525545293b0d0a7d0d0a23656c73650d0a2f2a204c696e75783a2063697a696d20286f7665726c6179292058313120676572656b74697269723b2064657374656b6c656e6d69796f722e202a2f0d0a737461746963206c6f6e67206c6f6e67207570705f63697a696d5f6261736c617428636f6e737420636861722a20737572656329207b0d0a2020202028766f69642973757265633b0d0a2020202072657475726e20303b0d0a7d0d0a73746174696320766f6964207570705f63697a696d5f74656d697a6c6528766f696429207b0d0a7d0d0a73746174696320766f6964207570705f63697a696d5f6b757475286c6f6e67206c6f6e6720782c206c6f6e67206c6f6e6720792c206c6f6e67"));
        tampon_yaz(upp_metin_hex_coz("206c6f6e6720772c206c6f6e67206c6f6e6720682c20636f6e737420636861722a2072656e6b29207b0d0a2020202028766f696429783b2028766f696429793b2028766f696429773b2028766f696429683b2028766f69642972656e6b3b0d0a7d0d0a73746174696320766f6964207570705f63697a696d5f79617a69286c6f6e67206c6f6e6720782c206c6f6e67206c6f6e6720792c20636f6e737420636861722a206d6574696e2c20636f6e737420636861722a2072656e6b29207b0d0a2020202028766f696429783b2028766f696429793b2028766f6964296d6574696e3b2028766f69642972656e6b3b0d0a7d0d0a23656e6469660d0a0d0a2f2a202d2d2d2d206c697374655b545d202f206861726974615b4b2c565d202876322e3729202d2d2d2d202a2f0d0a0d0a737461746963205f5570704b6f6c4c69737465482a205f7570705f6c697374656c6572203d204e554c4c3b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6c697374655f6e203d20313b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6c697374655f636170203d20303b0d0a737461746963205f5570704b6f6c486172697461482a205f7570705f6861726974616c6172203d204e554c4c3b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6861726974615f6e203d20313b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6861726974615f636170203d20303b0d0a0d0a73746174696320636861722a205f7570705f6b6f6c5f6b6f70796128636f6e737420636861722a207329207b0d0a2020202073697a655f74206e3b0d0a20202020636861722a20703b0d0a2020202069662028217329207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206e203d207374726c656e2873293b0d0a2020202070203d2028636861722a296d616c6c6f63286e202b203175293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206d656d63707928702c20732c206e202b203175293b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a737461746963205f5570704b6f6c4c69737465482a205f7570705f6c697374655f6875637265286c6f6e67206c6f6e6720696429207b0d0a20202020696620286964203c3d2030207c7c206964203e3d205f7570705f6c697374655f6e207c7c20215f7570705f6c697374656c657229207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202069662028215f7570705f6c697374656c65725b69645d2e63616e6c6929207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072"));
        tampon_yaz(upp_metin_hex_coz("657475726e20265f7570705f6c697374656c65725b69645d3b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6c697374655f79656e6928696e742074616729207b0d0a202020206c6f6e67206c6f6e6720693b0d0a202020206c6f6e67206c6f6e67206e633b0d0a202020205f5570704b6f6c4c69737465482a206e703b0d0a20202020666f72202869203d20313b2069203c205f7570705f6c697374655f6e3b20692b2b29207b0d0a202020202020202069662028215f7570705f6c697374656c65725b695d2e63616e6c6929207b0d0a2020202020202020202020206d656d73657428265f7570705f6c697374656c65725b695d2c20302c2073697a656f66285f5570704b6f6c4c697374654829293b0d0a2020202020202020202020205f7570705f6c697374656c65725b695d2e63616e6c69203d20313b0d0a2020202020202020202020205f7570705f6c697374656c65725b695d2e746167203d207461673b0d0a20202020202020202020202072657475726e20693b0d0a20202020202020207d0d0a202020207d0d0a20202020696620285f7570705f6c697374655f6e203e3d205550505f4b4f4c4c454b5349594f4e5f4d415829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620285f7570705f6c697374655f6e202b2031203e205f7570705f6c697374655f63617029207b0d0a20202020202020206e63203d205f7570705f6c697374655f636170203f205f7570705f6c697374655f636170202a2032203a2031363b0d0a2020202020202020696620286e63203c205f7570705f6c697374655f6e202b203129207b0d0a2020202020202020202020206e63203d205f7570705f6c697374655f6e202b20313b0d0a20202020202020207d0d0a20202020202020206e70203d20285f5570704b6f6c4c69737465482a297265616c6c6f63285f7570705f6c697374656c65722c202873697a655f74296e63202a2073697a656f66285f5570704b6f6c4c697374654829293b0d0a202020202020202069662028216e7029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020206d656d736574286e70202b205f7570705f6c697374655f6361702c20302c202873697a655f7429286e63202d205f7570705f6c697374655f63617029202a2073697a656f66285f5570704b6f6c4c697374654829293b0d0a20202020202020205f7570705f6c697374656c6572203d206e703b0d0a20202020202020205f7570705f6c697374655f636170203d206e633b0d0a202020207d0d0a2020202069203d205f7570705f6c697374655f6e2b2b3b0d0a202020206d656d73657428265f7570705f6c697374656c"));
        tampon_yaz(upp_metin_hex_coz("65725b695d2c20302c2073697a656f66285f5570704b6f6c4c697374654829293b0d0a202020205f7570705f6c697374656c65725b695d2e63616e6c69203d20313b0d0a202020205f7570705f6c697374656c65725b695d2e746167203d207461673b0d0a2020202072657475726e20693b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6c697374655f68617a6972285570704b6f6c4c697374652a204c2c20696e742074616729207b0d0a2020202069662028214c29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620284c2d3e6964203d3d203029207b0d0a20202020202020204c2d3e6964203d205f7570705f6c697374655f79656e6928746167293b0d0a2020202020202020696620284c2d3e6964203d3d203029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020207d0d0a202020207b0d0a20202020202020205f5570704b6f6c4c69737465482a2068203d205f7570705f6c697374655f6875637265284c2d3e6964293b0d0a2020202020202020696620282168207c7c20682d3e74616720213d2074616729207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202072657475726e20313b0d0a202020207d0d0a7d0d0a0d0a73746174696320696e74205f7570705f6c697374655f6275797574285f5570704b6f6c4c69737465482a206829207b0d0a202020206c6f6e67206c6f6e67206e633b0d0a2020202069662028216829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028682d3e636170203e203020262620682d3e636170203e20284c4c4f4e475f4d4158202f20322929207b0d0a20202020202020207570705f70616e696b28226c69737465206b61706173697465207461c59f6d6173c4b122293b0d0a202020207d0d0a202020206e63203d20682d3e636170203f20682d3e636170202a2032203a20383b0d0a2020202069662028682d3e746167203d3d205550505f4b545f4f4e44414c494b29207b0d0a2020202020202020646f75626c652a2070203d2028646f75626c652a297265616c6c6f6328682d3e642c202873697a655f74296e63202a2073697a656f6628646f75626c6529293b0d0a202020202020202069662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e64203d20703b0d0a202020207d20656c73652069662028682d3e746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020636861722a2a2070203d2028636861722a2a297265616c6c6f6328682d3e732c"));
        tampon_yaz(upp_metin_hex_coz("202873697a655f74296e63202a2073697a656f6628636861722a29293b0d0a202020202020202069662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e73203d20703b0d0a202020207d20656c7365207b0d0a20202020202020206c6f6e67206c6f6e672a2070203d20286c6f6e67206c6f6e672a297265616c6c6f6328682d3e692c202873697a655f74296e63202a2073697a656f66286c6f6e67206c6f6e6729293b0d0a202020202020202069662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e69203d20703b0d0a202020207d0d0a20202020682d3e636170203d206e633b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320766f6964207570705f6c697374655f656b6c65285570704b6f6c4c697374652a204c2c20696e74207461672c206c6f6e67206c6f6e672069762c20646f75626c652064762c20636f6e737420636861722a20737629207b0d0a202020205f5570704b6f6c4c69737465482a20683b0d0a2020202069662028215f7570705f6c697374655f68617a6972284c2c207461672929207b0d0a20202020202020207570705f70616e696b28226c697374652062656c6c656b20796574657273697a22293b0d0a202020207d0d0a2020202068203d205f7570705f6c697374655f6875637265284c2d3e6964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202069662028682d3e6e203e3d20682d3e63617020262620215f7570705f6c697374655f627579757428682929207b0d0a20202020202020207570705f70616e696b28226c697374652062656c6c656b20796574657273697a22293b0d0a202020207d0d0a2020202069662028682d3e6e203e3d20682d3e63617029207b0d0a20202020202020207570705f70616e696b28226c697374652062656c6c656b20796574657273697a22293b0d0a202020207d0d0a2020202069662028746167203d3d205550505f4b545f4f4e44414c494b29207b0d0a2020202020202020682d3e645b682d3e6e2b2b5d203d2064763b0d0a202020207d20656c73652069662028746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020682d3e735b682d3e6e2b2b5d203d205f7570705f6b6f6c5f6b6f707961287376293b0d0a202020207d20656c73652069662028746167203d3d205550505f4b545f4241595429207b0d0a2020202020202020682d3e695b682d3e6e2b2b5d203d20286c6f6e67206c6f6e67292828756e7369676e65642063686172296976293b0d0a20202020"));
        tampon_yaz(upp_metin_hex_coz("7d20656c7365207b0d0a2020202020202020682d3e695b682d3e6e2b2b5d203d2069763b0d0a202020207d0d0a7d0d0a0d0a73746174696320766f6964207570705f6c697374655f79617a285570704b6f6c4c697374652a204c2c20696e74207461672c206c6f6e67206c6f6e6720692c206c6f6e67206c6f6e672069762c20646f75626c652064762c20636f6e737420636861722a20737629207b0d0a202020205f5570704b6f6c4c69737465482a20683b0d0a2020202069662028214c207c7c204c2d3e6964203d3d203029207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202068203d205f7570705f6c697374655f6875637265284c2d3e6964293b0d0a20202020696620282168207c7c20682d3e74616720213d20746167207c7c2069203c2030207c7c2069203e3d20682d3e6e29207b0d0a20202020202020207570705f70616e696b28226c6973746520696e64656b732073c4b16ec4b1722064c4b1c59fc4b122293b0d0a202020207d0d0a2020202069662028746167203d3d205550505f4b545f4f4e44414c494b29207b0d0a2020202020202020682d3e645b695d203d2064763b0d0a202020207d20656c73652069662028746167203d3d205550505f4b545f4d4554494e29207b0d0a202020202020202069662028682d3e735b695d29207b0d0a2020202020202020202020206672656528682d3e735b695d293b0d0a20202020202020207d0d0a2020202020202020682d3e735b695d203d205f7570705f6b6f6c5f6b6f707961287376293b0d0a202020207d20656c73652069662028746167203d3d205550505f4b545f4241595429207b0d0a2020202020202020682d3e695b695d203d20286c6f6e67206c6f6e67292828756e7369676e65642063686172296976293b0d0a202020207d20656c7365207b0d0a2020202020202020682d3e695b695d203d2069763b0d0a202020207d0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6c697374655f616c5f69285570704b6f6c4c69737465204c2c206c6f6e67206c6f6e67206929207b0d0a202020205f5570704b6f6c4c69737465482a2068203d205f7570705f6c697374655f6875637265284c2e6964293b0d0a20202020696620282168207c7c2069203c2030207c7c2069203e3d20682d3e6e29207b0d0a20202020202020207570705f70616e696b28226c6973746520696e64656b732073c4b16ec4b1722064c4b1c59fc4b122293b0d0a202020207d0d0a2020202069662028682d3e746167203d3d205550505f4b545f4f4e44414c494b207c7c20682d3e746167203d3d205550505f4b545f4d4554494e29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028682d3e"));
        tampon_yaz(upp_metin_hex_coz("746167203d3d205550505f4b545f4241595429207b0d0a202020202020202072657475726e20286c6f6e67206c6f6e67292828756e7369676e6564206368617229682d3e695b695d293b0d0a202020207d0d0a2020202072657475726e20682d3e695b695d3b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f6c697374655f616c5f64285570704b6f6c4c69737465204c2c206c6f6e67206c6f6e67206929207b0d0a202020205f5570704b6f6c4c69737465482a2068203d205f7570705f6c697374655f6875637265284c2e6964293b0d0a20202020696620282168207c7c20682d3e74616720213d205550505f4b545f4f4e44414c494b207c7c2069203c2030207c7c2069203e3d20682d3e6e29207b0d0a20202020202020207570705f70616e696b28226c6973746520696e64656b732073c4b16ec4b1722064c4b1c59fc4b122293b0d0a202020207d0d0a2020202072657475726e20682d3e645b695d3b0d0a7d0d0a0d0a73746174696320636861722a207570705f6c697374655f616c5f73285570704b6f6c4c69737465204c2c206c6f6e67206c6f6e67206929207b0d0a202020205f5570704b6f6c4c69737465482a2068203d205f7570705f6c697374655f6875637265284c2e6964293b0d0a20202020696620282168207c7c20682d3e74616720213d205550505f4b545f4d4554494e207c7c2069203c2030207c7c2069203e3d20682d3e6e29207b0d0a20202020202020207570705f70616e696b28226c6973746520696e64656b732073c4b16ec4b1722064c4b1c59fc4b122293b0d0a202020207d0d0a2020202072657475726e20682d3e735b695d203f205f7570705f6b6f6c5f6b6f70796128682d3e735b695d29203a204e554c4c3b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6c697374655f757a756e6c756b285570704b6f6c4c69737465204c29207b0d0a202020205f5570704b6f6c4c69737465482a2068203d205f7570705f6c697374655f6875637265284c2e6964293b0d0a2020202072657475726e2068203f20682d3e6e203a20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f6c697374655f626f73616c74285570704b6f6c4c697374652a204c29207b0d0a202020205f5570704b6f6c4c69737465482a20683b0d0a202020206c6f6e67206c6f6e6720693b0d0a2020202069662028214c207c7c204c2d3e6964203d3d203029207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202068203d205f7570705f6c697374655f6875637265284c2d3e6964293b0d0a2020202069662028216829207b0d0a20202020202020204c2d3e6964203d20303b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020"));
        tampon_yaz(upp_metin_hex_coz("202069662028682d3e746167203d3d205550505f4b545f4d4554494e20262620682d3e7329207b0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020202020206672656528682d3e735b695d293b0d0a20202020202020207d0d0a202020207d0d0a202020206672656528682d3e69293b0d0a202020206672656528682d3e64293b0d0a202020206672656528682d3e73293b0d0a202020206d656d73657428682c20302c2073697a656f66285f5570704b6f6c4c697374654829293b0d0a202020204c2d3e6964203d20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f6c697374655f73696c285570704b6f6c4c697374652a204c2c206c6f6e67206c6f6e672069647829207b0d0a202020205f5570704b6f6c4c69737465482a20683b0d0a202020206c6f6e67206c6f6e67206a3b0d0a2020202069662028214c207c7c204c2d3e6964203d3d2030292072657475726e3b0d0a2020202068203d205f7570705f6c697374655f6875637265284c2d3e6964293b0d0a20202020696620282168207c7c20696478203c2030207c7c20696478203e3d20682d3e6e292072657475726e3b0d0a2020202069662028682d3e746167203d3d205550505f4b545f4d4554494e29207b0d0a202020202020202069662028682d3e735b6964785d29206672656528682d3e735b6964785d293b0d0a2020202020202020666f7220286a203d206964783b206a203c20682d3e6e202d20313b206a2b2b29207b0d0a202020202020202020202020682d3e735b6a5d203d20682d3e735b6a202b20315d3b0d0a20202020202020207d0d0a2020202020202020682d3e735b682d3e6e202d20315d203d204e554c4c3b0d0a202020207d20656c73652069662028682d3e746167203d3d205550505f4b545f4f4e44414c494b29207b0d0a2020202020202020666f7220286a203d206964783b206a203c20682d3e6e202d20313b206a2b2b29207b0d0a202020202020202020202020682d3e645b6a5d203d20682d3e645b6a202b20315d3b0d0a20202020202020207d0d0a202020207d20656c7365207b0d0a2020202020202020666f7220286a203d206964783b206a203c20682d3e6e202d20313b206a2b2b29207b0d0a202020202020202020202020682d3e695b6a5d203d20682d3e695b6a202b20315d3b0d0a20202020202020207d0d0a202020207d0d0a20202020682d3e6e2d2d3b0d0a7d0d0a0d0a73746174696320766f6964207570705f6c697374655f74656d697a6c65285570704b6f6c4c697374652a204c29207b0d0a202020205f5570704b6f6c4c69737465482a20683b0d0a202020206c6f6e67206c6f6e67206a3b0d0a2020202069662028214c207c7c20"));
        tampon_yaz(upp_metin_hex_coz("4c2d3e6964203d3d2030292072657475726e3b0d0a2020202068203d205f7570705f6c697374655f6875637265284c2d3e6964293b0d0a20202020696620282168292072657475726e3b0d0a2020202069662028682d3e746167203d3d205550505f4b545f4d4554494e20262620682d3e7329207b0d0a2020202020202020666f7220286a203d20303b206a203c20682d3e6e3b206a2b2b29207b0d0a20202020202020202020202069662028682d3e735b6a5d29206672656528682d3e735b6a5d293b0d0a202020202020202020202020682d3e735b6a5d203d204e554c4c3b0d0a20202020202020207d0d0a202020207d0d0a20202020682d3e6e203d20303b0d0a7d0d0a0d0a737461746963205f5570704b6f6c486172697461482a205f7570705f6861726974615f6875637265286c6f6e67206c6f6e6720696429207b0d0a20202020696620286964203c3d2030207c7c206964203e3d205f7570705f6861726974615f6e207c7c20215f7570705f6861726974616c617229207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202069662028215f7570705f6861726974616c61725b69645d2e63616e6c6929207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475726e20265f7570705f6861726974616c61725b69645d3b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6861726974615f79656e6928696e74206b7461672c20696e74207674616729207b0d0a202020206c6f6e67206c6f6e6720693b0d0a202020206c6f6e67206c6f6e67206e633b0d0a202020205f5570704b6f6c486172697461482a206e703b0d0a20202020666f72202869203d20313b2069203c205f7570705f6861726974615f6e3b20692b2b29207b0d0a202020202020202069662028215f7570705f6861726974616c61725b695d2e63616e6c6929207b0d0a2020202020202020202020206d656d73657428265f7570705f6861726974616c61725b695d2c20302c2073697a656f66285f5570704b6f6c4861726974614829293b0d0a2020202020202020202020205f7570705f6861726974616c61725b695d2e63616e6c69203d20313b0d0a2020202020202020202020205f7570705f6861726974616c61725b695d2e6b746167203d206b7461673b0d0a2020202020202020202020205f7570705f6861726974616c61725b695d2e76746167203d20767461673b0d0a20202020202020202020202072657475726e20693b0d0a20202020202020207d0d0a202020207d0d0a20202020696620285f7570705f6861726974615f6e203e3d205550505f4b4f4c4c454b5349594f4e5f4d415829207b0d0a202020202020202072657475726e"));
        tampon_yaz(upp_metin_hex_coz("20303b0d0a202020207d0d0a20202020696620285f7570705f6861726974615f6e202b2031203e205f7570705f6861726974615f63617029207b0d0a20202020202020206e63203d205f7570705f6861726974615f636170203f205f7570705f6861726974615f636170202a2032203a2031363b0d0a2020202020202020696620286e63203c205f7570705f6861726974615f6e202b203129207b0d0a2020202020202020202020206e63203d205f7570705f6861726974615f6e202b20313b0d0a20202020202020207d0d0a20202020202020206e70203d20285f5570704b6f6c486172697461482a297265616c6c6f63285f7570705f6861726974616c61722c202873697a655f74296e63202a2073697a656f66285f5570704b6f6c4861726974614829293b0d0a202020202020202069662028216e7029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020206d656d736574286e70202b205f7570705f6861726974615f6361702c20302c202873697a655f7429286e63202d205f7570705f6861726974615f63617029202a2073697a656f66285f5570704b6f6c4861726974614829293b0d0a20202020202020205f7570705f6861726974616c6172203d206e703b0d0a20202020202020205f7570705f6861726974615f636170203d206e633b0d0a202020207d0d0a2020202069203d205f7570705f6861726974615f6e2b2b3b0d0a202020206d656d73657428265f7570705f6861726974616c61725b695d2c20302c2073697a656f66285f5570704b6f6c4861726974614829293b0d0a202020205f7570705f6861726974616c61725b695d2e63616e6c69203d20313b0d0a202020205f7570705f6861726974616c61725b695d2e6b746167203d206b7461673b0d0a202020205f7570705f6861726974616c61725b695d2e76746167203d20767461673b0d0a2020202072657475726e20693b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6861726974615f68617a6972285570704b6f6c4861726974612a20482c20696e74206b7461672c20696e74207674616729207b0d0a2020202069662028214829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028482d3e6964203d3d203029207b0d0a2020202020202020482d3e6964203d205f7570705f6861726974615f79656e69286b7461672c2076746167293b0d0a202020202020202069662028482d3e6964203d3d203029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020207d0d0a202020207b0d0a20202020202020205f5570704b6f6c486172697461482a2068203d205f7570705f6861726974"));
        tampon_yaz(upp_metin_hex_coz("615f687563726528482d3e6964293b0d0a2020202020202020696620282168207c7c20682d3e6b74616720213d206b746167207c7c20682d3e7674616720213d207674616729207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202072657475726e20313b0d0a202020207d0d0a7d0d0a0d0a23646566696e65205550505f485f454d50545920282d314c4c290d0a23646566696e65205550505f485f544f4d422020282d324c4c290d0a0d0a73746174696320756e7369676e6564205f7570705f666e7628636f6e737420636861722a207329207b0d0a20202020756e7369676e65642068203d2032313636313336323631753b0d0a20202020636f6e737420756e7369676e656420636861722a2070203d2028636f6e737420756e7369676e656420636861722a292873203f2073203a202222293b0d0a202020207768696c6520282a7029207b0d0a202020202020202068205e3d202a702b2b3b0d0a202020202020202068202a3d203136373737363139753b0d0a202020207d0d0a2020202072657475726e2068203f2068203a2031753b0d0a7d0d0a0d0a73746174696320756e7369676e6564205f7570705f6d69785f69286c6f6e67206c6f6e67206b29207b0d0a20202020756e7369676e6564206c6f6e67206c6f6e672078203d2028756e7369676e6564206c6f6e67206c6f6e67296b202a20307839453337373942393746344137433135756c6c3b0d0a2020202078205e3d2078203e3e2033323b0d0a2020202072657475726e2028756e7369676e65642978203f2028756e7369676e65642978203a2031753b0d0a7d0d0a0d0a73746174696320756e7369676e6564205f7570705f686b6579285f5570704b6f6c486172697461482a20682c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a2020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e29207b0d0a202020202020202072657475726e205f7570705f666e76286b73293b0d0a202020207d0d0a2020202072657475726e205f7570705f6d69785f69286b69293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6865736974285f5570704b6f6c486172697461482a20682c206c6f6e67206c6f6e6720692c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a2020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020636f6e737420636861722a2061203d20682d3e6b5f735b695d203f20682d3e6b5f735b695d203a2022223b0d0a2020202020202020636f6e737420636861722a2062203d206b73203f206b73203a2022223b0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("202020202072657475726e20737472636d7028612c206229203d3d20303b0d0a202020207d0d0a2020202072657475726e20682d3e6b5f695b695d203d3d206b693b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6870726f6265285f5570704b6f6c486172697461482a20682c20756e7369676e65642068762c206c6f6e67206c6f6e67207374657029207b0d0a202020206c6f6e67206c6f6e6720636170203d20682d3e686173685f6361703b0d0a2020202069662028636170203c3d203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620282863617020262028636170202d20312929203d3d203029207b0d0a202020202020202072657475726e20286c6f6e67206c6f6e672928286876202b2028756e7369676e656429737465702920262028756e7369676e65642928636170202d203129293b0d0a202020207d0d0a2020202072657475726e20286c6f6e67206c6f6e672928286876202b2028756e7369676e656429737465702920252028756e7369676e656429636170293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6861726974615f726568617368285f5570704b6f6c486172697461482a20682c206c6f6e67206c6f6e672063617029207b0d0a202020206c6f6e67206c6f6e672a206e683b0d0a202020206c6f6e67206c6f6e6720693b0d0a202020206c6f6e67206c6f6e67206e203d20383b0d0a2020202069662028216829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028636170203c203829207b0d0a2020202020202020636170203d20383b0d0a202020207d0d0a202020207768696c6520286e203c2063617029207b0d0a2020202020202020696620286e203e20284c4c4f4e475f4d4158202f20322929207b0d0a2020202020202020202020206e203d206361703b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a20202020202020206e202a3d20323b0d0a202020207d0d0a20202020636170203d206e3b0d0a202020206e68203d20286c6f6e67206c6f6e672a296d616c6c6f63282873697a655f7429636170202a2073697a656f66286c6f6e67206c6f6e6729293b0d0a2020202069662028216e6829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020666f72202869203d20303b2069203c206361703b20692b2b29207b0d0a20202020202020206e685b695d203d205550505f485f454d5054593b0d0a202020207d0d0a202020206672656528682d3e68617368293b0d0a20202020682d3e68617368203d206e683b0d0a20202020682d3e686173685f636170203d206361703b0d0a20202020"));
        tampon_yaz(upp_metin_hex_coz("682d3e686173685f746f6d62203d20303b0d0a20202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020756e7369676e6564206876203d205f7570705f686b657928682c20682d3e6b746167203d3d205550505f4b545f4d4554494e203f2030203a20682d3e6b5f695b695d2c0d0a20202020202020202020202020202020202020202020202020202020202020682d3e6b746167203d3d205550505f4b545f4d4554494e203f20682d3e6b5f735b695d203a204e554c4c293b0d0a20202020202020206c6f6e67206c6f6e672073746570203d20303b0d0a2020202020202020666f7220283b3b29207b0d0a2020202020202020202020206c6f6e67206c6f6e672070203d205f7570705f6870726f626528682c2068762c2073746570293b0d0a20202020202020202020202069662028682d3e686173685b705d203d3d205550505f485f454d50545929207b0d0a20202020202020202020202020202020682d3e686173685b705d203d20693b0d0a20202020202020202020202020202020627265616b3b0d0a2020202020202020202020207d0d0a202020202020202020202020737465702b2b3b0d0a2020202020202020202020206966202873746570203e3d2063617029207b0d0a2020202020202020202020202020202066726565286e68293b0d0a20202020202020202020202020202020682d3e68617368203d204e554c4c3b0d0a20202020202020202020202020202020682d3e686173685f636170203d20303b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d0d0a202020207d0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f6861726974615f686173685f707574285f5570704b6f6c486172697461482a20682c206c6f6e67206c6f6e672069647829207b0d0a20202020756e7369676e65642068763b0d0a202020206c6f6e67206c6f6e672073746570203d20303b0d0a202020206c6f6e67206c6f6e6720746f6d62203d202d313b0d0a202020206c6f6e67206c6f6e67206361703b0d0a20202020696620282168207c7c2021682d3e68617368207c7c20682d3e686173685f636170203c203829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020636170203d20682d3e686173685f6361703b0d0a202020206876203d205f7570705f686b657928682c20682d3e6b746167203d3d205550505f4b545f4d4554494e203f2030203a20682d3e6b5f695b6964785d2c0d0a20202020202020202020202020202020202020682d3e6b746167203d3d205550505f4b545f4d4554494e203f20682d3e6b5f73"));
        tampon_yaz(upp_metin_hex_coz("5b6964785d203a204e554c4c293b0d0a20202020666f7220283b3b29207b0d0a20202020202020206c6f6e67206c6f6e672070203d205f7570705f6870726f626528682c2068762c2073746570293b0d0a20202020202020206c6f6e67206c6f6e6720736c203d20682d3e686173685b705d3b0d0a202020202020202069662028736c203d3d205550505f485f454d50545929207b0d0a20202020202020202020202069662028746f6d62203e3d203029207b0d0a20202020202020202020202020202020682d3e686173685b746f6d625d203d206964783b0d0a2020202020202020202020202020202069662028682d3e686173685f746f6d62203e203029207b0d0a2020202020202020202020202020202020202020682d3e686173685f746f6d622d2d3b0d0a202020202020202020202020202020207d0d0a2020202020202020202020207d20656c7365207b0d0a20202020202020202020202020202020682d3e686173685b705d203d206964783b0d0a2020202020202020202020207d0d0a20202020202020202020202072657475726e3b0d0a20202020202020207d0d0a202020202020202069662028736c203d3d205550505f485f544f4d4220262620746f6d62203c203029207b0d0a202020202020202020202020746f6d62203d20703b0d0a20202020202020207d0d0a2020202020202020737465702b2b3b0d0a20202020202020206966202873746570203e3d2063617029207b0d0a20202020202020202020202072657475726e3b0d0a20202020202020207d0d0a202020207d0d0a7d0d0a0d0a73746174696320766f6964205f7570705f6861726974615f696e64656b735f6b6179646574285f5570704b6f6c486172697461482a20682c206c6f6e67206c6f6e672069647829207b0d0a2020202069662028216829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020206966202821682d3e68617368207c7c20682d3e686173685f636170203c2038207c7c2028682d3e6e202b20682d3e686173685f746f6d6229202a20324c4c203e3d20682d3e686173685f63617029207b0d0a20202020202020206c6f6e67206c6f6e67206e63203d20682d3e686173685f636170203f20682d3e686173685f636170202a2032203a20383b0d0a20202020202020205f7570705f6861726974615f72656861736828682c206e63293b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020205f7570705f6861726974615f686173685f70757428682c20696478293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6861726974615f6275797574285f5570704b6f6c486172697461482a206829207b0d0a202020206c6f6e67206c6f6e67206e633b0d0a20202020696620"));
        tampon_yaz(upp_metin_hex_coz("28216829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028682d3e636170203e203020262620682d3e636170203e20284c4c4f4e475f4d4158202f20322929207b0d0a20202020202020207570705f70616e696b2822686172697461206b61706173697465207461c59f6d6173c4b122293b0d0a202020207d0d0a202020206e63203d20682d3e636170203f20682d3e636170202a2032203a20383b0d0a2020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020636861722a2a2070203d2028636861722a2a297265616c6c6f6328682d3e6b5f732c202873697a655f74296e63202a2073697a656f6628636861722a29293b0d0a202020202020202069662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e6b5f73203d20703b0d0a202020207d20656c7365207b0d0a20202020202020206c6f6e67206c6f6e672a2070203d20286c6f6e67206c6f6e672a297265616c6c6f6328682d3e6b5f692c202873697a655f74296e63202a2073697a656f66286c6f6e67206c6f6e6729293b0d0a202020202020202069662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e6b5f69203d20703b0d0a202020207d0d0a2020202069662028682d3e76746167203d3d205550505f4b545f4f4e44414c494b29207b0d0a2020202020202020646f75626c652a2070203d2028646f75626c652a297265616c6c6f6328682d3e765f642c202873697a655f74296e63202a2073697a656f6628646f75626c6529293b0d0a202020202020202069662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e765f64203d20703b0d0a202020207d20656c73652069662028682d3e76746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020636861722a2a2070203d2028636861722a2a297265616c6c6f6328682d3e765f732c202873697a655f74296e63202a2073697a656f6628636861722a29293b0d0a202020202020202069662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e765f73203d20703b0d0a202020207d20656c7365207b0d0a20202020202020206c6f6e67206c6f6e672a2070203d20286c6f6e67206c6f6e672a297265616c6c6f6328682d3e765f692c202873697a655f74296e63202a2073697a656f66286c6f6e67206c6f6e6729293b0d0a2020202020202020"));
        tampon_yaz(upp_metin_hex_coz("69662028217029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020682d3e765f69203d20703b0d0a202020207d0d0a20202020682d3e636170203d206e633b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6861726974615f62756c285f5570704b6f6c486172697461482a20682c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a20202020756e7369676e65642068763b0d0a202020206c6f6e67206c6f6e6720737465703b0d0a202020206c6f6e67206c6f6e67206361703b0d0a20202020696620282168207c7c20682d3e6e203c3d203029207b0d0a202020202020202072657475726e202d313b0d0a202020207d0d0a202020206966202821682d3e68617368207c7c20682d3e686173685f636170203c203829207b0d0a20202020202020206c6f6e67206c6f6e6720693b0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a202020202020202020202020696620285f7570705f686573697428682c20692c206b692c206b732929207b0d0a2020202020202020202020202020202072657475726e20693b0d0a2020202020202020202020207d0d0a20202020202020207d0d0a202020202020202072657475726e202d313b0d0a202020207d0d0a20202020636170203d20682d3e686173685f6361703b0d0a202020206876203d205f7570705f686b657928682c206b692c206b73293b0d0a20202020666f72202873746570203d20303b2073746570203c206361703b20737465702b2b29207b0d0a20202020202020206c6f6e67206c6f6e672070203d205f7570705f6870726f626528682c2068762c2073746570293b0d0a20202020202020206c6f6e67206c6f6e6720736c203d20682d3e686173685b705d3b0d0a202020202020202069662028736c203d3d205550505f485f454d50545929207b0d0a20202020202020202020202072657475726e202d313b0d0a20202020202020207d0d0a202020202020202069662028736c20213d205550505f485f544f4d4220262620736c203e3d203020262620736c203c20682d3e6e202626205f7570705f686573697428682c20736c2c206b692c206b732929207b0d0a20202020202020202020202072657475726e20736c3b0d0a20202020202020207d0d0a202020207d0d0a2020202072657475726e202d313b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f6861726974615f79617a5f6465676572285f5570704b6f6c486172697461482a20682c206c6f6e67206c6f6e6720692c206c6f6e67206c6f6e672076692c20646f75626c652076"));
        tampon_yaz(upp_metin_hex_coz("642c20636f6e737420636861722a20767329207b0d0a2020202069662028682d3e76746167203d3d205550505f4b545f4f4e44414c494b29207b0d0a2020202020202020682d3e765f645b695d203d2076643b0d0a202020207d20656c73652069662028682d3e76746167203d3d205550505f4b545f4d4554494e29207b0d0a202020202020202069662028682d3e765f735b695d29207b0d0a2020202020202020202020206672656528682d3e765f735b695d293b0d0a20202020202020207d0d0a2020202020202020682d3e765f735b695d203d205f7570705f6b6f6c5f6b6f707961287673293b0d0a202020207d20656c7365207b0d0a2020202020202020682d3e765f695b695d203d2076693b0d0a202020207d0d0a7d0d0a0d0a73746174696320766f6964207570705f6861726974615f6b6f79285570704b6f6c4861726974612a20482c20696e74206b7461672c20696e7420767461672c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b732c206c6f6e67206c6f6e672076692c20646f75626c652076642c20636f6e737420636861722a20767329207b0d0a202020205f5570704b6f6c486172697461482a20683b0d0a202020206c6f6e67206c6f6e6720663b0d0a2020202069662028215f7570705f6861726974615f68617a697228482c206b7461672c20767461672929207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202068203d205f7570705f6861726974615f687563726528482d3e6964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202066203d205f7570705f6861726974615f62756c28682c206b692c206b73293b0d0a202020206966202866203e3d203029207b0d0a20202020202020205f7570705f6861726974615f79617a5f646567657228682c20662c2076692c2076642c207673293b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202069662028682d3e6e203e3d20682d3e63617020262620215f7570705f6861726974615f627579757428682929207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202069662028682d3e6e203e3d20682d3e63617029207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020696620286b746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020682d3e6b5f735b682d3e6e5d203d205f7570705f6b6f6c5f6b6f707961286b73293b0d0a202020207d20656c7365207b0d0a2020202020202020682d3e6b5f695b682d3e6e5d203d206b693b0d0a202020207d0d0a202020206966202876746167203d3d205550505f4b545f4f4e44414c494b"));
        tampon_yaz(upp_metin_hex_coz("29207b0d0a2020202020202020682d3e765f645b682d3e6e5d203d2076643b0d0a202020207d20656c7365206966202876746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020682d3e765f735b682d3e6e5d203d205f7570705f6b6f6c5f6b6f707961287673293b0d0a202020207d20656c7365207b0d0a2020202020202020682d3e765f695b682d3e6e5d203d2076693b0d0a202020207d0d0a20202020682d3e6e2b2b3b0d0a202020205f7570705f6861726974615f696e64656b735f6b617964657428682c20682d3e6e202d2031293b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6861726974615f616c5f69285570704b6f6c48617269746120482c20696e74206b7461672c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a202020205f5570704b6f6c486172697461482a2068203d205f7570705f6861726974615f687563726528482e6964293b0d0a202020206c6f6e67206c6f6e6720663b0d0a20202020696620282168207c7c20682d3e6b74616720213d206b746167207c7c20682d3e76746167203d3d205550505f4b545f4f4e44414c494b207c7c20682d3e76746167203d3d205550505f4b545f4d4554494e29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202066203d205f7570705f6861726974615f62756c28682c206b692c206b73293b0d0a2020202072657475726e2066203c2030203f2030203a20682d3e765f695b665d3b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f6861726974615f616c5f64285570704b6f6c48617269746120482c20696e74206b7461672c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a202020205f5570704b6f6c486172697461482a2068203d205f7570705f6861726974615f687563726528482e6964293b0d0a202020206c6f6e67206c6f6e6720663b0d0a20202020696620282168207c7c20682d3e6b74616720213d206b746167207c7c20682d3e7674616720213d205550505f4b545f4f4e44414c494b29207b0d0a202020202020202072657475726e20302e303b0d0a202020207d0d0a2020202066203d205f7570705f6861726974615f62756c28682c206b692c206b73293b0d0a2020202072657475726e2066203c2030203f20302e30203a20682d3e765f645b665d3b0d0a7d0d0a0d0a73746174696320636861722a207570705f6861726974615f616c5f73285570704b6f6c48617269746120482c20696e74206b7461672c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a202020205f5570704b6f6c486172697461482a2068203d205f7570"));
        tampon_yaz(upp_metin_hex_coz("705f6861726974615f687563726528482e6964293b0d0a202020206c6f6e67206c6f6e6720663b0d0a20202020696620282168207c7c20682d3e6b74616720213d206b746167207c7c20682d3e7674616720213d205550505f4b545f4d4554494e29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202066203d205f7570705f6861726974615f62756c28682c206b692c206b73293b0d0a2020202072657475726e2066203c2030203f204e554c4c203a2028682d3e765f735b665d203f205f7570705f6b6f6c5f6b6f70796128682d3e765f735b665d29203a204e554c4c293b0d0a7d0d0a0d0a73746174696320696e74207570705f6861726974615f7661725f6d69285570704b6f6c48617269746120482c20696e74206b7461672c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a202020205f5570704b6f6c486172697461482a2068203d205f7570705f6861726974615f687563726528482e6964293b0d0a20202020696620282168207c7c20682d3e6b74616720213d206b74616729207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e205f7570705f6861726974615f62756c28682c206b692c206b7329203e3d20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6861726974615f757a756e6c756b285570704b6f6c486172697461204829207b0d0a202020205f5570704b6f6c486172697461482a2068203d205f7570705f6861726974615f687563726528482e6964293b0d0a2020202072657475726e2068203f20682d3e6e203a20303b0d0a7d0d0a0d0a73746174696320766f6964207570705f6861726974615f626f73616c74285570704b6f6c4861726974612a204829207b0d0a202020205f5570704b6f6c486172697461482a20683b0d0a202020206c6f6e67206c6f6e6720693b0d0a20202020696620282148207c7c20482d3e6964203d3d203029207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202068203d205f7570705f6861726974615f687563726528482d3e6964293b0d0a2020202069662028216829207b0d0a2020202020202020482d3e6964203d20303b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e20262620682d3e6b5f7329207b0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020202020206672656528682d3e6b5f735b695d293b0d0a20202020202020207d0d0a202020207d0d0a2020202069662028682d3e76746167203d3d205550"));
        tampon_yaz(upp_metin_hex_coz("505f4b545f4d4554494e20262620682d3e765f7329207b0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020202020206672656528682d3e765f735b695d293b0d0a20202020202020207d0d0a202020207d0d0a202020206672656528682d3e6b5f69293b0d0a202020206672656528682d3e6b5f73293b0d0a202020206672656528682d3e765f69293b0d0a202020206672656528682d3e765f64293b0d0a202020206672656528682d3e765f73293b0d0a202020206672656528682d3e68617368293b0d0a202020206d656d73657428682c20302c2073697a656f66285f5570704b6f6c4861726974614829293b0d0a20202020482d3e6964203d20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6861726974615f73696c285570704b6f6c4861726974612a20482c20696e74206b7461672c206c6f6e67206c6f6e67206b692c20636f6e737420636861722a206b7329207b0d0a202020205f5570704b6f6c486172697461482a20683b0d0a202020206c6f6e67206c6f6e67206964782c206a3b0d0a20202020696620282148207c7c20482d3e6964203d3d2030292072657475726e20303b0d0a2020202068203d205f7570705f6861726974615f687563726528482d3e6964293b0d0a20202020696620282168207c7c20682d3e6b74616720213d206b746167292072657475726e20303b0d0a20202020696478203d205f7570705f6861726974615f62756c28682c206b692c206b73293b0d0a2020202069662028696478203c2030292072657475726e20303b0d0a2020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e20262620682d3e6b5f735b6964785d29207b0d0a20202020202020206672656528682d3e6b5f735b6964785d293b0d0a202020207d0d0a2020202069662028682d3e76746167203d3d205550505f4b545f4d4554494e20262620682d3e765f735b6964785d29207b0d0a20202020202020206672656528682d3e765f735b6964785d293b0d0a202020207d0d0a20202020666f7220286a203d206964783b206a203c20682d3e6e202d20313b206a2b2b29207b0d0a202020202020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e2920682d3e6b5f735b6a5d203d20682d3e6b5f735b6a202b20315d3b0d0a2020202020202020656c736520682d3e6b5f695b6a5d203d20682d3e6b5f695b6a202b20315d3b0d0a0d0a202020202020202069662028682d3e76746167203d3d205550505f4b545f4d4554494e2920682d3e765f735b6a5d203d20682d3e765f735b6a202b20315d3b0d0a2020202020202020656c73652069662028682d3e76746167203d3d2055"));
        tampon_yaz(upp_metin_hex_coz("50505f4b545f4f4e44414c494b2920682d3e765f645b6a5d203d20682d3e765f645b6a202b20315d3b0d0a2020202020202020656c736520682d3e765f695b6a5d203d20682d3e765f695b6a202b20315d3b0d0a202020207d0d0a2020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e2920682d3e6b5f735b682d3e6e202d20315d203d204e554c4c3b0d0a2020202069662028682d3e76746167203d3d205550505f4b545f4d4554494e2920682d3e765f735b682d3e6e202d20315d203d204e554c4c3b0d0a20202020682d3e6e2d2d3b0d0a2020202069662028682d3e6861736820262620682d3e686173685f636170203e3d203829207b0d0a20202020202020205f7570705f6861726974615f72656861736828682c20682d3e686173685f636170293b0d0a202020207d0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320766f6964207570705f6861726974615f74656d697a6c65285570704b6f6c4861726974612a204829207b0d0a202020205f5570704b6f6c486172697461482a20683b0d0a202020206c6f6e67206c6f6e67206a3b0d0a20202020696620282148207c7c20482d3e6964203d3d2030292072657475726e3b0d0a2020202068203d205f7570705f6861726974615f687563726528482d3e6964293b0d0a20202020696620282168292072657475726e3b0d0a2020202069662028682d3e6b746167203d3d205550505f4b545f4d4554494e20262620682d3e6b5f7329207b0d0a2020202020202020666f7220286a203d20303b206a203c20682d3e6e3b206a2b2b29207b0d0a20202020202020202020202069662028682d3e6b5f735b6a5d29206672656528682d3e6b5f735b6a5d293b0d0a202020202020202020202020682d3e6b5f735b6a5d203d204e554c4c3b0d0a20202020202020207d0d0a202020207d0d0a2020202069662028682d3e76746167203d3d205550505f4b545f4d4554494e20262620682d3e765f7329207b0d0a2020202020202020666f7220286a203d20303b206a203c20682d3e6e3b206a2b2b29207b0d0a20202020202020202020202069662028682d3e765f735b6a5d29206672656528682d3e765f735b6a5d293b0d0a202020202020202020202020682d3e765f735b6a5d203d204e554c4c3b0d0a20202020202020207d0d0a202020207d0d0a20202020682d3e6e203d20303b0d0a20202020682d3e686173685f746f6d62203d20303b0d0a2020202069662028682d3e6861736820262620682d3e686173685f636170203e203029207b0d0a2020202020202020666f7220286a203d20303b206a203c20682d3e686173685f6361703b206a2b2b29207b0d0a202020202020202020202020682d3e686173685b6a5d203d20555050"));
        tampon_yaz(upp_metin_hex_coz("5f485f454d5054593b0d0a20202020202020207d0d0a202020207d0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6861726974615f616e61687461725f69285570704b6f6c48617269746120482c206c6f6e67206c6f6e67206929207b0d0a202020205f5570704b6f6c486172697461482a2068203d205f7570705f6861726974615f687563726528482e6964293b0d0a20202020696620282168207c7c20682d3e6b746167203d3d205550505f4b545f4d4554494e207c7c2069203c2030207c7c2069203e3d20682d3e6e29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202072657475726e20682d3e6b5f695b695d3b0d0a7d0d0a0d0a73746174696320636861722a207570705f6861726974615f616e61687461725f73285570704b6f6c48617269746120482c206c6f6e67206c6f6e67206929207b0d0a202020205f5570704b6f6c486172697461482a2068203d205f7570705f6861726974615f687563726528482e6964293b0d0a20202020696620282168207c7c20682d3e6b74616720213d205550505f4b545f4d4554494e207c7c2069203c2030207c7c2069203e3d20682d3e6e29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475726e20682d3e6b5f735b695d203f205f7570705f6b6f6c5f6b6f70796128682d3e6b5f735b695d29203a204e554c4c3b0d0a7d0d0a0d0a737461746963205570704b6f6c4c69737465207570705f6861726974615f616e61687461726c6172285570704b6f6c48617269746120482c20696e74206b74616729207b0d0a202020205570704b6f6c4c69737465204c3b0d0a202020205f5570704b6f6c486172697461482a20683b0d0a202020206c6f6e67206c6f6e6720693b0d0a202020204c2e6964203d20303b0d0a2020202068203d205f7570705f6861726974615f687563726528482e6964293b0d0a20202020696620282168207c7c20682d3e6b74616720213d206b74616729207b0d0a202020202020202072657475726e204c3b0d0a202020207d0d0a20202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020696620286b746167203d3d205550505f4b545f4d4554494e29207b0d0a2020202020202020202020207570705f6c697374655f656b6c6528264c2c205550505f4b545f4d4554494e2c20302c20302e302c20682d3e6b5f735b695d293b0d0a20202020202020207d20656c7365207b0d0a2020202020202020202020207570705f6c697374655f656b6c6528264c2c205550505f4b545f534159492c20682d3e6b5f695b695d2c20302e302c204e554c4c293b0d0a20202020202020207d0d0a202020207d"));
        tampon_yaz(upp_metin_hex_coz("0d0a2020202072657475726e204c3b0d0a7d0d0a0d0a737461746963205570704b6f6c4c69737465207570705f6d6574696e5f626f6c28636f6e737420636861722a20732c20636f6e737420636861722a2061796972616329207b0d0a202020205570704b6f6c4c69737465204c3b0d0a2020202073697a655f74206e2c206e613b0d0a202020204c2e6964203d20303b0d0a2020202069662028217329207b0d0a202020202020202073203d2022223b0d0a202020207d0d0a20202020696620282161796972616329207b0d0a2020202020202020617969726163203d2022223b0d0a202020207d0d0a202020206e203d207374726c656e2873293b0d0a202020206e61203d207374726c656e28617969726163293b0d0a20202020696620286e61203d3d203029207b0d0a202020202020202073697a655f742069203d20303b0d0a20202020202020207768696c65202869203c206e29207b0d0a2020202020202020202020206c6f6e67206c6f6e6720616476203d205f7570705f757466385f61647628732c206e2c2069293b0d0a2020202020202020202020206368617220746d705b385d3b0d0a20202020202020202020202069662028616476203c3d203029207b0d0a20202020202020202020202020202020627265616b3b0d0a2020202020202020202020207d0d0a202020202020202020202020696620282873697a655f7429616476203e2073697a656f6628746d7029202d20317529207b0d0a20202020202020202020202020202020616476203d20313b0d0a2020202020202020202020207d0d0a2020202020202020202020206d656d63707928746d702c2073202b20692c202873697a655f7429616476293b0d0a202020202020202020202020746d705b6164765d203d20303b0d0a2020202020202020202020207570705f6c697374655f656b6c6528264c2c205550505f4b545f4d4554494e2c20302c20302e302c20746d70293b0d0a20202020202020202020202069202b3d202873697a655f74296164763b0d0a20202020202020207d0d0a202020202020202072657475726e204c3b0d0a202020207d0d0a202020207b0d0a2020202020202020636f6e737420636861722a2070203d20733b0d0a2020202020202020636f6e737420636861722a20666f756e643b0d0a20202020202020207768696c65202828666f756e64203d2073747273747228702c20617969726163292920213d204e554c4c29207b0d0a20202020202020202020202073697a655f742074616b65203d202873697a655f742928666f756e64202d2070293b0d0a202020202020202020202020636861722a207061726361203d2028636861722a296d616c6c6f632874616b65202b203175293b0d0a20202020202020202020202069662028"));
        tampon_yaz(upp_metin_hex_coz("21706172636129207b0d0a2020202020202020202020202020202072657475726e204c3b0d0a2020202020202020202020207d0d0a2020202020202020202020206966202874616b6529207b0d0a202020202020202020202020202020206d656d6370792870617263612c20702c2074616b65293b0d0a2020202020202020202020207d0d0a20202020202020202020202070617263615b74616b655d203d20303b0d0a2020202020202020202020207570705f6c697374655f656b6c6528264c2c205550505f4b545f4d4554494e2c20302c20302e302c207061726361293b0d0a20202020202020202020202066726565287061726361293b0d0a20202020202020202020202070203d20666f756e64202b206e613b0d0a20202020202020207d0d0a20202020202020207570705f6c697374655f656b6c6528264c2c205550505f4b545f4d4554494e2c20302c20302e302c2070293b0d0a202020207d0d0a2020202072657475726e204c3b0d0a7d0d0a0d0a73746174696320636861722a207570705f6d6574696e5f6c697374655f6269726c6573746972285570704b6f6c4c69737465204c2c20636f6e737420636861722a2061796972616329207b0d0a202020205f5570704b6f6c4c69737465482a20683b0d0a202020206c6f6e67206c6f6e6720693b0d0a2020202073697a655f74206e612c206361702c20757365643b0d0a20202020636861722a206f75743b0d0a20202020696620282161796972616329207b0d0a2020202020202020617969726163203d2022223b0d0a202020207d0d0a202020206e61203d207374726c656e28617969726163293b0d0a2020202068203d205f7570705f6c697374655f6875637265284c2e6964293b0d0a20202020696620282168207c7c20682d3e74616720213d205550505f4b545f4d4554494e207c7c20682d3e6e203c3d203029207b0d0a20202020202020206f7574203d2028636861722a296d616c6c6f63283175293b0d0a202020202020202069662028216f757429207b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a20202020202020206f75745b305d203d20303b0d0a202020202020202072657475726e206f75743b0d0a202020207d0d0a20202020636170203d2031753b0d0a20202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020636170202b3d20682d3e735b695d203f207374726c656e28682d3e735b695d29203a2030753b0d0a20202020202020206966202869202b2031203c20682d3e6e29207b0d0a202020202020202020202020636170202b3d206e613b0d0a20202020202020207d0d0a202020207d0d0a202020206f7574203d2028636861722a296d61"));
        tampon_yaz(upp_metin_hex_coz("6c6c6f6328636170293b0d0a2020202069662028216f757429207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202075736564203d20303b0d0a20202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020636f6e737420636861722a207061726361203d20682d3e735b695d203f20682d3e735b695d203a2022223b0d0a202020202020202073697a655f74206e70203d207374726c656e287061726361293b0d0a20202020202020206d656d637079286f7574202b20757365642c2070617263612c206e70293b0d0a202020202020202075736564202b3d206e703b0d0a20202020202020206966202869202b2031203c20682d3e6e202626206e6129207b0d0a2020202020202020202020206d656d637079286f7574202b20757365642c206179697261632c206e61293b0d0a20202020202020202020202075736564202b3d206e613b0d0a20202020202020207d0d0a202020207d0d0a202020206f75745b757365645d203d20303b0d0a2020202072657475726e206f75743b0d0a7d0d0a0d0a2f2a202d2d2d2d207570702e6a736f6e202f207570702e737464696f202f207570702e73697374656d202876322e3529202d2d2d2d202a2f0d0a0d0a23696e636c756465203c66636e746c2e683e0d0a236966205550505f57494e0d0a23696e636c756465203c696f2e683e0d0a23656e6469660d0a23696e636c756465203c6572726e6f2e683e0d0a0d0a23646566696e65205550505f4a545f424f5320300d0a23646566696e65205550505f4a545f4e554c4c20310d0a23646566696e65205550505f4a545f4d414e54494b20320d0a23646566696e65205550505f4a545f5341594920330d0a23646566696e65205550505f4a545f4f4e44414c494b20340d0a23646566696e65205550505f4a545f4d4554494e20350d0a23646566696e65205550505f4a545f44495a4920360d0a23646566696e65205550505f4a545f4e45534e4520370d0a0d0a23646566696e65205550505f4a534f4e5f4d41585f444552494e4c494b2036340d0a23646566696e65205550505f4a534f4e5f4d41585f445547554d20313030303030304c4c0d0a0d0a7479706564656620737472756374207b0d0a20202020696e74207475723b0d0a20202020696e742063616e6c693b0d0a202020206c6f6e67206c6f6e6720736179693b0d0a20202020646f75626c65206f6e64616c696b3b0d0a20202020636861722a206d6574696e3b0d0a20202020636861722a2a20616e61687461726c61723b0d0a202020206c6f6e67206c6f6e672a206f67656c65723b0d0a202020206c6f6e67206c6f6e67206e3b0d0a202020206c6f6e67206c6f6e67206361703b0d0a7d20"));
        tampon_yaz(upp_metin_hex_coz("5570704a736f6e48756372653b0d0a0d0a7479706564656620737472756374207b0d0a20202020636f6e737420636861722a20733b0d0a2020202073697a655f74206c656e3b0d0a2020202073697a655f7420693b0d0a20202020696e7420686174613b0d0a20202020696e7420646572696e3b0d0a7d205570704a736f6e4f6b75797563753b0d0a0d0a7479706564656620737472756374207b0d0a20202020636861722a20703b0d0a2020202073697a655f74206e3b0d0a2020202073697a655f74206361703b0d0a7d205570704a736f6e59617a693b0d0a0d0a737461746963205570704a736f6e48756372652a205f7570705f6a736f6e5f68203d204e554c4c3b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f6e203d20313b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f636170203d20303b0d0a73746174696320696e74205f7570705f737464696f5f696b696c695f68617a6972203d20303b0d0a0d0a737461746963204a534f4e4465676572205f7570705f6a736f6e5f6964286c6f6e67206c6f6e6720696429207b0d0a202020204a534f4e446567657220763b0d0a20202020762e6964203d2069643b0d0a2020202072657475726e20763b0d0a7d0d0a0d0a737461746963205570704a736f6e48756372652a205f7570705f6a736f6e5f6875637265286c6f6e67206c6f6e6720696429207b0d0a20202020696620286964203c3d2030207c7c206964203e3d205f7570705f6a736f6e5f6e29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202069662028215f7570705f6a736f6e5f685b69645d2e63616e6c6929207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475726e20265f7570705f6a736f6e5f685b69645d3b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f79656e6928696e742074757229207b0d0a202020206c6f6e67206c6f6e6720693b0d0a202020205570704a736f6e48756372652a206e703b0d0a202020206c6f6e67206c6f6e67206e633b0d0a20202020666f72202869203d20313b2069203c205f7570705f6a736f6e5f6e3b20692b2b29207b0d0a202020202020202069662028215f7570705f6a736f6e5f685b695d2e63616e6c6929207b0d0a2020202020202020202020206d656d73657428265f7570705f6a736f6e5f685b695d2c20302c2073697a656f66285570704a736f6e487563726529293b0d0a2020202020202020202020205f7570705f6a736f6e5f685b695d2e747572203d207475723b0d0a2020202020202020202020205f7570705f6a736f6e5f685b695d2e63616e6c69203d20313b0d"));
        tampon_yaz(upp_metin_hex_coz("0a20202020202020202020202072657475726e20693b0d0a20202020202020207d0d0a202020207d0d0a20202020696620285f7570705f6a736f6e5f6e203e3d205550505f4a534f4e5f4d41585f445547554d29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620285f7570705f6a736f6e5f6e202b2031203e205f7570705f6a736f6e5f63617029207b0d0a20202020202020206e63203d205f7570705f6a736f6e5f636170203f205f7570705f6a736f6e5f636170202a2032203a2031363b0d0a2020202020202020696620286e63203c205f7570705f6a736f6e5f6e202b203129207b0d0a2020202020202020202020206e63203d205f7570705f6a736f6e5f6e202b20313b0d0a20202020202020207d0d0a20202020202020206e70203d20285570704a736f6e48756372652a297265616c6c6f63285f7570705f6a736f6e5f682c202873697a655f74296e63202a2073697a656f66285570704a736f6e487563726529293b0d0a202020202020202069662028216e7029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020206d656d736574286e70202b205f7570705f6a736f6e5f6361702c20302c202873697a655f7429286e63202d205f7570705f6a736f6e5f63617029202a2073697a656f66285570704a736f6e487563726529293b0d0a20202020202020205f7570705f6a736f6e5f68203d206e703b0d0a20202020202020205f7570705f6a736f6e5f636170203d206e633b0d0a202020207d0d0a2020202069203d205f7570705f6a736f6e5f6e2b2b3b0d0a202020206d656d73657428265f7570705f6a736f6e5f685b695d2c20302c2073697a656f66285570704a736f6e487563726529293b0d0a202020205f7570705f6a736f6e5f685b695d2e747572203d207475723b0d0a202020205f7570705f6a736f6e5f685b695d2e63616e6c69203d20313b0d0a2020202072657475726e20693b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6a736f6e5f6f67655f656b6c65285570704a736f6e48756372652a20682c206c6f6e67206c6f6e6720696429207b0d0a202020206c6f6e67206c6f6e67206e633b0d0a2020202069662028216829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028682d3e6e202b2031203e20682d3e63617029207b0d0a20202020202020206c6f6e67206c6f6e672a206f67656c6572323b0d0a2020202020202020636861722a2a20616e61687461726c617232203d204e554c4c3b0d0a20202020202020206e63203d20682d3e636170203f20682d3e636170202a2032203a20343b0d0a20202020202020206f67656c6572"));
        tampon_yaz(upp_metin_hex_coz("32203d20286c6f6e67206c6f6e672a296d616c6c6f63282873697a655f74296e63202a2073697a656f66286c6f6e67206c6f6e6729293b0d0a202020202020202069662028216f67656c65723229207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202069662028682d3e6e203e203020262620682d3e6f67656c657229207b0d0a2020202020202020202020206d656d637079286f67656c6572322c20682d3e6f67656c65722c202873697a655f7429682d3e6e202a2073697a656f66286c6f6e67206c6f6e6729293b0d0a20202020202020207d0d0a20202020202020206d656d736574286f67656c657232202b20682d3e6e2c20302c202873697a655f7429286e63202d20682d3e6e29202a2073697a656f66286c6f6e67206c6f6e6729293b0d0a202020202020202069662028682d3e747572203d3d205550505f4a545f4e45534e4529207b0d0a202020202020202020202020616e61687461726c617232203d2028636861722a2a296d616c6c6f63282873697a655f74296e63202a2073697a656f6628636861722a29293b0d0a2020202020202020202020206966202821616e61687461726c61723229207b0d0a2020202020202020202020202020202066726565286f67656c657232293b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020202020202069662028682d3e6e203e203020262620682d3e616e61687461726c617229207b0d0a202020202020202020202020202020206d656d63707928616e61687461726c6172322c20682d3e616e61687461726c61722c202873697a655f7429682d3e6e202a2073697a656f6628636861722a29293b0d0a2020202020202020202020207d0d0a2020202020202020202020206d656d73657428616e61687461726c617232202b20682d3e6e2c20302c202873697a655f7429286e63202d20682d3e6e29202a2073697a656f6628636861722a29293b0d0a20202020202020207d0d0a20202020202020206672656528682d3e6f67656c6572293b0d0a2020202020202020682d3e6f67656c6572203d206f67656c6572323b0d0a202020202020202069662028682d3e747572203d3d205550505f4a545f4e45534e4529207b0d0a2020202020202020202020206672656528682d3e616e61687461726c6172293b0d0a202020202020202020202020682d3e616e61687461726c6172203d20616e61687461726c6172323b0d0a20202020202020207d0d0a2020202020202020682d3e636170203d206e633b0d0a202020207d0d0a20202020682d3e6f67656c65725b682d3e6e5d203d2069643b0d0a20202020682d3e6e2b2b3b0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("2072657475726e20313b0d0a7d0d0a0d0a73746174696320636861722a205f7570705f6a736f6e5f6b6f70796128636f6e737420636861722a20732c2073697a655f74206e29207b0d0a20202020636861722a20703b0d0a2020202069662028217329207b0d0a202020202020202073203d2022223b0d0a20202020202020206e203d20303b0d0a202020207d0d0a2020202070203d2028636861722a296d616c6c6f63286e202b2031293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206d656d63707928702c20732c206e293b0d0a20202020705b6e5d203d20303b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f6a736f6e5f626f73616c745f6964286c6f6e67206c6f6e6720696429207b0d0a202020205570704a736f6e48756372652a20683b0d0a202020206c6f6e67206c6f6e6720693b0d0a2020202068203d205f7570705f6a736f6e5f6875637265286964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020682d3e63616e6c69203d20303b0d0a2020202069662028682d3e747572203d3d205550505f4a545f44495a49207c7c20682d3e747572203d3d205550505f4a545f4e45534e4529207b0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428682d3e6f67656c65725b695d293b0d0a20202020202020207d0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4e45534e4520262620682d3e616e61687461726c617229207b0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a2020202020202020202020206672656528682d3e616e61687461726c61725b695d293b0d0a20202020202020207d0d0a202020207d0d0a202020206672656528682d3e6d6574696e293b0d0a202020206672656528682d3e616e61687461726c6172293b0d0a202020206672656528682d3e6f67656c6572293b0d0a202020206d656d73657428682c20302c2073697a656f66285570704a736f6e487563726529293b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f6a736f6e5f61746c61285570704a736f6e4f6b75797563752a206f29207b0d0a202020207768696c6520286f2d3e69203c206f2d3e6c656e29207b0d0a2020202020202020636861722063203d206f2d3e735b6f2d3e695d3b0d0a2020202020202020696620286320213d20272027202626206320213d20275c742720262620632021"));
        tampon_yaz(upp_metin_hex_coz("3d20275c6e27202626206320213d20275c722729207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a20202020202020206f2d3e692b2b3b0d0a202020207d0d0a7d0d0a0d0a73746174696320696e74205f7570705f6a736f6e5f62656b6c656e656e285570704a736f6e4f6b75797563752a206f2c2063686172206329207b0d0a202020205f7570705f6a736f6e5f61746c61286f293b0d0a20202020696620286f2d3e69203e3d206f2d3e6c656e207c7c206f2d3e735b6f2d3e695d20213d206329207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206f2d3e692b2b3b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320696e74205f7570705f757466385f6b6f6428756e7369676e6564206c6f6e672063702c20636861722a206f757429207b0d0a20202020696620286370203c3d20307837467529207b0d0a20202020202020206f75745b305d203d2028636861722963703b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a20202020696620286370203c3d2030783746467529207b0d0a20202020202020206f75745b305d203d20286368617229283078433075207c20286370203e3e203629293b0d0a20202020202020206f75745b315d203d20286368617229283078383075207c20286370202620307833467529293b0d0a202020202020202072657475726e20323b0d0a202020207d0d0a20202020696620286370203c3d203078464646467529207b0d0a20202020202020206f75745b305d203d20286368617229283078453075207c20286370203e3e20313229293b0d0a20202020202020206f75745b315d203d20286368617229283078383075207c2028286370203e3e203629202620307833467529293b0d0a20202020202020206f75745b325d203d20286368617229283078383075207c20286370202620307833467529293b0d0a202020202020202072657475726e20333b0d0a202020207d0d0a20202020696620286370203c3d2030783130464646467529207b0d0a20202020202020206f75745b305d203d20286368617229283078463075207c20286370203e3e20313829293b0d0a20202020202020206f75745b315d203d20286368617229283078383075207c2028286370203e3e20313229202620307833467529293b0d0a20202020202020206f75745b325d203d20286368617229283078383075207c2028286370203e3e203629202620307833467529293b0d0a20202020202020206f75745b335d203d20286368617229283078383075207c20286370202620307833467529293b0d0a202020202020202072657475726e2034"));
        tampon_yaz(upp_metin_hex_coz("3b0d0a202020207d0d0a2020202072657475726e20303b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6865783428636f6e737420636861722a20732c20756e7369676e6564206c6f6e672a206f757429207b0d0a20202020756e7369676e6564206c6f6e672076203d20303b0d0a20202020696e7420693b0d0a20202020666f72202869203d20303b2069203c20343b20692b2b29207b0d0a2020202020202020756e7369676e656420636861722063203d2028756e7369676e6564206368617229735b695d3b0d0a202020202020202076203c3c3d20343b0d0a20202020202020206966202863203e3d202730272026262063203c3d2027392729207b0d0a20202020202020202020202076202b3d2028756e7369676e6564206c6f6e67292863202d20273027293b0d0a20202020202020207d20656c7365206966202863203e3d202761272026262063203c3d2027662729207b0d0a20202020202020202020202076202b3d2028756e7369676e6564206c6f6e67292863202d20276127202b203130293b0d0a20202020202020207d20656c7365206966202863203e3d202741272026262063203c3d2027462729207b0d0a20202020202020202020202076202b3d2028756e7369676e6564206c6f6e67292863202d20274127202b203130293b0d0a20202020202020207d20656c7365207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020207d0d0a202020202a6f7574203d20763b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f6f6b755f6465676572285570704a736f6e4f6b75797563752a206f293b0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f6f6b755f6d6574696e285570704a736f6e4f6b75797563752a206f29207b0d0a20202020636861722a206275663b0d0a2020202073697a655f74206e203d20303b0d0a2020202073697a655f7420636170203d2033323b0d0a202020206c6f6e67206c6f6e672069643b0d0a2020202069662028215f7570705f6a736f6e5f62656b6c656e656e286f2c202722272929207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020627566203d2028636861722a296d616c6c6f6328636170293b0d0a20202020696620282162756629207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020207768696c6520286f2d3e69203c206f2d3e6c656e29207b0d0a2020202020202020756e7369676e656420636861722063203d2028756e7369676e65642063686172296f2d3e735b6f2d3e695d3b0d"));
        tampon_yaz(upp_metin_hex_coz("0a20202020202020206368617220746d705b385d3b0d0a2020202020202020696e7420746e203d20313b0d0a20202020202020206966202863203d3d2027222729207b0d0a2020202020202020202020206f2d3e692b2b3b0d0a2020202020202020202020206275665b6e5d203d20303b0d0a2020202020202020202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4d4554494e293b0d0a2020202020202020202020206966202821696429207b0d0a202020202020202020202020202020206672656528627566293b0d0a202020202020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a2020202020202020202020205f7570705f6a736f6e5f685b69645d2e6d6574696e203d206275663b0d0a20202020202020202020202072657475726e2069643b0d0a20202020202020207d0d0a20202020202020206f2d3e692b2b3b0d0a20202020202020206966202863203d3d20275c5c2729207b0d0a202020202020202020202020696620286f2d3e69203e3d206f2d3e6c656e29207b0d0a202020202020202020202020202020206672656528627566293b0d0a202020202020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020202020202063203d2028756e7369676e65642063686172296f2d3e735b6f2d3e692b2b5d3b0d0a2020202020202020202020206966202863203d3d20272227207c7c2063203d3d20275c5c27207c7c2063203d3d20272f2729207b0d0a20202020202020202020202020202020746d705b305d203d20286368617229633b0d0a2020202020202020202020207d20656c7365206966202863203d3d2027622729207b0d0a20202020202020202020202020202020746d705b305d203d20275c62273b0d0a2020202020202020202020207d20656c7365206966202863203d3d2027662729207b0d0a20202020202020202020202020202020746d705b305d203d20275c66273b0d0a2020202020202020202020207d20656c7365206966202863203d3d20276e2729207b0d0a20202020202020202020202020202020746d705b305d203d20275c6e273b0d0a2020202020202020202020207d20656c7365206966202863203d3d2027722729207b0d0a20202020202020202020202020202020746d705b305d203d20275c72273b0d0a2020202020202020202020207d20656c7365206966202863203d3d2027742729207b0d0a20202020202020202020202020202020746d705b305d203d20275c74273b0d0a20202020202020"));
        tampon_yaz(upp_metin_hex_coz("20202020207d20656c7365206966202863203d3d2027752729207b0d0a20202020202020202020202020202020756e7369676e6564206c6f6e672063703b0d0a20202020202020202020202020202020696620286f2d3e69202b2034203e206f2d3e6c656e207c7c20215f7570705f68657834286f2d3e73202b206f2d3e692c202663702929207b0d0a20202020202020202020202020202020202020206672656528627566293b0d0a20202020202020202020202020202020202020206f2d3e68617461203d20313b0d0a202020202020202020202020202020202020202072657475726e20303b0d0a202020202020202020202020202020207d0d0a202020202020202020202020202020206f2d3e69202b3d20343b0d0a20202020202020202020202020202020696620286370203e3d2030784438303075202626206370203c3d2030784442464675202626206f2d3e69202b2036203c3d206f2d3e6c656e202626206f2d3e735b6f2d3e695d203d3d20275c5c27202626206f2d3e735b6f2d3e69202b20315d203d3d2027752729207b0d0a2020202020202020202020202020202020202020756e7369676e6564206c6f6e67206c6f3b0d0a2020202020202020202020202020202020202020696620285f7570705f68657834286f2d3e73202b206f2d3e69202b20322c20266c6f29202626206c6f203e3d2030784443303075202626206c6f203c3d203078444646467529207b0d0a2020202020202020202020202020202020202020202020206370203d203078313030303075202b202828286370202d203078443830307529203c3c20313029207c20286c6f202d203078444330307529293b0d0a2020202020202020202020202020202020202020202020206f2d3e69202b3d20363b0d0a20202020202020202020202020202020202020207d0d0a202020202020202020202020202020207d0d0a20202020202020202020202020202020746e203d205f7570705f757466385f6b6f642863702c20746d70293b0d0a2020202020202020202020202020202069662028746e203c3d203029207b0d0a20202020202020202020202020202020202020206672656528627566293b0d0a20202020202020202020202020202020202020206f2d3e68617461203d20313b0d0a202020202020202020202020202020202020202072657475726e20303b0d0a202020202020202020202020202020207d0d0a2020202020202020202020207d20656c7365207b0d0a202020202020202020202020202020206672656528627566293b0d0a202020202020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020"));
        tampon_yaz(upp_metin_hex_coz("207d20656c7365207b0d0a2020202020202020202020206966202863203c203078323029207b0d0a202020202020202020202020202020206672656528627566293b0d0a202020202020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a202020202020202020202020746d705b305d203d20286368617229633b0d0a20202020202020207d0d0a2020202020202020696620286e202b202873697a655f7429746e202b2031203e2063617029207b0d0a202020202020202020202020636861722a206e623b0d0a202020202020202020202020636170202a3d20323b0d0a20202020202020202020202069662028636170203c206e202b202873697a655f7429746e202b203129207b0d0a20202020202020202020202020202020636170203d206e202b202873697a655f7429746e202b20313b0d0a2020202020202020202020207d0d0a2020202020202020202020206e62203d2028636861722a297265616c6c6f63286275662c20636170293b0d0a20202020202020202020202069662028216e6229207b0d0a202020202020202020202020202020206672656528627566293b0d0a202020202020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a202020202020202020202020627566203d206e623b0d0a20202020202020207d0d0a20202020202020206d656d63707928627566202b206e2c20746d702c202873697a655f7429746e293b0d0a20202020202020206e202b3d202873697a655f7429746e3b0d0a202020207d0d0a202020206672656528627566293b0d0a202020206f2d3e68617461203d20313b0d0a2020202072657475726e20303b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f6f6b755f73617969285570704a736f6e4f6b75797563752a206f29207b0d0a2020202073697a655f74206261733b0d0a20202020696e74206f6e64616c696b203d20303b0d0a202020206368617220746d705b3132385d3b0d0a2020202073697a655f74206e3b0d0a202020206c6f6e67206c6f6e672069643b0d0a202020205f7570705f6a736f6e5f61746c61286f293b0d0a20202020626173203d206f2d3e693b0d0a20202020696620286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203d3d20272d2729207b0d0a20202020202020206f2d3e692b2b3b0d0a202020207d0d0a20202020696620286f2d3e69203e3d206f2d3e6c656e207c7c206f2d3e735b6f2d3e695d203c20273027207c7c206f2d3e735b6f2d3e695d20"));
        tampon_yaz(upp_metin_hex_coz("3e2027392729207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620286f2d3e735b6f2d3e695d203d3d2027302729207b0d0a20202020202020206f2d3e692b2b3b0d0a202020207d20656c7365207b0d0a20202020202020207768696c6520286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203e3d20273027202626206f2d3e735b6f2d3e695d203c3d2027392729207b0d0a2020202020202020202020206f2d3e692b2b3b0d0a20202020202020207d0d0a202020207d0d0a20202020696620286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203d3d20272e2729207b0d0a20202020202020206f6e64616c696b203d20313b0d0a20202020202020206f2d3e692b2b3b0d0a2020202020202020696620286f2d3e69203e3d206f2d3e6c656e207c7c206f2d3e735b6f2d3e695d203c20273027207c7c206f2d3e735b6f2d3e695d203e2027392729207b0d0a2020202020202020202020206f2d3e68617461203d20313b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020207768696c6520286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203e3d20273027202626206f2d3e735b6f2d3e695d203c3d2027392729207b0d0a2020202020202020202020206f2d3e692b2b3b0d0a20202020202020207d0d0a202020207d0d0a20202020696620286f2d3e69203c206f2d3e6c656e20262620286f2d3e735b6f2d3e695d203d3d20276527207c7c206f2d3e735b6f2d3e695d203d3d202745272929207b0d0a20202020202020206f6e64616c696b203d20313b0d0a20202020202020206f2d3e692b2b3b0d0a2020202020202020696620286f2d3e69203c206f2d3e6c656e20262620286f2d3e735b6f2d3e695d203d3d20272b27207c7c206f2d3e735b6f2d3e695d203d3d20272d272929207b0d0a2020202020202020202020206f2d3e692b2b3b0d0a20202020202020207d0d0a2020202020202020696620286f2d3e69203e3d206f2d3e6c656e207c7c206f2d3e735b6f2d3e695d203c20273027207c7c206f2d3e735b6f2d3e695d203e2027392729207b0d0a2020202020202020202020206f2d3e68617461203d20313b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020207768696c6520286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203e3d20273027202626206f2d3e735b6f2d3e695d203c3d2027392729207b0d0a2020202020202020202020206f2d3e692b2b3b0d0a20202020202020207d0d0a202020207d0d0a"));
        tampon_yaz(upp_metin_hex_coz("202020206e203d206f2d3e69202d206261733b0d0a20202020696620286e203e3d2073697a656f6628746d702929207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206d656d63707928746d702c206f2d3e73202b206261732c206e293b0d0a20202020746d705b6e5d203d20303b0d0a2020202069662028216f6e64616c696b29207b0d0a2020202020202020636861722a20656e64203d204e554c4c3b0d0a20202020202020206c6f6e67206c6f6e6720763b0d0a20202020202020206572726e6f203d20303b0d0a202020202020202076203d20737472746f6c6c28746d702c2026656e642c203130293b0d0a2020202020202020696620286572726e6f20213d204552414e474529207b0d0a2020202020202020202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f53415949293b0d0a2020202020202020202020206966202821696429207b0d0a202020202020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a2020202020202020202020205f7570705f6a736f6e5f685b69645d2e73617969203d20763b0d0a20202020202020202020202072657475726e2069643b0d0a20202020202020207d0d0a20202020202020206f6e64616c696b203d20313b0d0a202020207d0d0a202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4f4e44414c494b293b0d0a202020206966202821696429207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020205f7570705f6a736f6e5f685b69645d2e6f6e64616c696b203d20737472746f6428746d702c204e554c4c293b0d0a2020202072657475726e2069643b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6a736f6e5f6b656c696d65285570704a736f6e4f6b75797563752a206f2c20636f6e737420636861722a207729207b0d0a2020202073697a655f74206e203d207374726c656e2877293b0d0a202020205f7570705f6a736f6e5f61746c61286f293b0d0a20202020696620286f2d3e69202b206e203e206f2d3e6c656e207c7c206d656d636d70286f2d3e73202b206f2d3e692c20772c206e2920213d203029207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206f2d3e69202b3d206e3b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f6f6b755f64697a69285570704a736f6e4f6b75797563752a20"));
        tampon_yaz(upp_metin_hex_coz("6f29207b0d0a202020206c6f6e67206c6f6e672069643b0d0a2020202069662028215f7570705f6a736f6e5f62656b6c656e656e286f2c20275b272929207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f44495a49293b0d0a202020206966202821696429207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020205f7570705f6a736f6e5f61746c61286f293b0d0a20202020696620286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203d3d20275d2729207b0d0a20202020202020206f2d3e692b2b3b0d0a202020202020202072657475726e2069643b0d0a202020207d0d0a20202020666f7220283b3b29207b0d0a20202020202020206c6f6e67206c6f6e6720656c203d205f7570705f6a736f6e5f6f6b755f6465676572286f293b0d0a2020202020202020696620286f2d3e68617461207c7c2021656c207c7c20215f7570705f6a736f6e5f6f67655f656b6c6528265f7570705f6a736f6e5f685b69645d2c20656c2929207b0d0a2020202020202020202020206f2d3e68617461203d20313b0d0a20202020202020202020202069662028656c29207b0d0a202020202020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428656c293b0d0a2020202020202020202020207d0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020205f7570705f6a736f6e5f61746c61286f293b0d0a2020202020202020696620286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203d3d20272c2729207b0d0a2020202020202020202020206f2d3e692b2b3b0d0a202020202020202020202020636f6e74696e75653b0d0a20202020202020207d0d0a202020202020202069662028215f7570705f6a736f6e5f62656b6c656e656e286f2c20275d272929207b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202072657475726e2069643b0d0a202020207d0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f6f6b755f6e65736e65285570704a736f6e4f6b75797563752a206f29207b0d0a202020206c6f6e67206c6f6e672069643b0d0a2020202069662028215f7570705f6a736f6e5f62656b6c656e656e286f2c20277b27292920"));
        tampon_yaz(upp_metin_hex_coz("7b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4e45534e45293b0d0a202020206966202821696429207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020205f7570705f6a736f6e5f61746c61286f293b0d0a20202020696620286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203d3d20277d2729207b0d0a20202020202020206f2d3e692b2b3b0d0a202020202020202072657475726e2069643b0d0a202020207d0d0a20202020666f7220283b3b29207b0d0a20202020202020206c6f6e67206c6f6e67206b69643b0d0a20202020202020206c6f6e67206c6f6e67207669643b0d0a2020202020202020636861722a206b65793b0d0a20202020202020205f7570705f6a736f6e5f61746c61286f293b0d0a20202020202020206b6964203d205f7570705f6a736f6e5f6f6b755f6d6574696e286f293b0d0a2020202020202020696620286f2d3e68617461207c7c20216b696429207b0d0a2020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020206b6579203d205f7570705f6a736f6e5f685b6b69645d2e6d6574696e3b0d0a20202020202020205f7570705f6a736f6e5f685b6b69645d2e6d6574696e203d204e554c4c3b0d0a20202020202020205f7570705f6a736f6e5f626f73616c745f6964286b6964293b0d0a202020202020202069662028215f7570705f6a736f6e5f62656b6c656e656e286f2c20273a272929207b0d0a20202020202020202020202066726565286b6579293b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020766964203d205f7570705f6a736f6e5f6f6b755f6465676572286f293b0d0a2020202020202020696620286f2d3e68617461207c7c202176696429207b0d0a20202020202020202020202066726565286b6579293b0d0a2020202020202020202020206966202876696429207b0d0a202020202020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428766964293b0d0a2020202020202020202020207d0d0a2020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a"));
        tampon_yaz(upp_metin_hex_coz("20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202069662028215f7570705f6a736f6e5f6f67655f656b6c6528265f7570705f6a736f6e5f685b69645d2c207669642929207b0d0a20202020202020202020202066726565286b6579293b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428766964293b0d0a2020202020202020202020206f2d3e68617461203d20313b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020205f7570705f6a736f6e5f685b69645d2e616e61687461726c61725b5f7570705f6a736f6e5f685b69645d2e6e202d20315d203d206b65793b0d0a20202020202020205f7570705f6a736f6e5f61746c61286f293b0d0a2020202020202020696620286f2d3e69203c206f2d3e6c656e202626206f2d3e735b6f2d3e695d203d3d20272c2729207b0d0a2020202020202020202020206f2d3e692b2b3b0d0a202020202020202020202020636f6e74696e75653b0d0a20202020202020207d0d0a202020202020202069662028215f7570705f6a736f6e5f62656b6c656e656e286f2c20277d272929207b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020202020202072657475726e2069643b0d0a202020207d0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6a736f6e5f6f6b755f6465676572285570704a736f6e4f6b75797563752a206f29207b0d0a202020206c6f6e67206c6f6e672069643b0d0a202020205f7570705f6a736f6e5f61746c61286f293b0d0a20202020696620286f2d3e68617461207c7c206f2d3e69203e3d206f2d3e6c656e29207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620286f2d3e646572696e203e3d205550505f4a534f4e5f4d41585f444552494e4c494b29207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206f2d3e646572696e2b2b3b0d0a20202020696620286f2d3e735b6f2d3e695d203d3d2027222729207b0d0a20202020202020206964203d205f7570705f6a736f6e5f6f6b755f6d6574696e286f293b0d0a202020207d20656c736520696620286f2d3e735b6f2d3e695d203d3d20277b2729207b0d0a20202020202020206964203d205f7570705f"));
        tampon_yaz(upp_metin_hex_coz("6a736f6e5f6f6b755f6e65736e65286f293b0d0a202020207d20656c736520696620286f2d3e735b6f2d3e695d203d3d20275b2729207b0d0a20202020202020206964203d205f7570705f6a736f6e5f6f6b755f64697a69286f293b0d0a202020207d20656c736520696620286f2d3e735b6f2d3e695d203d3d20272d27207c7c20286f2d3e735b6f2d3e695d203e3d20273027202626206f2d3e735b6f2d3e695d203c3d202739272929207b0d0a20202020202020206964203d205f7570705f6a736f6e5f6f6b755f73617969286f293b0d0a202020207d20656c736520696620285f7570705f6a736f6e5f6b656c696d65286f2c202274727565222929207b0d0a20202020202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4d414e54494b293b0d0a202020202020202069662028696429207b0d0a2020202020202020202020205f7570705f6a736f6e5f685b69645d2e73617969203d20313b0d0a20202020202020207d0d0a202020207d20656c736520696620285f7570705f6a736f6e5f6b656c696d65286f2c202266616c7365222929207b0d0a20202020202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4d414e54494b293b0d0a202020202020202069662028696429207b0d0a2020202020202020202020205f7570705f6a736f6e5f685b69645d2e73617969203d20303b0d0a20202020202020207d0d0a202020207d20656c736520696620285f7570705f6a736f6e5f6b656c696d65286f2c20226e756c6c222929207b0d0a20202020202020206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4e554c4c293b0d0a202020207d20656c7365207b0d0a20202020202020206f2d3e68617461203d20313b0d0a20202020202020206964203d20303b0d0a202020207d0d0a202020206f2d3e646572696e2d2d3b0d0a202020206966202821696429207b0d0a20202020202020206f2d3e68617461203d20313b0d0a202020207d0d0a2020202072657475726e2069643b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6a736f6e5f79617a695f656b6c65285570704a736f6e59617a692a20792c20636f6e737420636861722a20732c2073697a655f74206c6e29207b0d0a20202020636861722a206e703b0d0a2020202073697a655f74206e633b0d0a2020202069662028217329207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a2020202069662028792d3e6e202b206c6e202b2031203e20792d3e63617029207b0d0a20202020202020206e63203d20792d3e636170203f20792d3e636170202a2032203a203235363b0d0a20202020202020207768696c6520286e63203c20792d3e6e202b206c6e202b20"));
        tampon_yaz(upp_metin_hex_coz("3129207b0d0a2020202020202020202020206e63202a3d20323b0d0a20202020202020207d0d0a20202020202020206e70203d2028636861722a297265616c6c6f6328792d3e702c206e63293b0d0a202020202020202069662028216e7029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020792d3e70203d206e703b0d0a2020202020202020792d3e636170203d206e633b0d0a202020207d0d0a202020206d656d63707928792d3e70202b20792d3e6e2c20732c206c6e293b0d0a20202020792d3e6e202b3d206c6e3b0d0a20202020792d3e705b792d3e6e5d203d20303b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6a736f6e5f79617a695f73285570704a736f6e59617a692a20792c20636f6e737420636861722a207329207b0d0a2020202072657475726e205f7570705f6a736f6e5f79617a695f656b6c6528792c20732c2073203f207374726c656e287329203a2030293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6a736f6e5f79617a695f6d6574696e285570704a736f6e59617a692a20792c20636f6e737420636861722a207329207b0d0a2020202073697a655f7420693b0d0a2020202073697a655f74206e3b0d0a2020202069662028215f7570705f6a736f6e5f79617a695f7328792c20225c22222929207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206e203d2073203f207374726c656e287329203a20303b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020756e7369676e656420636861722063203d2028756e7369676e6564206368617229735b695d3b0d0a202020202020202063686172206275665b385d3b0d0a20202020202020206966202863203d3d20272227207c7c2063203d3d20275c5c2729207b0d0a2020202020202020202020206275665b305d203d20275c5c273b0d0a2020202020202020202020206275665b315d203d20286368617229633b0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f656b6c6528792c206275662c20322929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d20656c7365206966202863203d3d20275c6e2729207b0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f7328792c20225c5c6e222929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d20656c7365206966202863203d"));
        tampon_yaz(upp_metin_hex_coz("3d20275c722729207b0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f7328792c20225c5c72222929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d20656c7365206966202863203d3d20275c742729207b0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f7328792c20225c5c74222929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d20656c7365206966202863203c203078323029207b0d0a202020202020202020202020736e7072696e7466286275662c2073697a656f6628627566292c20225c5c7525303478222c2063293b0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f7328792c206275662929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d20656c7365207b0d0a2020202020202020202020206275665b305d203d20286368617229633b0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f656b6c6528792c206275662c20312929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d0d0a202020207d0d0a2020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c20225c2222293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f6a736f6e5f79617a5f6964285570704a736f6e59617a692a20792c206c6f6e67206c6f6e672069642c20696e7420646572696e29207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f6875637265286964293b0d0a202020206368617220625b36345d3b0d0a202020206c6f6e67206c6f6e6720693b0d0a20202020696620282168207c7c20646572696e203e205550505f4a534f4e5f4d41585f444552494e4c494b29207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c20226e756c6c22293b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4e554c4c29207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c20226e756c6c22293b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4d414e54494b29207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c20682d3e73617969203f202274"));
        tampon_yaz(upp_metin_hex_coz("72756522203a202266616c736522293b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f5341594929207b0d0a2020202020202020736e7072696e746628622c2073697a656f662862292c2022256c6c64222c20682d3e73617969293b0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c2062293b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4f4e44414c494b29207b0d0a2020202020202020736e7072696e746628622c2073697a656f662862292c2022252e313767222c20682d3e6f6e64616c696b293b0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c2062293b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4d4554494e29207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f6d6574696e28792c20682d3e6d6574696e203f20682d3e6d6574696e203a202222293b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f44495a4929207b0d0a202020202020202069662028215f7570705f6a736f6e5f79617a695f7328792c20225b222929207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a202020202020202020202020696620286920262620215f7570705f6a736f6e5f79617a695f7328792c20222c222929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a5f696428792c20682d3e6f67656c65725b695d2c20646572696e202b20312929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c20225d22293b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4e45534e4529207b0d0a202020202020202069662028215f7570705f6a736f6e5f79617a695f7328792c20227b222929207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a2020202020202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a202020202020202020202020696620286920262620215f7570705f6a736f6e5f79617a695f7328792c20222c222929207b0d0a202020202020202020202020"));
        tampon_yaz(upp_metin_hex_coz("2020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f6d6574696e28792c20682d3e616e61687461726c61725b695d203f20682d3e616e61687461726c61725b695d203a2022222929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a695f7328792c20223a222929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020202020202069662028215f7570705f6a736f6e5f79617a5f696428792c20682d3e6f67656c65725b695d2c20646572696e202b20312929207b0d0a2020202020202020202020202020202072657475726e20303b0d0a2020202020202020202020207d0d0a20202020202020207d0d0a202020202020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c20227d22293b0d0a202020207d0d0a2020202072657475726e205f7570705f6a736f6e5f79617a695f7328792c20226e756c6c22293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f617972697374697228636f6e737420636861722a207329207b0d0a202020205570704a736f6e4f6b7579756375206f3b0d0a202020206c6f6e67206c6f6e672069643b0d0a202020206d656d73657428266f2c20302c2073697a656f66286f29293b0d0a202020206f2e73203d2073203f2073203a2022223b0d0a202020206f2e6c656e203d207374726c656e286f2e73293b0d0a20202020696620286f2e6c656e203e3d20332026262028756e7369676e65642063686172296f2e735b305d203d3d20307845462026262028756e7369676e65642063686172296f2e735b315d203d3d20307842422026262028756e7369676e65642063686172296f2e735b325d203d3d203078424629207b0d0a20202020202020206f2e69203d20333b0d0a202020207d0d0a202020206964203d205f7570705f6a736f6e5f6f6b755f646567657228266f293b0d0a202020205f7570705f6a736f6e5f61746c6128266f293b0d0a20202020696620286f2e68617461207c7c20216964207c7c206f2e6920213d206f2e6c656e29207b0d0a202020202020202069662028696429207b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a20202020202020207d0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a2020202072657475726e205f7570705f6a736f6e5f6964286964293b0d0a7d"));
        tampon_yaz(upp_metin_hex_coz("0d0a0d0a73746174696320636861722a207570705f6a736f6e5f6f6c7573747572284a534f4e4465676572207629207b0d0a202020205570704a736f6e59617a6920793b0d0a202020206d656d7365742826792c20302c2073697a656f66287929293b0d0a2020202069662028215f7570705f6a736f6e5f79617a5f69642826792c20762e69642c20302929207b0d0a20202020202020206672656528792e70293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475726e20792e70203f20792e70203a205f7570705f6a736f6e5f6b6f7079612822222c2030293b0d0a7d0d0a0d0a73746174696320636861722a207570705f6a736f6e5f747572284a534f4e4465676572207629207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e2028636861722a2922796f6b223b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4e554c4c29207b0d0a202020202020202072657475726e2028636861722a2922796f6b223b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4d414e54494b29207b0d0a202020202020202072657475726e2028636861722a29226d616e74696b223b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f5341594929207b0d0a202020202020202072657475726e2028636861722a292273617969223b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4f4e44414c494b29207b0d0a202020202020202072657475726e2028636861722a29226f6e64616c696b223b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4d4554494e29207b0d0a202020202020202072657475726e2028636861722a29226d6574696e223b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f44495a4929207b0d0a202020202020202072657475726e2028636861722a292264697a69223b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4e45534e4529207b0d0a202020202020202072657475726e2028636861722a29226e65736e65223b0d0a202020207d0d0a2020202072657475726e2028636861722a2922796f6b223b0d0a7d0d0a0d0a73746174696320626f6f6c207570705f6a736f6e5f7661725f6d69284a534f4e4465676572207629207b0d0a2020202072657475726e205f7570705f6a736f6e5f687563726528762e69642920213d204e554c4c3b0d0a7d0d0a"));
        tampon_yaz(upp_metin_hex_coz("0d0a737461746963206c6f6e67206c6f6e67207570705f6a736f6e5f757a756e6c756b284a534f4e4465676572207629207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f44495a49207c7c20682d3e747572203d3d205550505f4a545f4e45534e4529207b0d0a202020202020202072657475726e20682d3e6e3b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4d4554494e20262620682d3e6d6574696e29207b0d0a202020202020202072657475726e205f7570705f757466385f73617928682d3e6d6574696e2c207374726c656e28682d3e6d6574696e29293b0d0a202020207d0d0a2020202072657475726e20303b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f616e61687461725f616c284a534f4e446567657220762c20636f6e737420636861722a20616e616874617229207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a202020206c6f6e67206c6f6e6720693b0d0a20202020696620282168207c7c20682d3e74757220213d205550505f4a545f4e45534e45207c7c2021616e616874617229207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a20202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a202020202020202069662028682d3e616e61687461726c61725b695d20262620737472636d7028682d3e616e61687461726c61725b695d2c20616e616874617229203d3d203029207b0d0a20202020202020202020202072657475726e205f7570705f6a736f6e5f696428682d3e6f67656c65725b695d293b0d0a20202020202020207d0d0a202020207d0d0a2020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f64697a695f656c656d616e284a534f4e446567657220762c206c6f6e67206c6f6e67206929207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a20202020696620282168207c7c20682d3e74757220213d205550505f4a545f44495a49207c7c2069203c2030207c7c2069203e3d20682d3e6e29207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a2020202072657475"));
        tampon_yaz(upp_metin_hex_coz("726e205f7570705f6a736f6e5f696428682d3e6f67656c65725b695d293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f616e61687461726c6172284a534f4e4465676572207629207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a202020206c6f6e67206c6f6e67206172723b0d0a202020206c6f6e67206c6f6e6720693b0d0a20202020696620282168207c7c20682d3e74757220213d205550505f4a545f4e45534e4529207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a20202020617272203d205f7570705f6a736f6e5f79656e69285550505f4a545f44495a49293b0d0a20202020696620282161727229207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a20202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a20202020202020206c6f6e67206c6f6e6720736964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4d4554494e293b0d0a2020202020202020636f6e737420636861722a206b203d20682d3e616e61687461726c61725b695d203f20682d3e616e61687461726c61725b695d203a2022223b0d0a2020202020202020696620282173696429207b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428617272293b0d0a20202020202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a20202020202020207d0d0a20202020202020205f7570705f6a736f6e5f685b7369645d2e6d6574696e203d205f7570705f6a736f6e5f6b6f707961286b2c207374726c656e286b29293b0d0a202020202020202069662028215f7570705f6a736f6e5f685b7369645d2e6d6574696e29207b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428736964293b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428617272293b0d0a20202020202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a20202020202020207d0d0a202020202020202069662028215f7570705f6a736f6e5f6f67655f656b6c6528265f7570705f6a736f6e5f685b6172725d2c207369642929207b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428736964293b0d0a2020202020202020202020205f7570705f6a736f6e5f626f73616c745f696428617272293b0d0a20202020202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a2020"));
        tampon_yaz(upp_metin_hex_coz("2020202020207d0d0a202020207d0d0a2020202072657475726e205f7570705f6a736f6e5f696428617272293b0d0a7d0d0a0d0a73746174696320636861722a207570705f6a736f6e5f6d6574696e5f616c284a534f4e4465676572207629207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a20202020696620282168207c7c20682d3e74757220213d205550505f4a545f4d4554494e207c7c2021682d3e6d6574696e29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475726e205f7570705f6a736f6e5f6b6f70796128682d3e6d6574696e2c207374726c656e28682d3e6d6574696e29293b0d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6a736f6e5f736179695f616c284a534f4e4465676572207629207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f5341594929207b0d0a202020202020202072657475726e20682d3e736179693b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4f4e44414c494b29207b0d0a202020202020202072657475726e20286c6f6e67206c6f6e6729682d3e6f6e64616c696b3b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4d414e54494b29207b0d0a202020202020202072657475726e20682d3e73617969203f2031203a20303b0d0a202020207d0d0a2020202072657475726e20303b0d0a7d0d0a0d0a73746174696320646f75626c65207570705f6a736f6e5f6f6e64616c696b5f616c284a534f4e4465676572207629207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e20302e303b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4f4e44414c494b29207b0d0a202020202020202072657475726e20682d3e6f6e64616c696b3b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f5341594929207b0d0a202020202020202072657475726e2028646f75626c6529682d3e736179693b0d0a202020207d0d0a2020202072657475726e20302e303b0d0a7d0d0a0d0a73746174696320626f6f6c207570705f6a736f6e5f6d616e74696b5f616c284a534f4e4465676572207629207b"));
        tampon_yaz(upp_metin_hex_coz("0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f687563726528762e6964293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f4d414e54494b29207b0d0a202020202020202072657475726e20682d3e7361796920213d20303b0d0a202020207d0d0a2020202069662028682d3e747572203d3d205550505f4a545f5341594929207b0d0a202020202020202072657475726e20682d3e7361796920213d20303b0d0a202020207d0d0a2020202072657475726e20303b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f6e65736e6528766f696429207b0d0a2020202072657475726e205f7570705f6a736f6e5f6964285f7570705f6a736f6e5f79656e69285550505f4a545f4e45534e4529293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f64697a6928766f696429207b0d0a2020202072657475726e205f7570705f6a736f6e5f6964285f7570705f6a736f6e5f79656e69285550505f4a545f44495a4929293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f6d6574696e5f79617028636f6e737420636861722a207329207b0d0a202020206c6f6e67206c6f6e67206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4d4554494e293b0d0a202020206966202821696429207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a202020205f7570705f6a736f6e5f685b69645d2e6d6574696e203d205f7570705f6a736f6e5f6b6f7079612873203f2073203a2022222c2073203f207374726c656e287329203a2030293b0d0a2020202069662028215f7570705f6a736f6e5f685b69645d2e6d6574696e29207b0d0a20202020202020205f7570705f6a736f6e5f626f73616c745f6964286964293b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a2020202072657475726e205f7570705f6a736f6e5f6964286964293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f736179695f796170286c6f6e67206c6f6e67206e29207b0d0a202020206c6f6e67206c6f6e67206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f53415949293b0d0a202020206966202821696429207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a202020205f7570705f6a736f6e5f685b69645d2e73617969203d206e3b0d0a20"));
        tampon_yaz(upp_metin_hex_coz("20202072657475726e205f7570705f6a736f6e5f6964286964293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f6f6e64616c696b5f79617028646f75626c65206e29207b0d0a202020206c6f6e67206c6f6e67206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4f4e44414c494b293b0d0a202020206966202821696429207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a202020205f7570705f6a736f6e5f685b69645d2e6f6e64616c696b203d206e3b0d0a2020202072657475726e205f7570705f6a736f6e5f6964286964293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f6d616e74696b5f79617028626f6f6c206229207b0d0a202020206c6f6e67206c6f6e67206964203d205f7570705f6a736f6e5f79656e69285550505f4a545f4d414e54494b293b0d0a202020206966202821696429207b0d0a202020202020202072657475726e205f7570705f6a736f6e5f69642830293b0d0a202020207d0d0a202020205f7570705f6a736f6e5f685b69645d2e73617969203d2062203f2031203a20303b0d0a2020202072657475726e205f7570705f6a736f6e5f6964286964293b0d0a7d0d0a0d0a737461746963204a534f4e4465676572207570705f6a736f6e5f796f6b5f79617028766f696429207b0d0a2020202072657475726e205f7570705f6a736f6e5f6964285f7570705f6a736f6e5f79656e69285550505f4a545f4e554c4c29293b0d0a7d0d0a0d0a73746174696320766f6964207570705f6a736f6e5f6b6f79284a534f4e4465676572206e65736e652c20636f6e737420636861722a20616e61687461722c204a534f4e446567657220646567657229207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f6875637265286e65736e652e6964293b0d0a202020206c6f6e67206c6f6e6720693b0d0a20202020636861722a206b65793b0d0a20202020696620282168207c7c20682d3e74757220213d205550505f4a545f4e45534e45207c7c2021616e616874617229207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020666f72202869203d20303b2069203c20682d3e6e3b20692b2b29207b0d0a202020202020202069662028682d3e616e61687461726c61725b695d20262620737472636d7028682d3e616e61687461726c61725b695d2c20616e616874617229203d3d203029207b0d0a20202020202020202020202069662028682d3e6f67656c65725b695d20213d2064656765722e696429207b0d0a202020202020202020202020202020205f7570705f6a736f6e5f626f73616c74"));
        tampon_yaz(upp_metin_hex_coz("5f696428682d3e6f67656c65725b695d293b0d0a20202020202020202020202020202020682d3e6f67656c65725b695d203d2064656765722e69643b0d0a2020202020202020202020207d0d0a20202020202020202020202072657475726e3b0d0a20202020202020207d0d0a202020207d0d0a202020206b6579203d205f7570705f6a736f6e5f6b6f70796128616e61687461722c207374726c656e28616e616874617229293b0d0a2020202069662028216b6579207c7c20215f7570705f6a736f6e5f6f67655f656b6c6528682c2064656765722e69642929207b0d0a202020202020202066726565286b6579293b0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020682d3e616e61687461726c61725b682d3e6e202d20315d203d206b65793b0d0a7d0d0a0d0a73746174696320766f6964207570705f6a736f6e5f656b6c65284a534f4e44656765722064697a692c204a534f4e446567657220646567657229207b0d0a202020205570704a736f6e48756372652a2068203d205f7570705f6a736f6e5f68756372652864697a692e6964293b0d0a20202020696620282168207c7c20682d3e74757220213d205550505f4a545f44495a4929207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020205f7570705f6a736f6e5f6f67655f656b6c6528682c2064656765722e6964293b0d0a7d0d0a0d0a73746174696320766f6964207570705f6a736f6e5f626f73616c74284a534f4e4465676572207629207b0d0a202020205f7570705f6a736f6e5f626f73616c745f696428762e6964293b0d0a7d0d0a0d0a73746174696320696e74205f7570705f61736369695f6e69636d7028636f6e737420636861722a20612c20636f6e737420636861722a20622c2073697a655f74206e29207b0d0a2020202073697a655f7420693b0d0a20202020666f72202869203d20303b2069203c206e3b20692b2b29207b0d0a2020202020202020756e7369676e65642063686172206361203d2028756e7369676e6564206368617229615b695d3b0d0a2020202020202020756e7369676e65642063686172206362203d2028756e7369676e6564206368617229625b695d3b0d0a2020202020202020696620286361203e3d20274127202626206361203c3d20275a2729207b0d0a2020202020202020202020206361203d2028756e7369676e6564206368617229286361202d20274127202b20276127293b0d0a20202020202020207d0d0a2020202020202020696620286362203e3d20274127202626206362203c3d20275a2729207b0d0a2020202020202020202020206362203d2028756e7369676e6564206368617229286362202d20274127202b20276127293b0d0a20202020202020207d0d0a"));
        tampon_yaz(upp_metin_hex_coz("202020202020202069662028636120213d20636229207b0d0a20202020202020202020202072657475726e2028696e74296361202d2028696e742963623b0d0a20202020202020207d0d0a2020202020202020696620286361203d3d203029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a202020207d0d0a2020202072657475726e20303b0d0a7d0d0a0d0a236966205550505f57494e0d0a73746174696320766f6964205f7570705f737464696f5f696b696c6928766f696429207b0d0a20202020696620285f7570705f737464696f5f696b696c695f68617a697229207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020205f7365746d6f6465285f66696c656e6f28737464696e292c205f4f5f42494e415259293b0d0a202020205f7365746d6f6465285f66696c656e6f287374646f7574292c205f4f5f42494e415259293b0d0a202020205f7570705f737464696f5f696b696c695f68617a6972203d20313b0d0a7d0d0a23656c73650d0a73746174696320766f6964205f7570705f737464696f5f696b696c6928766f696429207b0d0a202020202f2a204c696e75783a2043524c4620c3a76576697269736920796f6b3b20696b696c69206d6f6420676572656b6d657a2e202a2f0d0a202020205f7570705f737464696f5f696b696c695f68617a6972203d20313b0d0a7d0d0a23656e6469660d0a0d0a73746174696320636861722a207570705f737464696f5f67656c656e5f6f6b7528766f696429207b0d0a2020202063686172206261736c696b5b383139325d3b0d0a2020202073697a655f7420626e203d20303b0d0a20202020696e7420633b0d0a202020206c6f6e67206c6f6e6720757a756e6c756b203d202d313b0d0a20202020636861722a20676f7664653b0d0a2020202073697a655f74206f6b756e616e203d20303b0d0a202020205f7570705f737464696f5f696b696c6928293b0d0a20202020666f7220283b3b29207b0d0a202020202020202063203d20666765746328737464696e293b0d0a20202020202020206966202863203d3d20454f4629207b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a202020202020202069662028626e202b2031203e3d2073697a656f66286261736c696b2929207b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a20202020202020206261736c696b5b626e2b2b5d203d20286368617229633b0d0a202020202020202069662028626e203e3d2034202626206d656d636d70286261736c696b202b20626e202d20342c20225c725c6e5c725c6e222c203429203d3d203029207b0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("202020202020202020627265616b3b0d0a20202020202020207d0d0a202020202020202069662028626e203e3d2032202626206d656d636d70286261736c696b202b20626e202d20322c20225c6e5c6e222c203229203d3d203029207b0d0a202020202020202020202020627265616b3b0d0a20202020202020207d0d0a202020207d0d0a202020206261736c696b5b626e5d203d20303b0d0a202020207b0d0a2020202020202020636861722a2070203d206261736c696b3b0d0a20202020202020207768696c6520282a7029207b0d0a202020202020202020202020636861722a206e6c3b0d0a202020202020202020202020696620285f7570705f61736369695f6e69636d7028702c2022436f6e74656e742d4c656e6774683a222c20313529203d3d203029207b0d0a2020202020202020202020202020202070202b3d2031353b0d0a202020202020202020202020202020207768696c6520282a70203d3d20272027207c7c202a70203d3d20275c742729207b0d0a2020202020202020202020202020202020202020702b2b3b0d0a202020202020202020202020202020207d0d0a20202020202020202020202020202020757a756e6c756b203d20737472746f6c6c28702c204e554c4c2c203130293b0d0a2020202020202020202020207d0d0a2020202020202020202020206e6c203d2073747273747228702c20225c6e22293b0d0a20202020202020202020202069662028216e6c29207b0d0a20202020202020202020202020202020627265616b3b0d0a2020202020202020202020207d0d0a20202020202020202020202070203d206e6c202b20313b0d0a20202020202020207d0d0a202020207d0d0a2020202069662028757a756e6c756b203c2030207c7c20757a756e6c756b203e2036344c4c202a20313032344c4c202a20313032344c4c29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020676f766465203d2028636861722a296d616c6c6f63282873697a655f7429757a756e6c756b202b2031293b0d0a202020206966202821676f76646529207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020207768696c6520286f6b756e616e203c202873697a655f7429757a756e6c756b29207b0d0a202020202020202073697a655f742072203d20667265616428676f766465202b206f6b756e616e2c20312c202873697a655f7429757a756e6c756b202d206f6b756e616e2c20737464696e293b0d0a20202020202020206966202872203d3d203029207b0d0a2020202020202020202020206672656528676f766465293b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a2020202020"));
        tampon_yaz(upp_metin_hex_coz("2020206f6b756e616e202b3d20723b0d0a202020207d0d0a20202020676f7664655b757a756e6c756b5d203d20303b0d0a2020202072657475726e20676f7664653b0d0a7d0d0a0d0a73746174696320766f6964207570705f737464696f5f79617a28636f6e737420636861722a206d6574696e29207b0d0a2020202073697a655f74206e203d206d6574696e203f207374726c656e286d6574696e29203a20303b0d0a202020205f7570705f737464696f5f696b696c6928293b0d0a20202020667072696e7466287374646f75742c2022436f6e74656e742d4c656e6774683a20257a755c725c6e5c725c6e222c206e293b0d0a20202020696620286e202626206d6574696e29207b0d0a2020202020202020667772697465286d6574696e2c20312c206e2c207374646f7574293b0d0a202020207d0d0a2020202066666c757368287374646f7574293b0d0a7d0d0a0d0a236966205550505f57494e0d0a73746174696320537572656343696b7469207570705f73697374656d5f63616c697374697228636f6e737420636861722a206b6f6d757429207b0d0a2020202053454355524954595f415454524942555445532073613b0d0a2020202048414e444c45207264203d204e554c4c3b0d0a2020202048414e444c45207772203d204e554c4c3b0d0a2020202053544152545550494e464f412073693b0d0a2020202050524f434553535f494e464f524d4154494f4e2070693b0d0a20202020636861722a2073617469723b0d0a20202020636861722a2063696b74693b0d0a2020202073697a655f74206e203d20303b0d0a2020202073697a655f7420636170203d20343039363b0d0a2020202044574f5244206f6b756e616e3b0d0a2020202044574f524420657869745f6b6f64203d20313b0d0a2020202063686172206275665b343039365d3b0d0a20202020537572656343696b746920723b0d0a20202020722e63696b7469203d204e554c4c3b0d0a20202020722e6b6f64203d20313b0d0a20202020722e6261736172696c69203d20303b0d0a2020202069662028216b6f6d7574207c7c20216b6f6d75745b305d29207b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a202020206d656d736574282673612c20302c2073697a656f6628736129293b0d0a2020202073612e6e4c656e677468203d2073697a656f66287361293b0d0a2020202073612e62496e686572697448616e646c65203d20545255453b0d0a20202020696620282143726561746550697065282672642c202677722c202673612c20302929207b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a2020202053657448616e646c65496e666f726d6174696f6e2872642c2048414e444c455f464c41475f494e4845"));
        tampon_yaz(upp_metin_hex_coz("5249542c2030293b0d0a202020206d656d736574282673692c20302c2073697a656f6628736929293b0d0a2020202073692e6362203d2073697a656f66287369293b0d0a2020202073692e6477466c616773203d205354415254465f55534553544448414e444c45533b0d0a2020202073692e685374644f7574707574203d2077723b0d0a2020202073692e685374644572726f72203d2077723b0d0a2020202073692e68537464496e707574203d2047657453746448616e646c65285354445f494e5055545f48414e444c45293b0d0a202020206d656d736574282670692c20302c2073697a656f6628706929293b0d0a202020207361746972203d2028636861722a296d616c6c6f63287374726c656e286b6f6d757429202b203136293b0d0a202020206966202821736174697229207b0d0a2020202020202020436c6f736548616e646c65287264293b0d0a2020202020202020436c6f736548616e646c65287772293b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a20202020736e7072696e74662873617469722c207374726c656e286b6f6d757429202b2031362c2022636d642e657865202f43202573222c206b6f6d7574293b0d0a20202020696620282143726561746550726f6365737341284e554c4c2c2073617469722c204e554c4c2c204e554c4c2c20545255452c204352454154455f4e4f5f57494e444f572c204e554c4c2c204e554c4c2c202673692c202670692929207b0d0a202020202020202066726565287361746972293b0d0a2020202020202020436c6f736548616e646c65287264293b0d0a2020202020202020436c6f736548616e646c65287772293b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a2020202066726565287361746972293b0d0a20202020436c6f736548616e646c65287772293b0d0a2020202063696b7469203d2028636861722a296d616c6c6f6328636170293b0d0a20202020696620282163696b746929207b0d0a2020202020202020436c6f736548616e646c65287264293b0d0a2020202020202020436c6f736548616e646c652870692e6850726f63657373293b0d0a2020202020202020436c6f736548616e646c652870692e68546872656164293b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a2020202063696b74695b305d203d20303b0d0a202020207768696c6520285265616446696c652872642c206275662c2073697a656f6628627566292c20266f6b756e616e2c204e554c4c29202626206f6b756e616e29207b0d0a2020202020202020696620286e202b206f6b756e616e202b2031203e2063617029207b0d0a202020202020202020202020636861722a206e703b0d0a202020202020202020"));
        tampon_yaz(upp_metin_hex_coz("202020636170202a3d20323b0d0a20202020202020202020202069662028636170203c206e202b206f6b756e616e202b203129207b0d0a20202020202020202020202020202020636170203d206e202b206f6b756e616e202b20313b0d0a2020202020202020202020207d0d0a2020202020202020202020206e70203d2028636861722a297265616c6c6f632863696b74692c20636170293b0d0a20202020202020202020202069662028216e7029207b0d0a20202020202020202020202020202020667265652863696b7469293b0d0a20202020202020202020202020202020436c6f736548616e646c65287264293b0d0a20202020202020202020202020202020436c6f736548616e646c652870692e6850726f63657373293b0d0a20202020202020202020202020202020436c6f736548616e646c652870692e68546872656164293b0d0a2020202020202020202020202020202072657475726e20723b0d0a2020202020202020202020207d0d0a20202020202020202020202063696b7469203d206e703b0d0a20202020202020207d0d0a20202020202020206d656d6370792863696b7469202b206e2c206275662c206f6b756e616e293b0d0a20202020202020206e202b3d206f6b756e616e3b0d0a202020202020202063696b74695b6e5d203d20303b0d0a202020207d0d0a2020202057616974466f7253696e676c654f626a6563742870692e6850726f636573732c20494e46494e495445293b0d0a2020202047657445786974436f646550726f636573732870692e6850726f636573732c2026657869745f6b6f64293b0d0a20202020436c6f736548616e646c65287264293b0d0a20202020436c6f736548616e646c652870692e6850726f63657373293b0d0a20202020436c6f736548616e646c652870692e68546872656164293b0d0a20202020722e63696b7469203d2063696b74693b0d0a20202020722e6b6f64203d20286c6f6e67206c6f6e6729657869745f6b6f643b0d0a20202020722e6261736172696c69203d2028657869745f6b6f64203d3d2030293b0d0a2020202072657475726e20723b0d0a7d0d0a23656c73650d0a73746174696320537572656343696b7469207570705f73697374656d5f63616c697374697228636f6e737420636861722a206b6f6d757429207b0d0a2020202046494c452a20663b0d0a20202020636861722a2063696b74693b0d0a2020202073697a655f74206e203d20303b0d0a2020202073697a655f7420636170203d20343039363b0d0a2020202063686172206275665b343039365d3b0d0a2020202073697a655f742072643b0d0a20202020696e742073743b0d0a20202020537572656343696b746920723b0d0a20202020722e63696b7469203d204e554c4c3b0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("20722e6b6f64203d20313b0d0a20202020722e6261736172696c69203d20303b0d0a2020202069662028216b6f6d7574207c7c20216b6f6d75745b305d29207b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a2020202066203d20706f70656e286b6f6d75742c20227222293b0d0a2020202069662028216629207b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a2020202063696b7469203d2028636861722a296d616c6c6f6328636170293b0d0a20202020696620282163696b746929207b0d0a202020202020202070636c6f73652866293b0d0a202020202020202072657475726e20723b0d0a202020207d0d0a2020202063696b74695b305d203d20303b0d0a202020207768696c652028287264203d206672656164286275662c20312c2073697a656f6628627566292c20662929203e203029207b0d0a2020202020202020696620286e202b207264202b2031203e2063617029207b0d0a202020202020202020202020636861722a206e703b0d0a202020202020202020202020636170202a3d20323b0d0a20202020202020202020202069662028636170203c206e202b207264202b203129207b0d0a20202020202020202020202020202020636170203d206e202b207264202b20313b0d0a2020202020202020202020207d0d0a2020202020202020202020206e70203d2028636861722a297265616c6c6f632863696b74692c20636170293b0d0a20202020202020202020202069662028216e7029207b0d0a20202020202020202020202020202020667265652863696b7469293b0d0a2020202020202020202020202020202070636c6f73652866293b0d0a2020202020202020202020202020202072657475726e20723b0d0a2020202020202020202020207d0d0a20202020202020202020202063696b7469203d206e703b0d0a20202020202020207d0d0a20202020202020206d656d6370792863696b7469202b206e2c206275662c207264293b0d0a20202020202020206e202b3d2072643b0d0a202020202020202063696b74695b6e5d203d20303b0d0a202020207d0d0a202020207374203d2070636c6f73652866293b0d0a20202020722e63696b7469203d2063696b74693b0d0a20202020696620287374203c203029207b0d0a2020202020202020722e6b6f64203d20313b0d0a202020207d20656c7365207b0d0a236966646566205749464558495445440d0a2020202020202020722e6b6f64203d2057494645584954454428737429203f20574558495453544154555328737429203a20313b0d0a23656c73650d0a2020202020202020722e6b6f64203d20286c6f6e67206c6f6e672973743b0d0a23656e6469660d0a202020207d0d0a20202020722e6261736172"));
        tampon_yaz(upp_metin_hex_coz("696c69203d2028722e6b6f64203d3d2030293b0d0a2020202072657475726e20723b0d0a7d0d0a23656e6469660d0a0d0a73746174696320636861722a207570705f73697374656d5f6b6f6d75745f62756c28636f6e737420636861722a20616429207b0d0a2020202069662028216164207c7c202161645b305d29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a236966205550505f57494e0d0a202020207b0d0a202020202020202063686172206275665b4d41585f504154485d3b0d0a202020202020202044574f5244206e203d205365617263685061746841284e554c4c2c2061642c20222e657865222c204d41585f504154482c206275662c204e554c4c293b0d0a2020202020202020696620286e203d3d2030207c7c206e203e3d204d41585f5041544829207b0d0a2020202020202020202020206e203d205365617263685061746841284e554c4c2c2061642c204e554c4c2c204d41585f504154482c206275662c204e554c4c293b0d0a20202020202020207d0d0a2020202020202020696620286e203d3d2030207c7c206e203e3d204d41585f5041544829207b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a202020207d0d0a23656c73650d0a202020207b0d0a2020202020202020636f6e737420636861722a20706174683b0d0a2020202020202020636861722a206b6f7079613b0d0a2020202020202020636861722a20746f6b3b0d0a2020202020202020636861722064656e656d655b343039365d3b0d0a2020202020202020696620287374726368722861642c20272f272929207b0d0a202020202020202020202020696620286163636573732861642c20585f4f4b29203d3d203029207b0d0a2020202020202020202020202020202072657475726e207570705f6d6574696e5f6b6f707961286164293b0d0a2020202020202020202020207d0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a202020202020202070617468203d20676574656e7628225041544822293b0d0a202020202020202069662028217061746829207b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a20202020202020206b6f707961203d207570705f6d6574696e5f6b6f7079612870617468293b0d0a202020202020202069662028216b6f70796129207b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a2020202020202020746f6b203d20737472746f6b286b6f7079612c20223a22293b0d"));
        tampon_yaz(upp_metin_hex_coz("0a20202020202020207768696c652028746f6b29207b0d0a202020202020202020202020736e7072696e74662864656e656d652c2073697a656f662864656e656d65292c202225732f2573222c20746f6b2c206164293b0d0a202020202020202020202020696620286163636573732864656e656d652c20585f4f4b29203d3d203029207b0d0a20202020202020202020202020202020636861722a206f7574203d207570705f6d6574696e5f6b6f7079612864656e656d65293b0d0a2020202020202020202020202020202066726565286b6f707961293b0d0a2020202020202020202020202020202072657475726e206f75743b0d0a2020202020202020202020207d0d0a202020202020202020202020746f6b203d20737472746f6b284e554c4c2c20223a22293b0d0a20202020202020207d0d0a202020202020202066726565286b6f707961293b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a23656e6469660d0a7d0d0a0d0a73746174696320636861722a207570705f73697374656d5f6578655f64697a696e28766f696429207b0d0a236966205550505f57494e0d0a2020202063686172206275665b4d41585f504154485d3b0d0a20202020636861722a20736c6173683b0d0a2020202069662028214765744d6f64756c6546696c654e616d6541284e554c4c2c206275662c204d41585f504154482929207b0d0a2020202020202020696620282147657443757272656e744469726563746f727941284d41585f504154482c206275662929207b0d0a20202020202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a20202020202020207d0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a202020207d0d0a20202020736c617368203d2073747272636872286275662c20275c5c27293b0d0a202020206966202821736c61736829207b0d0a2020202020202020736c617368203d2073747272636872286275662c20272f27293b0d0a202020207d0d0a2020202069662028736c61736829207b0d0a20202020202020202a736c617368203d20303b0d0a202020207d0d0a2020202072657475726e207570705f6d6574696e5f6b6f707961286275665b305d203f20627566203a20222e22293b0d0a23656c73650d0a2020202063686172206275665b343039365d3b0d0a23696620646566696e6564285f57494e3332290d0a202020202f2a204d696e4757202d2d6c696e75783a20726561646c696e6b20796f6b3b20c3a7616cc4b1c59f6d612064697a696e692079657465722028676572c3a7656b204c696e75782067636320504f5349582064616cc4b1292e202a2f0d0a202020206966202821"));
        tampon_yaz(upp_metin_hex_coz("676574637764286275662c2073697a656f6628627566292929207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a202020207d0d0a2020202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a23656c73650d0a202020207b0d0a20202020202020207373697a655f74206e203d20726561646c696e6b28222f70726f632f73656c662f657865222c206275662c2073697a656f662862756629202d2031293b0d0a2020202020202020636861722a20736c6173683b0d0a2020202020202020696620286e203c3d203029207b0d0a2020202020202020202020206966202821676574637764286275662c2073697a656f6628627566292929207b0d0a2020202020202020202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a2020202020202020202020207d0d0a20202020202020202020202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a20202020202020207d0d0a20202020202020206275665b6e5d203d20303b0d0a2020202020202020736c617368203d2073747272636872286275662c20272f27293b0d0a202020202020202069662028736c61736829207b0d0a2020202020202020202020202a736c617368203d20303b0d0a20202020202020207d0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f707961286275665b305d203f20627566203a20222e22293b0d0a202020207d0d0a23656e6469660d0a23656e6469660d0a7d0d0a0d0a73746174696320636f6e737420636861722a207570705f706c6174666f726d28766f696429207b0d0a236966205550505f57494e0d0a2020202072657475726e202277696e646f7773223b0d0a23656c73650d0a2020202072657475726e20226c696e7578223b0d0a23656e6469660d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f73697374656d5f70696428766f696429207b0d0a236966205550505f57494e0d0a2020202072657475726e20286c6f6e67206c6f6e672947657443757272656e7450726f63657373496428293b0d0a23656c73650d0a2020202072657475726e20286c6f6e67206c6f6e672967657470696428293b0d0a23656e6469660d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f6f7274616d5f79617a28636f6e737420636861722a2061642c20636f6e737420636861722a20646567657229207b0d0a2020202069662028216164207c7c202161645b305d292072657475726e20303b0d0a236966205550505f57494e0d0a2020202072657475726e205f707574656e765f732861642c206465676572203f206465676572203a202222"));
        tampon_yaz(upp_metin_hex_coz("29203d3d2030203f2031203a20303b0d0a23656c73650d0a2020202069662028216465676572292072657475726e20756e736574656e7628616429203d3d2030203f2031203a20303b0d0a2020202072657475726e20736574656e762861642c2064656765722c203129203d3d2030203f2031203a20303b0d0a23656e6469660d0a7d0d0a0d0a737461746963206c6f6e67206c6f6e67207570705f646f7379615f626f79757428636f6e737420636861722a20796f6c29207b0d0a202020206966202821796f6c292072657475726e202d313b0d0a236966205550505f57494e0d0a2020202057494e33325f46494c455f4154545249425554455f44415441206661643b0d0a202020204c415247455f494e54454745522073697a653b0d0a20202020696620282147657446696c654174747269627574657345784128796f6c2c2047657446696c654578496e666f5374616e646172642c202666616429292072657475726e202d313b0d0a2020202073697a652e4c6f7750617274203d206661642e6e46696c6553697a654c6f773b0d0a2020202073697a652e4869676850617274203d206661642e6e46696c6553697a65486967683b0d0a2020202072657475726e20286c6f6e67206c6f6e672973697a652e51756164506172743b0d0a23656c73650d0a2020202073747275637420737461742073743b0d0a20202020696620287374617428796f6c2c202673742920213d2030292072657475726e202d313b0d0a2020202072657475726e20286c6f6e67206c6f6e672973742e73745f73697a653b0d0a23656e6469660d0a7d0d0a0d0a2f2a204c696e75782d7370656369666963204150497320287570702e6c696e75782e2a29202a2f0d0a23696620215550505f57494e2026262021646566696e6564285f57494e3332290d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f70696428766f696429207b0d0a2020202072657475726e20286c6f6e67206c6f6e672967657470696428293b0d0a7d0d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f73696e79616c5f676f6e646572286c6f6e67206c6f6e67207069642c206c6f6e67206c6f6e672073696729207b0d0a2020202072657475726e206b696c6c28287069645f74297069642c2028696e742973696729203d3d2030203f2031203a20303b0d0a7d0d0a73746174696320636861722a207570705f6c696e75785f70726f635f6f6b75286c6f6e67206c6f6e67207069642c20636f6e737420636861722a20646f73796129207b0d0a202020206368617220706174685b3235365d3b0d0a2020202046494c452a20663b0d0a20202020636861722a206275663b0d0a2020202073697a655f74206e203d20302c20636170203d2034"));
        tampon_yaz(upp_metin_hex_coz("3039363b0d0a202020206368617220746d705b313032345d3b0d0a2020202073697a655f742072643b0d0a202020206966202821646f737961292072657475726e204e554c4c3b0d0a20202020736e7072696e746628706174682c2073697a656f662870617468292c20222f70726f632f256c6c642f2573222c207069642c20646f737961293b0d0a2020202066203d20666f70656e28706174682c20227222293b0d0a20202020696620282166292072657475726e204e554c4c3b0d0a20202020627566203d2028636861722a296d616c6c6f6328636170293b0d0a20202020696620282162756629207b2066636c6f73652866293b2072657475726e204e554c4c3b207d0d0a202020207768696c652028287264203d20667265616428746d702c20312c2073697a656f6628746d70292c20662929203e203029207b0d0a2020202020202020696620286e202b207264202b2031203e2063617029207b0d0a202020202020202020202020636170203d2028636170202a2032203e206e202b207264202b203129203f20636170202a2032203a206e202b207264202b20313b0d0a202020202020202020202020636861722a206e62203d2028636861722a297265616c6c6f63286275662c20636170293b0d0a20202020202020202020202069662028216e6229207b206672656528627566293b2066636c6f73652866293b2072657475726e204e554c4c3b207d0d0a202020202020202020202020627566203d206e623b0d0a20202020202020207d0d0a20202020202020206d656d63707928627566202b206e2c20746d702c207264293b0d0a20202020202020206e202b3d2072643b0d0a202020207d0d0a202020206275665b6e5d203d20303b0d0a2020202066636c6f73652866293b0d0a2020202072657475726e206275663b0d0a7d0d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f736179695f6f6b75286c6f6e67206c6f6e67207069642c206c6f6e67206c6f6e6720616472657329207b0d0a23696620646566696e6564285f5f6c696e75785f5f2920262620646566696e6564285f474e555f534f55524345290d0a202020206c6f6e67206c6f6e672076616c203d20303b0d0a2020202073747275637420696f766563206c6f63616c5b315d3b0d0a2020202073747275637420696f7665632072656d6f74655b315d3b0d0a202020206c6f63616c5b305d2e696f765f62617365203d202676616c3b0d0a202020206c6f63616c5b305d2e696f765f6c656e203d2073697a656f662876616c293b0d0a2020202072656d6f74655b305d2e696f765f62617365203d2028766f69642a292875696e747074725f742961647265733b0d0a2020202072656d6f74655b305d2e696f765f6c656e203d2073697a65"));
        tampon_yaz(upp_metin_hex_coz("6f662876616c293b0d0a202020206966202870726f636573735f766d5f726561647628287069645f74297069642c206c6f63616c2c20312c2072656d6f74652c20312c203029203d3d2073697a656f662876616c29292072657475726e2076616c3b0d0a2020202072657475726e20303b0d0a23656c73650d0a2020202028766f6964297069643b2028766f69642961647265733b0d0a2020202072657475726e20303b0d0a23656e6469660d0a7d0d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f736179695f79617a286c6f6e67206c6f6e67207069642c206c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720646567657229207b0d0a23696620646566696e6564285f5f6c696e75785f5f2920262620646566696e6564285f474e555f534f55524345290d0a2020202073747275637420696f766563206c6f63616c5b315d3b0d0a2020202073747275637420696f7665632072656d6f74655b315d3b0d0a202020206c6f63616c5b305d2e696f765f62617365203d202664656765723b0d0a202020206c6f63616c5b305d2e696f765f6c656e203d2073697a656f66286465676572293b0d0a2020202072656d6f74655b305d2e696f765f62617365203d2028766f69642a292875696e747074725f742961647265733b0d0a2020202072656d6f74655b305d2e696f765f6c656e203d2073697a656f66286465676572293b0d0a2020202072657475726e2070726f636573735f766d5f77726974657628287069645f74297069642c206c6f63616c2c20312c2072656d6f74652c20312c203029203d3d2073697a656f6628646567657229203f2031203a20303b0d0a23656c73650d0a2020202028766f6964297069643b2028766f69642961647265733b2028766f69642964656765723b0d0a2020202072657475726e20303b0d0a23656e6469660d0a7d0d0a23656c73650d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f70696428766f696429207b2072657475726e20303b207d0d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f73696e79616c5f676f6e646572286c6f6e67206c6f6e67207069642c206c6f6e67206c6f6e672073696729207b2028766f6964297069643b2028766f6964297369673b2072657475726e20303b207d0d0a73746174696320636861722a207570705f6c696e75785f70726f635f6f6b75286c6f6e67206c6f6e67207069642c20636f6e737420636861722a20646f73796129207b2028766f6964297069643b2028766f696429646f7379613b2072657475726e204e554c4c3b207d0d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f736179695f6f6b75286c6f6e67206c6f6e6720706964"));
        tampon_yaz(upp_metin_hex_coz("2c206c6f6e67206c6f6e6720616472657329207b2028766f6964297069643b2028766f69642961647265733b2072657475726e20303b207d0d0a737461746963206c6f6e67206c6f6e67207570705f6c696e75785f736179695f79617a286c6f6e67206c6f6e67207069642c206c6f6e67206c6f6e672061647265732c206c6f6e67206c6f6e6720646567657229207b2028766f6964297069643b2028766f69642961647265733b2028766f69642964656765723b2072657475726e20303b207d0d0a23656e6469660d0a0d0a2f2a2057696e646f77732d737065636966696320616c696173657320287570702e77696e646f77732e2a29202a2f0d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f6261676c616e207570705f62656c6c656b5f6261676c616e0d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f6d6f64756c5f62756c207570705f62656c6c656b5f6d6f64756c5f62756c0d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f736179696f6b75207570705f62656c6c656b5f736179696f6b750d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f7361796979617a207570705f62656c6c656b5f7361796979617a0d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f6f6b755f6f6e64616c696b207570705f62656c6c656b5f6f6b755f6f6e64616c696b0d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f79617a5f6f6e64616c696b207570705f62656c6c656b5f79617a5f6f6e64616c696b0d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f6f6b755f6d6574696e207570705f62656c6c656b5f6f6b755f6d6574696e0d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f7a696e6369725f6f6b75207570705f62656c6c656b5f7a696e6369725f6f6b750d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f79616d615f796170207570705f62656c6c656b5f79616d615f7961700d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f6b6f72756d615f6465676973746972207570705f62656c6c656b5f6b6f72756d615f64656769737469720d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f6b6f72756d615f64656769737469725f73617969207570705f62656c6c656b5f6b6f72756d615f64656769737469725f736179690d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f736f6e5f68617461207570705f62656c6c656b5f736f6e5f686174610d0a23646566696e65207570705f77696e646f77735f62656c6c656b5f6f6b756e647520757070"));
        tampon_yaz(upp_metin_hex_coz("5f62656c6c656b5f6f6b756e64750d0a0d0a23646566696e65207570705f77696e646f77735f67697264695f666172655f74617369207570705f67697264695f666172655f746173690d0a23646566696e65207570705f77696e646f77735f67697264695f666172655f74696b6c61207570705f67697264695f666172655f74696b6c610d0a23646566696e65207570705f77696e646f77735f67697264695f7475735f626173207570705f67697264695f7475735f6261730d0a0d0a23646566696e65207570705f77696e646f77735f63697a696d5f6261736c6174207570705f63697a696d5f6261736c61740d0a23646566696e65207570705f77696e646f77735f63697a696d5f74656d697a6c65207570705f63697a696d5f74656d697a6c650d0a23646566696e65207570705f77696e646f77735f63697a696d5f6b757475207570705f63697a696d5f6b7574750d0a23646566696e65207570705f77696e646f77735f63697a696d5f79617a69207570705f63697a696d5f79617a690d0a0d0a23646566696e65207570705f77696e646f77735f6d6573616a207570705f6d6573616a0d0a23646566696e65207570705f77696e646f77735f68617461207570705f686174610d0a23646566696e65207570705f77696e646f77735f7365735f63616c207570705f7365735f63616c0d0a0d0a73746174696320696e74205f7570705f796f6c5f7365702863686172206329207b0d0a2020202072657475726e2063203d3d20272f27207c7c2063203d3d20275c5c273b0d0a7d0d0a0d0a7374617469632063686172205f7570705f796f6c5f61796972616328766f696429207b0d0a236966205550505f57494e0d0a2020202072657475726e20275c5c273b0d0a23656c73650d0a2020202072657475726e20272f273b0d0a23656e6469660d0a7d0d0a0d0a73746174696320636861722a207570705f796f6c5f64697a696e28636f6e737420636861722a20796f6c29207b0d0a20202020636861722a206b3b0d0a20202020636861722a20736c6173683b0d0a2020202073697a655f74206e3b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a202020207d0d0a202020206b203d207570705f6d6574696e5f6b6f70796128796f6c293b0d0a2020202069662028216b29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a202020206e203d207374726c656e286b293b0d0a202020207768696c6520286e203e2031202626205f7570705f796f6c5f736570286b5b6e202d20315d2929207b0d0a20202020202020206b5b2d2d6e5d203d20303b0d0a202020207d0d0a202020"));
        tampon_yaz(upp_metin_hex_coz("20736c617368203d2073747272636872286b2c20275c5c27293b0d0a202020206966202821736c61736829207b0d0a2020202020202020736c617368203d2073747272636872286b2c20272f27293b0d0a202020207d0d0a202020206966202821736c61736829207b0d0a202020202020202066726565286b293b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a202020207d0d0a2020202069662028736c617368203d3d206b29207b0d0a2020202020202020736c6173685b315d203d20303b0d0a202020202020202072657475726e206b3b0d0a202020207d0d0a202020202a736c617368203d20303b0d0a2020202069662028216b5b305d29207b0d0a202020202020202066726565286b293b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a202020207d0d0a2020202072657475726e206b3b0d0a7d0d0a0d0a73746174696320636861722a207570705f796f6c5f6269726c657374697228636f6e737420636861722a20612c20636f6e737420636861722a206229207b0d0a2020202073697a655f74206e612c206e623b0d0a20202020636861722a20703b0d0a2020202063686172207365703b0d0a2020202069662028216129207b0d0a202020202020202061203d2022223b0d0a202020207d0d0a2020202069662028216229207b0d0a202020202020202062203d2022223b0d0a202020207d0d0a202020206966202821615b305d29207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f7079612862293b0d0a202020207d0d0a202020206966202821625b305d29207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f7079612861293b0d0a202020207d0d0a20202020696620285f7570705f796f6c5f73657028625b305d290d0a236966205550505f57494e0d0a20202020202020207c7c20282828625b305d203e3d2027412720262620625b305d203c3d20275a2729207c7c2028625b305d203e3d2027612720262620625b305d203c3d20277a27292920262620625b315d203d3d20273a27290d0a23656e6469660d0a2020202029207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f7079612862293b0d0a202020207d0d0a202020206e61203d207374726c656e2861293b0d0a202020206e62203d207374726c656e2862293b0d0a20202020736570203d205f7570705f796f6c5f61796972616328293b0d0a2020202070203d2028636861722a296d616c6c6f63286e61202b206e62202b203375293b0d0a2020202069662028217029207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020"));
        tampon_yaz(upp_metin_hex_coz("6d656d63707928702c20612c206e61293b0d0a2020202069662028215f7570705f796f6c5f73657028615b6e61202d20315d2929207b0d0a2020202020202020705b6e612b2b5d203d207365703b0d0a202020207d0d0a202020206d656d6370792870202b206e612c20622c206e62202b203175293b0d0a2020202072657475726e20703b0d0a7d0d0a0d0a73746174696320636861722a207570705f796f6c5f67657263656b28636f6e737420636861722a20796f6c29207b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f707961282222293b0d0a202020207d0d0a236966205550505f57494e0d0a202020207b0d0a202020202020202063686172206275665b4d41585f50415448202a20325d3b0d0a202020202020202044574f5244206e203d2047657446756c6c506174684e616d654128796f6c2c202844574f52442973697a656f6628627566292c206275662c204e554c4c293b0d0a2020202020202020696620286e203d3d2030207c7c206e203e3d2073697a656f66286275662929207b0d0a20202020202020202020202072657475726e207570705f6d6574696e5f6b6f70796128796f6c293b0d0a20202020202020207d0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a202020207d0d0a23656c73650d0a202020207b0d0a202020202020202063686172206275665b343039365d3b0d0a23696620646566696e6564285f57494e3332290d0a202020202020202069662028215f66756c6c70617468286275662c20796f6c2c2073697a656f6628627566292929207b0d0a20202020202020202020202072657475726e207570705f6d6574696e5f6b6f70796128796f6c293b0d0a20202020202020207d0d0a23656c73650d0a202020202020202069662028217265616c7061746828796f6c2c206275662929207b0d0a20202020202020202020202072657475726e207570705f6d6574696e5f6b6f70796128796f6c293b0d0a20202020202020207d0d0a23656e6469660d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a202020207d0d0a23656e6469660d0a7d0d0a0d0a73746174696320636861722a207570705f796f6c5f63616c69736d6128766f696429207b0d0a236966205550505f57494e0d0a2020202063686172206275665b4d41585f504154485d3b0d0a20202020696620282147657443757272656e744469726563746f727941284d41585f504154482c206275662929207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a202020207d0d0a2020"));
        tampon_yaz(upp_metin_hex_coz("202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a23656c73650d0a2020202063686172206275665b504154485f4d41585d3b0d0a202020206966202821676574637764286275662c2073697a656f6628627566292929207b0d0a202020202020202072657475726e207570705f6d6574696e5f6b6f70796128222e22293b0d0a202020207d0d0a2020202072657475726e207570705f6d6574696e5f6b6f70796128627566293b0d0a23656e6469660d0a7d0d0a0d0a73746174696320696e74207570705f796f6c5f6d75746c616b5f6d6928636f6e737420636861722a20796f6c29207b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a2020202069662028796f6c5b305d203d3d20272f27207c7c20796f6c5b305d203d3d20275c5c2729207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a236966205550505f57494e0d0a20202020696620282828796f6c5b305d203e3d2027412720262620796f6c5b305d203c3d20275a2729207c7c2028796f6c5b305d203e3d2027612720262620796f6c5b305d203c3d20277a27292920262620796f6c5b315d203d3d20273a2729207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a23656e6469660d0a2020202072657475726e20303b0d0a7d0d0a0d0a73746174696320696e74207570705f796f6c5f6b6c61736f725f6f6c757374757228636f6e737420636861722a20796f6c29207b0d0a20202020636861722a206275663b0d0a20202020636861722a20703b0d0a2020202073697a655f74206e3b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a202020202020202072657475726e20313b0d0a202020207d0d0a202020206e203d207374726c656e28796f6c293b0d0a20202020627566203d2028636861722a296d616c6c6f63286e202b203175293b0d0a20202020696620282162756629207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a202020206d656d637079286275662c20796f6c2c206e202b203175293b0d0a236966205550505f57494e0d0a20202020666f72202870203d206275663b202a703b20702b2b29207b0d0a2020202020202020696620282a70203d3d20272f2729207b0d0a2020202020202020202020202a70203d20275c5c273b0d0a20202020202020207d0d0a202020207d0d0a2020202070203d206275663b0d0a2020202069662028705b305d20262620705b315d203d3d20273a2729207b0d0a202020202020202070202b3d20323b0d0a2020202020202020696620282a70203d3d20275c5c2729207b0d0a202020202020"));
        tampon_yaz(upp_metin_hex_coz("202020202020702b2b3b0d0a20202020202020207d0d0a202020207d0d0a20202020666f7220283b202a703b20702b2b29207b0d0a2020202020202020696620282a70203d3d20275c5c2729207b0d0a2020202020202020202020202a70203d20303b0d0a202020202020202020202020696620286275665b305d29207b0d0a202020202020202020202020202020204372656174654469726563746f727941286275662c204e554c4c293b0d0a2020202020202020202020207d0d0a2020202020202020202020202a70203d20275c5c273b0d0a20202020202020207d0d0a202020207d0d0a20202020696620286275665b305d29207b0d0a20202020202020204372656174654469726563746f727941286275662c204e554c4c293b0d0a202020207d0d0a23656c73650d0a2020202070203d206275663b0d0a20202020696620282a70203d3d20272f2729207b0d0a2020202020202020702b2b3b0d0a202020207d0d0a20202020666f7220283b202a703b20702b2b29207b0d0a2020202020202020696620282a70203d3d20272f2729207b0d0a2020202020202020202020202a70203d20303b0d0a202020202020202020202020696620286275665b305d29207b0d0a23696620646566696e6564285f57494e3332290d0a202020202020202020202020202020206d6b64697228627566293b0d0a23656c73650d0a202020202020202020202020202020206d6b646972286275662c2030373535293b0d0a23656e6469660d0a2020202020202020202020207d0d0a2020202020202020202020202a70203d20272f273b0d0a20202020202020207d0d0a202020207d0d0a20202020696620286275665b305d29207b0d0a23696620646566696e6564285f57494e3332290d0a20202020202020206d6b64697228627566293b0d0a23656c73650d0a20202020202020206d6b646972286275662c2030373535293b0d0a23656e6469660d0a202020207d0d0a23656e6469660d0a202020206672656528627566293b0d0a2020202072657475726e20313b0d0a7d0d0a0d0a737461746963205570704b6f6c4c69737465207570705f646f7379615f6c697374656c6528636f6e737420636861722a20796f6c29207b0d0a202020205570704b6f6c4c69737465204c3b0d0a202020204c2e6964203d20303b0d0a202020206966202821796f6c207c7c2021796f6c5b305d29207b0d0a2020202020202020796f6c203d20222e223b0d0a202020207d0d0a236966205550505f57494e0d0a202020207b0d0a20202020202020206368617220646573656e5b4d41585f50415448202a20325d3b0d0a202020202020202057494e33325f46494e445f44415441412066643b0d0a202020202020202048414e444c4520683b0d0a20202020202020"));
        tampon_yaz(upp_metin_hex_coz("2073697a655f74206e203d207374726c656e28796f6c293b0d0a2020202020202020696620286e202b2033203e3d2073697a656f6628646573656e2929207b0d0a20202020202020202020202072657475726e204c3b0d0a20202020202020207d0d0a20202020202020206d656d63707928646573656e2c20796f6c2c206e202b203175293b0d0a2020202020202020696620286e203e203020262620646573656e5b6e202d20315d20213d20275c5c2720262620646573656e5b6e202d20315d20213d20272f2729207b0d0a202020202020202020202020646573656e5b6e2b2b5d203d20275c5c273b0d0a202020202020202020202020646573656e5b6e5d203d20303b0d0a20202020202020207d0d0a20202020202020206d656d63707928646573656e202b206e2c20222a222c203275293b0d0a202020202020202068203d2046696e64466972737446696c654128646573656e2c20266664293b0d0a20202020202020206966202868203d3d20494e56414c49445f48414e444c455f56414c554529207b0d0a20202020202020202020202072657475726e204c3b0d0a20202020202020207d0d0a2020202020202020646f207b0d0a20202020202020202020202069662028737472636d702866642e6346696c654e616d652c20222e2229203d3d2030207c7c20737472636d702866642e6346696c654e616d652c20222e2e2229203d3d203029207b0d0a20202020202020202020202020202020636f6e74696e75653b0d0a2020202020202020202020207d0d0a2020202020202020202020207570705f6c697374655f656b6c6528264c2c205550505f4b545f4d4554494e2c20302c20302e302c2066642e6346696c654e616d65293b0d0a20202020202020207d207768696c65202846696e644e65787446696c654128682c2026666429293b0d0a202020202020202046696e64436c6f73652868293b0d0a202020207d0d0a23656c73650d0a202020207b0d0a20202020202020204449522a2064203d206f70656e64697228796f6c293b0d0a202020202020202073747275637420646972656e742a20653b0d0a202020202020202069662028216429207b0d0a20202020202020202020202072657475726e204c3b0d0a20202020202020207d0d0a20202020202020207768696c6520282865203d20726561646469722864292920213d204e554c4c29207b0d0a20202020202020202020202069662028737472636d7028652d3e645f6e616d652c20222e2229203d3d2030207c7c20737472636d7028652d3e645f6e616d652c20222e2e2229203d3d203029207b0d0a20202020202020202020202020202020636f6e74696e75653b0d0a2020202020202020202020207d0d0a2020202020202020202020207570705f6c697374"));
        tampon_yaz(upp_metin_hex_coz("655f656b6c6528264c2c205550505f4b545f4d4554494e2c20302c20302e302c20652d3e645f6e616d65293b0d0a20202020202020207d0d0a2020202020202020636c6f73656469722864293b0d0a202020207d0d0a23656e6469660d0a2020202072657475726e204c3b0d0a7d0d0a0d0a23646566696e65205550505f4b494c49545f4d415820313030303030304c4c0d0a0d0a7479706564656620737472756374207b0d0a20202020696e742063616e6c693b0d0a20202020696e7420696e69743b0d0a236966205550505f57494e0d0a20202020435249544943414c5f53454354494f4e2063733b0d0a23656c73650d0a20202020707468726561645f6d757465785f74206d753b0d0a23656e6469660d0a7d205f5570704b696c6974483b0d0a0d0a737461746963205f5570704b696c6974482a205f7570705f6b696c69746c6572203d204e554c4c3b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6b696c69745f6e203d20313b0d0a737461746963206c6f6e67206c6f6e67205f7570705f6b696c69745f636170203d20303b0d0a0d0a737461746963206c6f6e67206c6f6e67205f7570705f6b696c69745f79656e6928766f696429207b0d0a202020206c6f6e67206c6f6e6720693b0d0a202020206c6f6e67206c6f6e67206e633b0d0a202020205f5570704b696c6974482a206e703b0d0a20202020666f72202869203d20313b2069203c205f7570705f6b696c69745f6e3b20692b2b29207b0d0a202020202020202069662028215f7570705f6b696c69746c65725b695d2e63616e6c6929207b0d0a2020202020202020202020206d656d73657428265f7570705f6b696c69746c65725b695d2c20302c2073697a656f66285f5570704b696c69744829293b0d0a2020202020202020202020205f7570705f6b696c69746c65725b695d2e63616e6c69203d20313b0d0a20202020202020202020202072657475726e20693b0d0a20202020202020207d0d0a202020207d0d0a20202020696620285f7570705f6b696c69745f6e203e3d205550505f4b494c49545f4d415829207b0d0a202020202020202072657475726e20303b0d0a202020207d0d0a20202020696620285f7570705f6b696c69745f6e202b2031203e205f7570705f6b696c69745f63617029207b0d0a20202020202020206e63203d205f7570705f6b696c69745f636170203f205f7570705f6b696c69745f636170202a2032203a20383b0d0a2020202020202020696620286e63203c205f7570705f6b696c69745f6e202b203129207b0d0a2020202020202020202020206e63203d205f7570705f6b696c69745f6e202b20313b0d0a20202020202020207d0d0a20202020202020206e70203d20285f5570704b696c6974482a297265616c6c"));
        tampon_yaz(upp_metin_hex_coz("6f63285f7570705f6b696c69746c65722c202873697a655f74296e63202a2073697a656f66285f5570704b696c69744829293b0d0a202020202020202069662028216e7029207b0d0a20202020202020202020202072657475726e20303b0d0a20202020202020207d0d0a20202020202020206d656d736574286e70202b205f7570705f6b696c69745f6361702c20302c202873697a655f7429286e63202d205f7570705f6b696c69745f63617029202a2073697a656f66285f5570704b696c69744829293b0d0a20202020202020205f7570705f6b696c69746c6572203d206e703b0d0a20202020202020205f7570705f6b696c69745f636170203d206e633b0d0a202020207d0d0a2020202069203d205f7570705f6b696c69745f6e2b2b3b0d0a202020206d656d73657428265f7570705f6b696c69746c65725b695d2c20302c2073697a656f66285f5570704b696c69744829293b0d0a202020205f7570705f6b696c69746c65725b695d2e63616e6c69203d20313b0d0a2020202072657475726e20693b0d0a7d0d0a0d0a737461746963205f5570704b696c6974482a205f7570705f6b696c69745f6875637265284b696c69742a206b29207b0d0a2020202069662028216b29207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a20202020696620286b2d3e6964203d3d203029207b0d0a20202020202020206b2d3e6964203d205f7570705f6b696c69745f79656e6928293b0d0a2020202020202020696620286b2d3e6964203d3d203029207b0d0a20202020202020202020202072657475726e204e554c4c3b0d0a20202020202020207d0d0a202020207d0d0a20202020696620286b2d3e6964203c3d2030207c7c206b2d3e6964203e3d205f7570705f6b696c69745f6e207c7c20215f7570705f6b696c69746c657229207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202069662028215f7570705f6b696c69746c65725b6b2d3e69645d2e63616e6c6929207b0d0a202020202020202072657475726e204e554c4c3b0d0a202020207d0d0a2020202072657475726e20265f7570705f6b696c69746c65725b6b2d3e69645d3b0d0a7d0d0a0d0a73746174696320766f6964205f7570705f6b696c69745f6d757465785f6b7572285f5570704b696c6974482a206829207b0d0a20202020696620282168207c7c20682d3e696e697429207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a236966205550505f57494e0d0a20202020496e697469616c697a65437269746963616c53656374696f6e2826682d3e6373293b0d0a23656c73650d0a20202020707468726561645f6d757465785f696e69742826682d3e6d752c204e554c4c293b"));
        tampon_yaz(upp_metin_hex_coz("0d0a23656e6469660d0a20202020682d3e696e6974203d20313b0d0a7d0d0a0d0a73746174696320766f6964207570705f6b696c69745f68617a6972284b696c69742a206b29207b0d0a202020205f7570705f6b696c69745f6d757465785f6b7572285f7570705f6b696c69745f6875637265286b29293b0d0a7d0d0a0d0a73746174696320766f6964207570705f6b696c69745f6b696c69746c65284b696c69742a206b29207b0d0a202020205f5570704b696c6974482a2068203d205f7570705f6b696c69745f6875637265286b293b0d0a2020202069662028216829207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a202020205f7570705f6b696c69745f6d757465785f6b75722868293b0d0a236966205550505f57494e0d0a20202020456e746572437269746963616c53656374696f6e2826682d3e6373293b0d0a23656c73650d0a20202020707468726561645f6d757465785f6c6f636b2826682d3e6d75293b0d0a23656e6469660d0a7d0d0a0d0a73746174696320766f6964207570705f6b696c69745f626972616b284b696c69742a206b29207b0d0a202020205f5570704b696c6974482a20683b0d0a2020202069662028216b207c7c206b2d3e6964203d3d203029207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a20202020696620286b2d3e6964203c3d2030207c7c206b2d3e6964203e3d205f7570705f6b696c69745f6e207c7c20215f7570705f6b696c69746c657229207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a2020202068203d20265f7570705f6b696c69746c65725b6b2d3e69645d3b0d0a202020206966202821682d3e63616e6c69207c7c2021682d3e696e697429207b0d0a202020202020202072657475726e3b0d0a202020207d0d0a236966205550505f57494e0d0a202020204c65617665437269746963616c53656374696f6e2826682d3e6373293b0d0a23656c73650d0a20202020707468726561645f6d757465785f756e6c6f636b2826682d3e6d75293b0d0a23656e6469660d0a7d0d0a"));
    }
}

bool arg_esit(char* a, char* b) {
    {
        return metin_esit(a, b);
    }
}

long long harf_kucuk(long long c) {
    {
        if (((c >= 65LL) && (c <= 90LL)))
        {
            return (c + 32LL);
        }
        return c;
    }
}

bool kutup_uzanti_mi(char* s) {
    {
        long long n = metin_uzun(s);
        if ((n < 4LL))
        {
            return false;
        }
        if ((metin_cp(s, (n - 4LL)) != 46LL))
        {
            return false;
        }
        if ((harf_kucuk(metin_cp(s, (n - 3LL))) != 117LL))
        {
            return false;
        }
        if ((harf_kucuk(metin_cp(s, (n - 2LL))) != 112LL))
        {
            return false;
        }
        long long son = harf_kucuk(metin_cp(s, (n - 1LL)));
        return ((son == 112LL) || (son == 104LL));
    }
}

bool baslik_mi(char* s) {
    {
        long long n = metin_uzun(s);
        if ((n < 4LL))
        {
            return false;
        }
        if ((metin_cp(s, (n - 4LL)) != 46LL))
        {
            return false;
        }
        if ((harf_kucuk(metin_cp(s, (n - 3LL))) != 117LL))
        {
            return false;
        }
        if ((harf_kucuk(metin_cp(s, (n - 2LL))) != 112LL))
        {
            return false;
        }
        return (harf_kucuk(metin_cp(s, (n - 1LL))) == 104LL);
    }
}

bool liste_ad_var(long long L, char* s) {
    {
        long long i = 0LL;
        while ((i < liste_uzunluk(L)))
        {
            if (metin_esit(dugum_ad(liste_al(L, i)), s))
            {
                return true;
            }
            (i = (i + 1LL));
        }
        return false;
    }
}

void liste_ad_ekle(long long L, char* s) {
    {
        liste_ekle(L, kimlik_dugum(0LL, 1LL, s));
    }
}

char* kullan_coz(char* spec, char* dahil_yol, long long satir, long long sutun) {
    {
        if ((!kutup_uzanti_mi(spec)))
        {
            hata_fmt_basla();
            hata_fmt_yaz("'kullan' yolu '.upp' veya '.uph' ile bitmeli, '");
            hata_fmt_yaz(spec);
            hata_fmt_yaz("' bulundu.");
            hata_fmt_ver(satir, sutun);
            return "";
        }
        if (os_yol_mutlak_mi(spec))
        {
            if (os_dosya_var(spec))
            {
                return os_yol_gercek(spec);
            }
            hata_fmt_basla();
            hata_fmt_yaz("'kullan' dosyası bulunamadı: ");
            hata_fmt_yaz(spec);
            hata_fmt_ver(satir, sutun);
            return "";
        }
        if (os_dosya_var(dahil_yol))
        {
            char* p = os_yol_birlestir(os_yol_dizin(dahil_yol), spec);
            if (os_dosya_var(p))
            {
                return os_yol_gercek(p);
            }
        }
        char* p2 = os_yol_birlestir(os_cwd(), spec);
        if (os_dosya_var(p2))
        {
            return os_yol_gercek(p2);
        }
        hata_fmt_basla();
        hata_fmt_yaz("'kullan' dosyası bulunamadı: ");
        hata_fmt_yaz(spec);
        hata_fmt_ver(satir, sutun);
        return "";
    }
}

void yukle_birim(char* kaynak, char* yol, long long kutuphane, long long seen, long long stack, long long merged, char* alias) {
    {
        char* gercek = intern(yol);
        if (os_dosya_var(yol))
        {
            (gercek = os_yol_gercek(yol));
        }
        if (liste_ad_var(stack, gercek))
        {
            hata_fmt_basla();
            hata_fmt_yaz("kullan döngüsü: ");
            long long i = 0LL;
            while ((i < liste_uzunluk(stack)))
            {
                if ((i > 0LL))
                {
                    hata_fmt_yaz(" -> ");
                }
                hata_fmt_yaz(dugum_ad(liste_al(stack, i)));
                (i = (i + 1LL));
            }
            hata_fmt_yaz(" -> ");
            hata_fmt_yaz(gercek);
            hata_fmt_ver(1LL, 1LL);
            return;
        }
        if (liste_ad_var(seen, gercek))
        {
            return;
        }
        liste_ad_ekle(seen, gercek);
        liste_ad_ekle(stack, gercek);
        long long kutup = kutuphane;
        if (baslik_mi(yol))
        {
            (kutup = 1LL);
        }
        jeton_sifirla();
        if ((kaynak == NULL))
        {
            (kaynak = intern(""));
        }
        else
        {
            (kaynak = intern(kaynak));
        }
        kaynak_ayarla(kaynak, intern(yol));
        ayristir_i_yaz(1LL);
        loop_yaz(0LL);
        switch_yaz(0LL);
        lex_tara();
        if (hata_var())
        {
            return;
        }
        ayristir_i_yaz(1LL);
        long long birim = ayristir_birim();
        if ((hata_var() || (birim == 0LL)))
        {
            return;
        }
        long long decls = dugum_liste(birim);
        long long i = 0LL;
        while (((i < liste_uzunluk(decls)) && (!hata_var())))
        {
            long long d = liste_al(decls, i);
            if ((dugum_tur(d) == DugumTur_D_KULLAN))
            {
                char* spec = dugum_ad(d);
                char* hedef = kullan_coz(spec, yol, dugum_satir(d), dugum_sutun(d));
                if ((hata_var() || metin_esit(hedef, "")))
                {
                    return;
                }
                char* alt = upp_dosya_oku(hedef);
                if ((alt == NULL))
                {
                    hata_fmt_basla();
                    hata_fmt_yaz("'kullan' dosyası okunamadı: ");
                    hata_fmt_yaz(hedef);
                    hata_fmt_ver(dugum_satir(d), dugum_sutun(d));
                    return;
                }
                char* alt_alias = upp_metin_kopya("");
                if ((dugum_b(d) != 0LL))
                {
                    (alt_alias = dugum_ad(dugum_b(d)));
                }
                yukle_birim(kaynak_lf(alt), hedef, 1LL, seen, stack, merged, alt_alias);
            }
            else
            {
                if ((((kutup != 0LL) && (dugum_tur(d) == DugumTur_D_FONK)) && metin_esit(dugum_ad(d), "ana")))
                {
                    hata_ver(dugum_satir(d), dugum_sutun(d), "Kütüphane dosyasında 'fonk ana' yasaktır; giriş noktası yalnızca kök dosyada olur.");
                }
                else
                {
                    if ((!metin_esit(alias, "")))
                    {
                        long long al = kimlik_dugum(dugum_satir(d), dugum_sutun(d), alias);
                        dugum_yaz_d(d, al);
                        if ((dugum_tur(d) == DugumTur_D_SINIF))
                        {
                            long long M = dugum_liste2(d);
                            long long j = 0LL;
                            while ((j < liste_uzunluk(M)))
                            {
                                dugum_yaz_d(liste_al(M, j), al);
                                (j = (j + 1LL));
                            }
                        }
                    }
                    liste_ekle(merged, d);
                }
            }
            (i = (i + 1LL));
        }
        liste_kisalt(stack);
    }
}

long long yukle_birlesik(char* kaynak, char* ad, long long ana_zorunlu) {
    {
        kaynak_ayarla(kaynak, intern(ad));
        if (((ana_zorunlu != 0LL) && baslik_mi(ad)))
        {
            hata_ver(1LL, 1LL, "Başlık dosyası (.uph) derleme girişi olamaz; bir .upp programından 'kullan' ile katın.");
            return 0LL;
        }
        long long seen = liste_yeni();
        long long stack = liste_yeni();
        long long merged = liste_yeni();
        long long kok_kutup = 0LL;
        if (baslik_mi(ad))
        {
            (kok_kutup = 1LL);
        }
        yukle_birim(kaynak, ad, kok_kutup, seen, stack, merged, "");
        if (hata_var())
        {
            return 0LL;
        }
        long long ana_say = 0LL;
        long long ana_ikinci = 0LL;
        long long i = 0LL;
        while ((i < liste_uzunluk(merged)))
        {
            long long d = liste_al(merged, i);
            if (((dugum_tur(d) == DugumTur_D_FONK) && metin_esit(dugum_ad(d), "ana")))
            {
                (ana_say = (ana_say + 1LL));
                if ((ana_say > 1LL))
                {
                    (ana_ikinci = d);
                }
            }
            (i = (i + 1LL));
        }
        if ((ana_say > 1LL))
        {
            hata_ver(dugum_satir(ana_ikinci), dugum_sutun(ana_ikinci), "Birden fazla 'fonk ana()' tanımlanamaz; giriş noktası tek olmalıdır.");
            return 0LL;
        }
        if (((ana_zorunlu != 0LL) && (ana_say == 0LL)))
        {
            hata_ver(1LL, 1LL, "Giriş noktası bulunamadı. Bir 'fonk ana()' tanımlayın.");
            return 0LL;
        }
        long long n = dugum_yeni(DugumTur_D_PROGRAM, 1LL, 1LL);
        dugum_yaz_liste(n, merged);
        return n;
    }
}

long long analiz_program(char* kaynak, char* ad, long long cok_hata) {
    {
        havuz_sifirla();
        long long ana_zorunlu = 1LL;
        if (baslik_mi(ad))
        {
            (ana_zorunlu = 0LL);
        }
        long long program = yukle_birlesik(kaynak, ad, ana_zorunlu);
        if ((hata_var() || (program == 0LL)))
        {
            return 0LL;
        }
        if ((cok_hata != 0LL))
        {
            hata_analiz_topla_ayarla(1LL);
        }
        tip_calistir(program);
        if (hata_var())
        {
            return 0LL;
        }
        guven_kontrol(program);
        if ((hata_var() || (hata_analiz_say() > 0LL)))
        {
            return 0LL;
        }
        return program;
    }
}

bool derle_kaynak(char* kaynak, char* ad) {
    {
        kaynak_ayarla(kaynak, intern(ad));
        if (baslik_mi(ad))
        {
            hata_ver(1LL, 1LL, "Başlık dosyası (.uph) derleme girişi olamaz; bir .upp programından 'kullan' ile katın.");
            return false;
        }
        long long program = analiz_program(kaynak, ad, 0LL);
        if ((program == 0LL))
        {
            return false;
        }
        uret_program(program);
        if (hata_var())
        {
            return false;
        }
        return true;
    }
}

void ast_yaz(long long node, long long derinlik) {
    {
        if ((node == 0LL))
        {
            return;
        }
        hata_fmt_basla();
        long long i = 0LL;
        while ((i < derinlik))
        {
            hata_fmt_yaz("  ");
            (i = (i + 1LL));
        }
        hata_fmt_yaz("Dugum tur=");
        hata_fmt_sayi(dugum_tur(node));
        hata_fmt_yaz(" ad=");
        hata_fmt_yaz(dugum_ad(node));
        char* satir = intern(hata_fmt_al());
        printf("%s\n", ((satir) ? (satir) : ""));
        ast_yaz(dugum_a(node), (derinlik + 1LL));
        ast_yaz(dugum_b(node), (derinlik + 1LL));
        ast_yaz(dugum_c(node), (derinlik + 1LL));
        ast_yaz(dugum_d(node), (derinlik + 1LL));
        long long L = dugum_liste(node);
        if ((L != 0LL))
        {
            (i = 0LL);
            while ((i < liste_uzunluk(L)))
            {
                ast_yaz(liste_al(L, i), (derinlik + 1LL));
                (i = (i + 1LL));
            }
        }
    }
}

long long havuz_sinir_test(void) {
    {
        havuz_sifirla();
        long long t = dugum_tur((-1LL));
        if ((!hata_var()))
        {
            printf("%s\n", "havuz test: negatif id sessiz geçti");
            return 1LL;
        }
        hata_sifirla();
        (t = dugum_tur(999999LL));
        if ((!hata_var()))
        {
            printf("%s\n", "havuz test: büyük id sessiz geçti");
            return 1LL;
        }
        hata_sifirla();
        long long L = liste_yeni();
        liste_al(L, 0LL);
        if ((!hata_var()))
        {
            printf("%s\n", "havuz test: boş liste indeksi sessiz geçti");
            return 1LL;
        }
        hata_sifirla();
        printf("%s\n", "havuz sınır testleri: tamam");
        return 0LL;
    }
}

long long harf_kucuk_ascii(long long c) {
    {
        if (((c >= 65LL) && (c <= 90LL)))
        {
            return (c + 32LL);
        }
        return c;
    }
}

bool yol_son_esit(char* s, char* uz) {
    {
        long long n = metin_uzun(s);
        long long m = metin_uzun(uz);
        if (((m <= 0LL) || (n < m)))
        {
            return false;
        }
        long long i = 0LL;
        while ((i < m))
        {
            if ((harf_kucuk_ascii(metin_cp(s, ((n - m) + i))) != harf_kucuk_ascii(metin_cp(uz, i))))
            {
                return false;
            }
            (i = (i + 1LL));
        }
        return true;
    }
}

char* yol_uzanti_kes(char* s) {
    {
        long long n = metin_uzun(s);
        if (((n >= 4LL) && (((yol_son_esit(s, ".exe") || yol_son_esit(s, ".upp")) || yol_son_esit(s, ".uph")) || yol_son_esit(s, ".out"))))
        {
            return intern_dilim(s, 0LL, (n - 4LL));
        }
        if (((n >= 2LL) && yol_son_esit(s, ".c")))
        {
            return intern_dilim(s, 0LL, (n - 2LL));
        }
        return s;
    }
}

char* dil_surum(void) {
    {
        return "3.0.1";
    }
}

long long dil_protokol(void) {
    {
        return 1LL;
    }
}

void surum_yazdir(bool json) {
    {
        if (json)
        {
            printf("%s", "{\"dil\":\"u++\",\"surum\":\"");
            printf("%s", ((dil_surum()) ? (dil_surum()) : ""));
            printf("%s", "\",\"protokol\":");
            printf("%s", ((sayi_metin(dil_protokol())) ? (sayi_metin(dil_protokol())) : ""));
            printf("%s\n", "}");
        }
        else
        {
            printf("%s", "u++ ");
            printf("%s\n", ((dil_surum()) ? (dil_surum()) : ""));
        }
    }
}

int main(int argc, char **argv) {
    upp_argv_ayarla(argc, argv);
    _upp_kuresel_hazir();
    #if defined(_WIN32)
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    #endif
    {
        long long argn = upp_arguman_sayisi();
        if (((argn >= 2LL) && metin_esit(upp_arguman(1LL), "--havuz-test")))
        {
            return (int)(havuz_sinir_test());
        }
        char* kaynak_yol = upp_metin_kopya("ornekler/ornek1.upp");
        char* cikti = upp_metin_kopya("");
        bool sadece_c = false;
        bool sadece_derle = false;
        bool dump_ast = false;
        bool json_hata = false;
        bool analiz = false;
        bool bicim = false;
        bool bicim_kontrol = false;
        bool hedef_linux = false;
        bool hedef_windows = false;
        bool surum_iste = false;
        bool surum_json = false;
        long long i = 1LL;
        while ((i < argn))
        {
            char* a = upp_arguman(i);
            if ((metin_esit(a, "--surum") || metin_esit(a, "-v")))
            {
                (surum_iste = true);
            }
            else
                if (metin_esit(a, "--json"))
                {
                    (surum_json = true);
                }
                else
                    if ((metin_esit(a, "--yardim") || metin_esit(a, "-h")))
                    {
                        printf("%s\n", "Kullanım: uppc <dosya.upp> [seçenekler]");
                        printf("%s\n", "");
                        printf("%s\n", "Seçenekler:");
                        printf("%s\n", "  --sadece-derle     Derle ve ikili üret, çalıştırma");
                        printf("%s\n", "  -c, --sadece-c     Sadece C kodu üret (.c)");
                        printf("%s\n", "  -o, --cikti <ad>   Çıktı dosya adı belirle");
                        printf("%s\n", "  --runtime <yol>    Özel C çalışma zamanı dosyası");
                        printf("%s\n", "  --analiz           LSP analiz modu (JSON hata)");
                        printf("%s\n", "  --json-hata        Hataları JSON biçiminde ver");
                        printf("%s\n", "  --ast              AST ağacını yazdır");
                        printf("%s\n", "  --bicim            Dosyayı biçimlendir");
                        printf("%s\n", "  --bicim-kontrol    Biçim kontrolü yap");
                        printf("%s\n", "  --windows          Windows hedefi için derle");
                        printf("%s\n", "  --linux            Linux hedefi için derle");
                        printf("%s\n", "  -v, --surum        Sürüm: u++ 3.0.1");
                        printf("%s\n", "  --json             --surum ile makine-okur JSON");
                        printf("%s\n", "  -h, --yardim       Bu yardım mesajını göster");
                        return (int)(0LL);
                    }
                    else
                        if (metin_esit(a, "--runtime"))
                        {
                            (i = (i + 1LL));
                            if ((i < argn))
                            {
                                os_runtime_ozel_yaz(upp_arguman(i));
                            }
                        }
                        else
                            if ((metin_esit(a, "--sadece-c") || metin_esit(a, "-c")))
                            {
                                (sadece_c = true);
                            }
                            else
                                if (metin_esit(a, "--sadece-derle"))
                                {
                                    (sadece_derle = true);
                                }
                                else
                                    if (metin_esit(a, "--ast"))
                                    {
                                        (dump_ast = true);
                                    }
                                    else
                                        if (metin_esit(a, "--json-hata"))
                                        {
                                            (json_hata = true);
                                        }
                                        else
                                            if (metin_esit(a, "--analiz"))
                                            {
                                                (analiz = true);
                                            }
                                            else
                                                if (metin_esit(a, "--bicim"))
                                                {
                                                    (bicim = true);
                                                }
                                                else
                                                    if (metin_esit(a, "--bicim-kontrol"))
                                                    {
                                                        (bicim_kontrol = true);
                                                    }
                                                    else
                                                        if (metin_esit(a, "--linux"))
                                                        {
                                                            (hedef_linux = true);
                                                        }
                                                        else
                                                            if (metin_esit(a, "--windows"))
                                                            {
                                                                (hedef_windows = true);
                                                            }
                                                            else
                                                                if ((metin_esit(a, "--cikti") || metin_esit(a, "-o")))
                                                                {
                                                                    (i = (i + 1LL));
                                                                    if ((i < argn))
                                                                    {
                                                                        (cikti = upp_arguman(i));
                                                                    }
                                                                }
                                                                else
                                                                    if (((metin_uzun(a) > 0LL) && (metin_cp(a, 0LL) != 45LL)))
                                                                    {
                                                                        (kaynak_yol = a);
                                                                    }
            (i = (i + 1LL));
        }
        if (surum_iste)
        {
            surum_yazdir(surum_json);
            return (int)(0LL);
        }
        if ((bicim && bicim_kontrol))
        {
            printf("%s\n", "[u++ HATA] --bicim ve --bicim-kontrol birlikte kullanılamaz.");
            return (int)(1LL);
        }
        if ((hedef_linux && hedef_windows))
        {
            printf("%s\n", "[u++ HATA] --linux ve --windows birlikte kullanılamaz.");
            return (int)(1LL);
        }
        if (hedef_linux)
        {
            os_hedef_yaz(1LL);
        }
        else
            if (hedef_windows)
            {
                os_hedef_yaz(0LL);
            }
        if (analiz)
        {
            (json_hata = true);
        }
        if (json_hata)
        {
            hata_json_mod_ayarla(1LL);
            (dump_ast = false);
        }
        if ((!os_dosya_var(kaynak_yol)))
        {
            hata_fmt_basla();
            hata_fmt_yaz("Dosya bulunamadı: ");
            hata_fmt_yaz(kaynak_yol);
            if (json_hata)
            {
                hata_json_yaz(1LL, 1LL, "hata", intern(hata_fmt_al()));
            }
            else
            {
                hata_fmt_basla();
                hata_fmt_yaz("[u++ HATA] Dosya bulunamadı: ");
                hata_fmt_yaz(kaynak_yol);
                printf("%s\n", ((hata_fmt_al()) ? (hata_fmt_al()) : ""));
            }
            return (int)(1LL);
        }
        if ((bicim || bicim_kontrol))
        {
            long long kod = os_bicim_dosya(kaynak_yol, (bicim_kontrol ? 1LL : 0LL));
            if ((bicim_kontrol && (kod != 0LL)))
            {
                printf("%s", "[u++ HATA] --bicim-kontrol: '");
                printf("%s", ((kaynak_yol) ? (kaynak_yol) : ""));
                printf("%s\n", "' biçimsiz.");
                return (int)(1LL);
            }
            if (((!bicim_kontrol) && (kod != 0LL)))
            {
                printf("%s\n", "[u++ HATA] biçim yazılamadı.");
                return (int)(1LL);
            }
            return (int)(0LL);
        }
        if ((!json_hata))
        {
            printf("%s", "[u++] '");
            printf("%s", ((kaynak_yol) ? (kaynak_yol) : ""));
            printf("%s\n", "' ayrıştırılıyor...");
        }
        char* kaynak = kaynak_lf(upp_dosya_oku(kaynak_yol));
        if (analiz)
        {
            long long program = analiz_program(kaynak, kaynak_yol, 1LL);
            if ((program == 0LL))
            {
                hata_json_bas();
                return (int)(1LL);
            }
            hata_json_ok();
            return (int)(0LL);
        }
        if (dump_ast)
        {
            long long program = analiz_program(kaynak, kaynak_yol, 0LL);
            if ((program == 0LL))
            {
                hata_bas();
                return (int)(1LL);
            }
            ast_yaz(program, 0LL);
            return (int)(0LL);
        }
        if ((!derle_kaynak(kaynak, kaynak_yol)))
        {
            if (json_hata)
            {
                hata_json_bas();
            }
            else
            {
                hata_bas();
            }
            return (int)(1LL);
        }
        char* c_yol = upp_metin_kopya("cikti.c");
        char* exe_yol = upp_metin_kopya("program.exe");
        char* exe_uz = upp_metin_kopya(".exe");
        if ((os_hedef_al() != 0LL))
        {
            (exe_yol = "program.out");
            (exe_uz = ".out");
        }
        if ((!metin_esit(cikti, "")))
        {
            char* kok = yol_uzanti_kes(cikti);
            hata_fmt_basla();
            hata_fmt_yaz(kok);
            hata_fmt_yaz(".c");
            (c_yol = intern(hata_fmt_al()));
            hata_fmt_basla();
            hata_fmt_yaz(kok);
            hata_fmt_yaz(exe_uz);
            (exe_yol = intern(hata_fmt_al()));
        }
        if ((!os_dosya_yaz(c_yol, tampon_al())))
        {
            hata_fmt_basla();
            hata_fmt_yaz("Çıktı yazılamadı (");
            hata_fmt_yaz(c_yol);
            hata_fmt_yaz(")");
            if (json_hata)
            {
                hata_json_yaz(1LL, 1LL, "hata", intern(hata_fmt_al()));
            }
            else
            {
                hata_fmt_basla();
                hata_fmt_yaz("[u++ HATA] Çıktı yazılamadı (");
                hata_fmt_yaz(c_yol);
                hata_fmt_yaz(")");
                printf("%s\n", ((hata_fmt_al()) ? (hata_fmt_al()) : ""));
            }
            return (int)(1LL);
        }
        if ((!json_hata))
        {
            printf("%s", "[u++] Ara C kodu üretildi: ");
            printf("%s\n", ((c_yol) ? (c_yol) : ""));
        }
        if (sadece_c)
        {
            if (json_hata)
            {
                hata_json_ok();
            }
            return (int)(0LL);
        }
        char* gcc = os_gcc_bul();
        if (metin_esit(gcc, ""))
        {
            if (json_hata)
            {
                if ((os_hedef_al() != 0LL))
                {
                    hata_json_yaz(1LL, 1LL, "hata", "GCC bulunamadı. 'gcc' paketini kurun.");
                }
                else
                {
                    hata_json_yaz(1LL, 1LL, "hata", "GCC bulunamadı. MinGW-w64 kurun.");
                }
            }
            else
            {
                if ((os_hedef_al() != 0LL))
                {
                    printf("%s\n", "[u++ HATA] GCC bulunamadı. 'gcc' paketini kurun.");
                }
                else
                {
                    printf("%s\n", "[u++ HATA] GCC bulunamadı. MinGW-w64 kurun.");
                }
            }
            return (int)(1LL);
        }
        if ((!json_hata))
        {
            if ((os_hedef_al() != 0LL))
            {
                printf("%s\n", "[u++] GCC ile native ikili derleniyor (gcc -std=gnu11 -pthread -lm)...");
            }
            else
            {
                printf("%s\n", "[u++] GCC ile native ikili derleniyor (gcc -std=gnu11 -luser32 -lwinmm -lgdi32)...");
            }
        }
        long long rc = os_gcc_cagir(gcc, c_yol, exe_yol);
        if ((rc != 0LL))
        {
            if (json_hata)
            {
                char* gcc_mesaj = os_gcc_cikti();
                if (metin_esit(gcc_mesaj, ""))
                {
                    hata_json_yaz(1LL, 1LL, "gcc", "GCC derleme başarısız");
                }
                else
                {
                    hata_json_yaz(1LL, 1LL, "gcc", gcc_mesaj);
                }
            }
            else
            {
                printf("%s\n", "[GCC DERLEME HATASI]");
            }
            return (int)(1LL);
        }
        if (json_hata)
        {
            hata_json_ok();
            return (int)(0LL);
        }
        printf("%s", "[u++] DERLEME BAŞARILI: ");
        printf("%s\n", ((exe_yol) ? (exe_yol) : ""));
        if (sadece_derle)
        {
            return (int)(0LL);
        }
        if ((os_hedef_al() != os_host_linux()))
        {
            if ((os_hedef_al() != 0LL))
            {
                printf("%s", "[u++] Hedef 'linux' bu makinede çalıştırılmıyor; ikili: ");
            }
            else
            {
                printf("%s", "[u++] Hedef 'windows' bu makinede çalıştırılmıyor; ikili: ");
            }
            printf("%s\n", ((exe_yol) ? (exe_yol) : ""));
            return (int)(0LL);
        }
        printf("%s\n", "==================================================");
        return (int)(os_program_calistir(exe_yol));
    }
}

