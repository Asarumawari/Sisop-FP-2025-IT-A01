# Sisop-FP-2025-IT-A01

# Final Project Sistem Operasi IT

## Peraturan
1. Waktu pengerjaan dimulai hari Kamis (19 Juni 2025) setelah soal dibagikan hingga hari Rabu (25 Juni 2025) pukul 23.59 WIB.
2. Praktikan diharapkan membuat laporan penjelasan dan penyelesaian soal dalam bentuk Readme(github).
3. Format nama repository github “Sisop-FP-2025-IT-[Kelas][Kelompok]” (contoh:Sisop-FP-2025-IT-A01).
4. Setelah pengerjaan selesai, seluruh source code dan semua script bash, awk, dan file yang berisi cron job ditaruh di github masing - masing kelompok, dan link github dikumpulkan pada form yang disediakan. Pastikan github di setting ke publik.
5. Commit terakhir maksimal 10 menit setelah waktu pengerjaan berakhir. Jika melewati maka akan dinilai berdasarkan commit terakhir.
6. Jika tidak ada pengumuman perubahan soal oleh asisten, maka soal dianggap dapat diselesaikan.
7. Jika ditemukan soal yang tidak dapat diselesaikan, harap menuliskannya pada Readme beserta permasalahan yang ditemukan.
8. Praktikan tidak diperbolehkan menanyakan jawaban dari soal yang diberikan kepada asisten maupun praktikan dari kelompok lainnya.
9. Jika ditemukan indikasi kecurangan dalam bentuk apapun di pengerjaan soal final project, maka nilai dianggap 0.
10. Pengerjaan soal final project sesuai dengan modul yang telah diajarkan.

## Kelompok A01

| Nama                   | NRP        |
| ---------------------- | ---------- |
| Jonathan Zelig Sutopo  | 5027241047 |
| Aslam Ahmad Usman      | 5027241074 |

## Deskripsi Soal

Buat program dimana dapat membuat fork yang memiliki loop untuk melakukan log dengan data PID, PPID, dan timestamp human-readable, untuk mendemonstrasikan apa yang terjadi ketika parent exit tidak terduga.

### Catatan

> Insert catatan dari pengerjaan kalian... (contoh dibawah) // hapus line ini

Struktur repository:
```
.
..
```

## Pengerjaan

> Insert poin soal...

**Teori**

...

**Solusi**

...

mendemonstrasikan apa yang terjadi ketika parent exit tidak terduga.

**Teori**

Dalam sistem operasi Unix atau Linux, saat proses induk (parent) keluar sebelum anaknya (child), maka child process akan berubah  menjadi orphan process. Orphan kemudian akan di-reparent oleh proses init atau systemd dengan PID 1.

Berdasarkan paper "Automatically Detecting Missing Cleanup for Ungraceful Exits" (Jia et al., 2019), proses yang keluar tanpa membersihkan child-nya disebut `ungraceful exit`. Hal ini menyebabkan orphan process tetap hidup dan bisa menimbulkan resource leak atau uncontrolled behavior jika child tidak diawasi lagi.
Mekanisme seperti `wait()` juga biasanya digunakan untuk menangani child secara benar. Namun jika dilewati, child tetap berjalan dan dapat terdeteksi sebagai orphan.


**Solusi**

```
if (child_pid > 0) {
    log_message("[PARENT][PID: %d]: Exiting in 2 seconds, orphaning (%d) child.\n", getpid(), child_pid);
    sleep(2); // sleep for 2 seconds to allow child to log its PPID
    log_message("[PARENT][PID: %d]: Exiting now.\n", getpid());
}
```
Bagian diatas pada `orphan.c` mensimulasikan bagaimana ungraceful exit terjadi dengan cara membiarkan parent process keluar 2 detik setelah proses `fork()` tanpa memanggil `wait()` atau `kill()`. Hal ini membuat child menjadi orphan dan tetap berjalan, mencatat PPID yang awalnya ID parent, lalu berubah menjadi 1 (systemd).

Cara ini cocok untuk mendemonstrasikan teori orphan process dan efek dari exit yang tidak ditangani dengan baik. Dalam sistem nyata, pendekatan seperti SafeExit—yang disinggung paper diatas—bisa digunakan untuk mendeteksi dan memperbaiki perilaku tidak bersih ini dengan cara mengidentifikasi child yang tidak di-kill atau file PID yang tertinggal.


**Video Menjalankan Program**
...

## Daftar Pustaka

Sitasi 1
Sitasi 2
Sitasi 3
