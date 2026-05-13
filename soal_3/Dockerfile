FROM ubuntu:latest

RUN apt-get update && apt-get install -y \
    samba \
    samba-vfs-modules \
    && rm -rf /var/lib/apt/lists/*

COPY smb.conf /etc/samba/smb.conf
COPY entrypoint.sh /entrypoint.sh
RUN chmod +x /entrypoint.sh

ENTRYPOINT ["/entrypoint.sh"]
