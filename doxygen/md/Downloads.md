# 📦 Downloads {#downloads_page}

## Pre-built Binaries

* **Fedora** : [cuarzo-xdgkit](https://copr.fedorainfracloud.org/coprs/cuarzo/software/) - *By [Cuarzo Software](https://github.com/CuarzoSoftware) (always up to date)*.

## Manual Building

Install a C++ compiler, Meson and then:

```bash
$ git clone https://github.com/CuarzoSoftware/XDGKit.git
$ cd XDGKit
$ meson setup builddir
$ cd builddir
$ meson install
```