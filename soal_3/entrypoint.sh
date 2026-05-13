#!/bin/bash

# Fungsi untuk tambah grup jika belum ada
id -g staff &>/dev/null || groupadd staff
id -g readonly &>/dev/null || groupadd readonly

# Fungsi untuk tambah user jika belum ada (Poin a)
if ! id "librarian" &>/dev/null; then
    useradd -m librarian && (echo "lib789"; echo "lib789") | smbpasswd -a -s librarian
    usermod -aG staff librarian
fi

if ! id "contributor" &>/dev/null; then
    useradd -m contributor && (echo "contrib456"; echo "contrib456") | smbpasswd -a -s contributor
    usermod -aG staff contributor
fi

if ! id "member" &>/dev/null; then
    useradd -m member && (echo "member123"; echo "member123") | smbpasswd -a -s member
    usermod -aG readonly member
fi

# Buat folder dan set permission (Poin b & c)
mkdir -p /libraryit/ebooks /libraryit/papers /libraryit/sourcecode /libraryit/docs
chown root:staff /libraryit/ebooks /libraryit/papers /libraryit/sourcecode
chown librarian:staff /libraryit/docs
chmod 770 /libraryit/ebooks /libraryit/papers
chmod 750 /libraryit/sourcecode
chmod 775 /libraryit/docs

# Persiapan Log
touch /var/log/samba/libraryit.log
chmod 666 /var/log/samba/libraryit.log

echo "Samba is starting..."
# Jalankan nmbd di background
nmbd -D

# Jalankan smbd di foreground agar container tidak exit
# Kita tidak pakai redirect >> lagi di sini agar log-nya keluar di 'docker logs' juga
smbd -F --no-process-group </dev/null >> /var/log/samba/libraryit.log 2>&1
