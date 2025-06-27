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

Struktur repository:
```
Sisop-FP-2025-IT-A01-main
├── README.md
├── include
│   ├── process_roles.h
│   └── utils.h
├── makefile
└── src
    ├── orphan.c
    ├── process_roles.c
    └── utils.c
```

## Pengerjaan

> mendemonstrasikan apa yang terjadi ketika parent exit tidak terduga.

**Teori**

Dalam sistem operasi Unix atau Linux, ketika parent process keluar tanpa melakukan cleanup terhadap child-nya (seperti `wait()` atau `kill()`), maka child akan tetap berjalan dan menjadi orphan process. 
Berdasarkan paper "Automatically Detecting Missing Cleanup for Ungraceful Exits" (Jia et al., 2019), paper ini menyebut kondisi tersebut sebagai ungraceful exit, yaitu saat parent process mati secara tidak terduga tanpa melakukan cleanup ke child process. Hal ini berpotensi menyebabkan orphan process dan resource leak.

**Solusi**

`process_roles.c` mendemonstrasikan bagaimana ungraceful exit bekerja melalui fungsi `orphaner(int order)`, dimana fungsi ini membuat satu proses grandchild melalui `fork()`, lalu parent-nya langsung keluar tanpa `wait()`:
```
void orphaner(int order){
   ...
    pid_t grandchild_pid = fork(); 
    if (grandchild_pid > 0) {
        sleep(2);
        log_message(...);
        exit(EXIT_SUCCESS); // parent keluar → orphan terjadi
    } else if (grandchild_pid == 0) {
        while(getppid() == child_pid){
            sleep(2);
            log_message(...);
        }
        log_message(...); // proses telah orphan
        exit(EXIT_SUCCESS);
    }
}
```
Dapat dilihat bagaimana parent yang keluar tanpa menunggu membuat child akan menjadi orphan, lalu di-reparent oleh proses init atau systemd (biasanya PID 1). Hal tersebut merupakan contoh dari ungraceful exit, dimana parent tidak melakukan proses cleanup atau pengawasan terhadap terhadap child.

> Membuat proses anak dengan fork() dan mencatat PID, PPID, serta timestamp.

**Teori**

Berdasarkan paper “Process Management in Unix/Linux” (Wang., 2018) menyebutkan bahwa dalam sistem Unix/Linux, proses dibuat secara dinamis menggunakan system call fork(). Wang (2018) menjelaskan bahwa fork() menciptakan proses anak dengan salinan identik dari parent, termasuk semua file descriptor, memory image, dan pointer ke lingkungan eksekusi.

**Solusi**

Dalam file `orphan.c`, proses utama (controller) melakukan beberapa kali forking berdasarkan input dari pengguna, lalu jumlah child untuk mendemonstrasikan orphan/zombie serta jumlah file worker dapat ditentukan secara dinamis saat runtime program.
```
for (int i = 1; i <= num_children; i++) { 
    pid_t demo_pid = fork();
    if (demo_pid == 0) {
        // reset signal handler
        ...
        // tentukan apakah proses ini akan jadi orphan atau zombie
        if (...) {
            orphaner(i);
        } else {
            exit(EXIT_SUCCESS); // menjadi zombie
        }
    }
}
```
Dalam `orphan.c`, semua `fork()` hanya dilakukan oleh parent utama dan setiap child akan menjalankan perannya masing-masing secara terpisah.

> Mendemonstrasikan bagaimana proses zombie terbentuk ketika parent tidak memanggil `wait()` terhadap child yang sudah `exit()`.

**Teori**

Menurut paper “Process Management in Unix/Linux” (Wang., 2018), saat sebuah child process berhenti, maka kernel akan menyimpan status output-nya agar bisa dikumpulkan oleh parent. Jika parent tidak segera melakukan wait(), maka entry proses tersebut tidak dihapus dan proses itu disebut dengan `zombie`.

**Solusi**

Pada `orphan.c`, zombie process dibuat berdasarkan input pengguna. Jika urutan child tidak cocok dengan mode orphaning (odd/even), maka child akan langsung keluar dengan `exit()`, sementara controller tidak memanggil `wait()` secara langsung untuk child tersebut saat itu:
```
if ((odd_even && current_child_odd) || (!odd_even && !current_child_odd)) {
    orphaner(i); // akan membuat orphan
} else {
    log_message(LOG_ZOMBIE, "ZOMBIE CHILD", "Order: %d; Becoming a zombie child.", i);
    exit(EXIT_SUCCESS); // menjadi zombie
}
```

Zombie child akan tetap muncul di process list (ps -ef) sebagai proses dengan status Z/zombie hingga parent (controller) melakukan wait():
```
// controller loop
while(!shutdown_requested){
    pause();
}

// saat shutdown
while (wait(NULL) > 0);
```
Karena `wait()` baru dipanggil saat shutdown, maka selama program berjalan, zombie akan tetap ada di process table`. Dalam paper yang disebutkan, hal ini berhubungan dengan bagaimana zombie terbentuk akibat tidak adanya `wait()`.

**Video Menjalankan Program**
...

## Daftar Pustaka

Wang, K.C. (2018) Process Management in Unix/Linux. In: Systems Programming in Unix/Linux. Cham: Springer. Available at: https://doi.org/10.1007/978-3-319-92429-8_3 (Accessed: 26 June 2025).

Jia, Z., Li, S., Yu, T., Liao, X. and Wang, J. (2019) ‘Automatically detecting missing cleanup for ungraceful exits’, Proceedings of the 2019 27th ACM Joint Meeting on European Software Engineering Conference and Symposium on the Foundations of Software Engineering (ESEC/FSE), pp. 751–762. Available at: https://doi.org/10.1145/3338906.3338938 (Accessed: 26 June 2025).
