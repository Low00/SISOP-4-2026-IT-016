# SISOP-4-2026-IT-016
*Practicum Report*

---

## Soal 1 – Save Asisten Kenz
*Author : Asisten_Kalkuluz/Ibnu*

---

## Tujuan

Membuat filesystem virtual berbasis FUSE yang melakukan mount pada sebuah folder source dan menyediakan file tambahan virtual bernama tujuan.txt yang berisi hasil gabungan informasi dari file 1.txt sampai 7.txt.

Fitur utama:

Filesystem mount menggunakan FUSE3
Read-only filesystem (passthrough file asli)
Virtual file tujuan.txt
Parsing isi file berdasarkan keyword "KOORD:"
Implementasi operasi dasar FUSE (getattr, readdir, open, read)

---

## Struktur File

```BASH
soal_1/
 └── kenz_rescue.c
```
---

## Konsep Program

Program FUSE ini bekerja dengan konsep passthrough filesystem, yaitu folder source tetap bisa diakses seperti biasa, namun ketika dimount, sistem menambahkan satu file virtual baru tujuan.txt yang tidak ada secara fisik di folder asli.

---

## Source Directory

Folder source akan dimasukkan melalui argumen saat menjalankan program:
```BASH
./kenz_rescue <source_dir> <mount_point>
```
Variabel global digunakan untuk menyimpan path folder source:
```c
static char *source_dir;
```
---

## Fungsi fullpath()
Digunakan untuk membuat path lengkap file dari folder source:
```c
static void fullpath(char fpath[PATH_MAX], const char *path) {
    strcpy(fpath, source_dir);
    strncat(fpath, path, PATH_MAX - strlen(source_dir) - 1);
}
```
Dengan fungsi ini, file yang diminta dari mount akan diarahkan ke file asli di folder source.

## Virtual file tujuan.txt

Filesystem menyediakan file virtual: tujuan.txt, File ini tidak ada secara fisik, namun bisa dibaca user setelah mount.

---

## Build content tujuan.txt

Isi file tujuan.txt dibentuk dari file 1.txt sampai 7.txt yang ada di folder source.
Program membaca semua file tersebut dan mengambil baris yang diawali dengan KOORD:.
Implementasi fungsi builder:
```c
void build_tujuan_content(char *content)
```
Format output yang dibuat:
```BASH
Tujuan Mas Amba: <hasil_koordinat>
```
Setiap koordinat digabung menjadi satu baris.

---

## Operasi Fuse

### 1. getattr()
Digunakan untuk membaca atribut file.

Jika path adalah /, dianggap sebagai directory root.
Jika path adalah /tujuan.txt, maka dianggap file virtual dengan size sesuai isi content yang dibentuk.
Selain itu, akan dilakukan lstat() ke file asli di folder source.
```c
static int x_getattr(const char *path, struct stat *stbuf,
                     struct fuse_file_info *fi)
```
---

### 2. readdir()
Digunakan untuk menampilkan daftar file pada directory root.

Membaca semua file dari folder source dengan opendir()
Menambahkan file virtual tujuan.txt secara manual
```c
static int x_readdir(const char *path, void *buf,
                     fuse_fill_dir_t filler,
                     off_t offset,
                     struct fuse_file_info *fi,
                     enum fuse_readdir_flags flags)
```
Bagian penambahan file virtual:
```c
filler(buf, "tujuan.txt", NULL, 0, 0);
```
---

### 3. open()
Digunakan untuk membuka file.

Jika file tujuan.txt, maka langsung return 0 karena virtual.
Jika file lain, maka dicek keberadaannya dengan open() mode read-only.
```c
static int x_open(const char *path, struct fuse_file_info *fi)
```
---

### 4. read()
Digunakan untuk membaca isi file.

Jika tujuan.txt, maka buffer diisi dari hasil build_tujuan_content()
Jika file normal, dilakukan pread() pada file asli di folder source
```c
static int x_read(const char *path, char *buf,
                  size_t size, off_t offset,
                  struct fuse_file_info *fi)
```
---

## Fuse Operations Struct
Program menggunakan struct operasi berikut:
```c
static struct fuse_operations operations = {
    .getattr = x_getattr,
    .readdir = x_readdir,
    .open    = x_open,
    .read    = x_read,
};
```
---

## Alur Program

User menjalankan program dengan argumen folder source dan mount point
Program menyimpan folder source ke variabel global
FUSE dijalankan melalui fuse_main()
Ketika directory dibuka → readdir() dipanggil
Ketika file dibuka → open() dipanggil
Ketika file dibaca → read() dipanggil
File tujuan.txt akan selalu dibentuk secara dinamis dari file 1.txt sampai 7.txt

---

## Cara Compile & Run

### Compile

```bash
gcc kenz_rescue.c -o kenz_rescue `pkg-config fuse3 --cflags --libs`
```

### Run
Mmunt point bernama mnt:
```bash
mkdir -p mnt
./kenz_rescue amba_files mnt
```
(dir amba_files didapat dari download link flashdisk dengan gdown lalu unzip)

### Unmount

```bash
fusermount3 -u mnt
```
---

## Kendala

Tidak ada kendala.

---

## Soal 3 – LibraryIT
*Author : HANN*

---

## Tujuan

Membuat server file sharing berbasis Samba (SMB) menggunakan Docker dengan konfigurasi akses role user berbeda serta audit logging.

Fitur utama:

Docker container untuk Samba server
Konfigurasi share folder LibraryIT
Multi-user Samba authentication
Pembagian role (staff & readonly)
Permission folder berbeda sesuai kebutuhan
Logging aktivitas Samba dengan full_audit
Logger container untuk memantau log realtime

---

## Struktur file
```bash
soal_3/
 ├── Dockerfile
 ├── docker-compose.yml
 ├── smb.conf
 ├── entrypoint.sh
 ├── data/
 │   ├── ebooks/
 │   ├── papers/
 │   ├── sourcecode/
 │   └── docs/
 └── logs/
     └── libraryit.log
```

---

## Konfigurasi Docker Compose
File: docker-compose.yml
```YAML
version: '3.8'

services:
  libraryit-server:
    build: .
    container_name: libraryit-server
    ports:
      - "1445:445"
    volumes:
      - ./data:/libraryit
      - ./logs:/var/log/samba
    restart: always

  libraryit-logger:
    image: busybox
    container_name: libraryit-logger
    depends_on:
      - libraryit-server
    volumes:
      - ./logs:/var/log/samba
    command: tail -F /var/log/samba/libraryit.log
```

Penjelasan:

libraryit-server menjalankan Samba server utama
Port host 1445 diarahkan ke port Samba 445
Folder data di-mount ke /libraryit dalam container
Folder logs di-mount ke /var/log/samba agar log tersimpan di host
libraryit-logger memonitor log menggunakan tail -F agar realtime

---

## Dockerfile

file : Dockerfile
```dockerfile
FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    samba \
    samba-vfs-modules \
    && rm -rf /var/lib/apt/lists/*

COPY smb.conf /etc/samba/smb.conf
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
```

Penjelasan:

Base image menggunakan Ubuntu terbaru
Install samba dan modul tambahan samba-vfs-modules untuk audit log
Copy konfigurasi smb.conf ke dalam container
Entry point diatur menggunakan script entrypoint.sh

---

## Entrypoint Script
File: entrypoint.sh

Script ini bertugas melakukan setup otomatis user, group, folder, permission, dan log sebelum Samba dijalankan.

### 1. Membuat Group
```bash
id -g staff &>/dev/null || groupadd staff
id -g readonly &>/dev/null || groupadd readonly
```
Group digunakan untuk membagi hak akses user:

staff → boleh akses penuh ke folder tertentu
readonly → hanya bisa membaca

### 2. Membuat User Samba
| User        | Password   | Role     |
| ----------- | ---------- | -------- |
| librarian   | lib789     | staff    |
| contributor | contrib456 | staff    |
| member      | member123  | readonly |

Contoh pembuatan user librarian :
```bash
useradd -m librarian && (echo "lib789"; echo "lib789") | smbpasswd -a -s librarian
usermod -aG staff librarian
```

### 3. Membuat folder share
```bash
mkdir -p /libraryit/ebooks /libraryit/papers /libraryit/sourcecode /libraryit/docs
```

### 4. Setting ownership dan permission
```bash
chown root:staff /libraryit/ebooks /libraryit/papers /libraryit/sourcecode
chown librarian:staff /libraryit/docs

chmod 770 /libraryit/ebooks /libraryit/papers
chmod 750 /libraryit/sourcecode
chmod 775 /libraryit/docs
```
Aturan permission:

ebooks & papers → staff bisa read/write, readonly hanya read
sourcecode → hanya staff yang bisa akses, folder disembunyikan
docs → read-only untuk umum, hanya librarian yang bisa write

### 5. Setup logging
```bash
touch /var/log/samba/libraryit.log
chmod 666 /var/log/samba/libraryit.log
```
Log dibuat agar bisa diakses container logger dan bisa dibaca host.

### 6. Menjalankan samba
```bash
nmbd -D
smbd -F --no-process-group </dev/null >> /var/log/samba/libraryit.log 2>&1
```
Penjelasan:

nmbd dijalankan di background untuk layanan NetBIOS
smbd dijalankan di foreground agar container tetap hidup
Output diarahkan ke log file libraryit.log

---

## Konfigurasi Samba
File: smb.conf

### Global Config
```INI
[global]
   workgroup = WORKGROUP
   server string = LibraryIT Server
   security = user
   map to guest = bad user

   log file = /var/log/samba/libraryit.log
   max log size = 50
   log level = 0 vfs:1

   debug prefix timestamp = no
```
Penjelasan:

security = user → wajib login user samba
log diarahkan ke /var/log/samba/libraryit.log
debug prefix timestamp = no agar output log lebih bersih

### Share Folder
#### 1. ebooks
```INI
[ebooks]
   path = /libraryit/ebooks
   browseable = yes
   read only = no
   valid users = @readonly @staff
   write list = @staff

   vfs objects = full_audit
   full_audit:prefix = [%t] [INFO] [%u] [%a] [%S]
   full_audit:success = connect disconnect openat mkdirat renameat unlinkat pwrite
   full_audit:failure = connect openat
   full_audit:syslog = no
```
Bisa dilihat semua user
staff bisa write
readonly hanya read
aktivitas dicatat menggunakan audit log

#### 2. papers
```INI
[papers]
   path = /libraryit/papers
   browseable = yes
   read only = no
   valid users = @readonly @staff
   write list = @staff

   vfs objects = full_audit
   full_audit:prefix = [%t] [INFO] [%u] [%a] [%S]
   full_audit:success = connect disconnect openat mkdirat renameat unlinkat pwrite
   full_audit:failure = connect openat
   full_audit:syslog = no
```

#### 3. sourcecode
```bash
[sourcecode]
   path = /libraryit/sourcecode
   browseable = no
   valid users = @staff
   write list = @staff

   vfs objects = full_audit
   full_audit:prefix = [%t] [INFO] [%u] [%a] [%S]
   full_audit:success = connect disconnect openat mkdirat renameat unlinkat pwrite
   full_audit:failure = connect openat
   full_audit:syslog = no
```
browseable = no membuat folder ini tersembunyi
hanya staff yang dapat akses

#### 4. docs
```INI
[docs]
   path = /libraryit/docs
   browseable = yes
   read only = yes
   valid users = @readonly @staff
   write list = librarian

   vfs objects = full_audit
   full_audit:prefix = [%t] [INFO] [%u] [%a] [%S]
   full_audit:success = connect disconnect openat mkdirat renameat unlinkat pwrite
   full_audit:failure = connect openat
   full_audit:syslog = no
```
Semua user dapat membaca docs
Hanya librarian yang boleh menulis

---

### Audit Logging (full_audit)
Samba menggunakan modul full_audit untuk mencatat aktivitas user seperti:

connect / disconnect
membuka file (openat)
membuat folder (mkdirat)
rename file (renameat)
delete file (unlinkat)
write file (pwrite)

Format log:
[time] [INFO] [username] [action] [share]

---

## Cara Compile & Run

### Jalankan Docker Compose

```bash
docker-compose up --build
```

### Stop Container

```bash
docker-compose down
```
### Cara Akses SMB
Server berjalan pada port:1445
Akses dari client Linux :
```bash
smbclient //localhost/ebooks -p 1445 -U librarian
```
Atau mount :
```bash
sudo mount -t cifs //localhost/ebooks /mnt -o user=librarian,port=1445
```
---

## Alur Program

1. User menjalankan docker-compose up --build
2. Container samba dibangun dari Dockerfile
3. entrypoint.sh berjalan otomatis:
membuat group
membuat user samba
membuat folder share
set permission
setup log
4. Samba server (smbd) berjalan
5. User dapat login ke SMB share sesuai hak akses
6. Semua aktivitas tercatat pada libraryit.log
7. Logger container melakukan monitoring log realtime

---

## Kendala

Tidak ada kendala.
