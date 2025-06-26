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

> Membuat proses anak dengan fork() dan mencatat PID, PPID, serta timestamp.

**Teori**

Berdasarkan paper “Process Management in Unix/Linux” (Wang., 2018) menyebutkan bahwa dalam sistem Unix/Linux, proses dibuat secara dinamis menggunakan system call fork(). Wang (2018) menjelaskan bahwa fork() menciptakan proses anak dengan salinan identik dari parent, termasuk semua file descriptor, memory image, dan pointer ke lingkungan eksekusi.

**Solusi**

Dalam kode orphan.c, proses utama akan memanggil fork() sebanyak tiga kali, salah satunya untuk menjalankan fungsi run_orphan_demonstrator(). Pada fungsi ini, terjadi fork() ulang untuk membuat child baru:

```
pid_t child_pid = fork();
if (child_pid == 0) {
    // child logic
} else if (child_pid > 0) {
    // parent logic
}
```

Dengan cara ini maka program akan mendemonstrasikan bagaimana proses baru dibuat dan berjalan sendiri.


**Video Menjalankan Program**
...

## Daftar Pustaka

Wang, K.C. (2018) Process Management in Unix/Linux. In: Systems Programming in Unix/Linux. Cham: Springer. Available at: https://doi.org/10.1007/978-3-319-92429-8_3 (Accessed: 26 June 2025).

Ritchie, D.M. and Thompson, K. (1974) ‘The UNIX time-sharing system’, Communications of the ACM, 17(7), pp. 365–375. Available at: https://doi.org/10.1145/361011.361061 (Accessed: 26 June 2025).

Jia, Z., Li, S., Yu, T., Liao, X. and Wang, J. (2019) ‘Automatically detecting missing cleanup for ungraceful exits’, Proceedings of the 2019 27th ACM Joint Meeting on European Software Engineering Conference and Symposium on the Foundations of Software Engineering (ESEC/FSE), pp. 751–762. Available at: https://doi.org/10.1145/3338906.3338938 (Accessed: 26 June 2025).
