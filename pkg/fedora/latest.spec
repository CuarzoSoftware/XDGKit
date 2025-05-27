%global basever 1.0.0
%global origrel 1
%global somajor 1

Name:           cuarzo-xdgkit
Version:        %{basever}%{?origrel:_%{origrel}}
Release:        1%{?dist}
Summary:        C++ library providing utilities for working with XDG standards

License:        LGPLv2.1
URL:            https://github.com/CuarzoSoftware/XDGKit

BuildRequires:  tar
BuildRequires:  wget
BuildRequires:  meson
BuildRequires:  gcc-c++

%description
XDGKit is a C++ library providing utilities for working with XDG standards.

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.

%package        utils
Summary:        Utility applications for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    utils
The %{name}-utils package contains utility applications for %{name}.

%package        examples
Summary:        Example applications using %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    examples
The %{name}-examples package contains example applications using %{name}.

%prep
rm -rf repo
rm -f src.tar.gz
mkdir -p repo
wget -O src.tar.gz %{url}/archive/refs/tags/v%{basever}-%{origrel}.tar.gz
tar --strip-components=1 -xzvf src.tar.gz -C repo

%build
pushd repo
%meson
%meson_build

%install
pushd repo
%meson_install

%files
%license repo/LICENSE
%doc repo/BUILD repo/CHANGES repo/VERSION
%{_libdir}/libXDGKit.so.%{somajor}

%files utils
%{_bindir}/xdgkit-icon-theme-indexer

%files examples
%{_bindir}/xdgkit-icons

%files devel
%doc repo/README.md repo/doxygen
%{_includedir}/XDGKit/
%{_libdir}/libXDGKit.so
%{_libdir}/pkgconfig/XDGKit.pc

%changelog
* Tue May 27 2025 Eduardo Hopperdietzel <ehopperdietzel@gmail.com> - %{basever}-%{origrel}
- Themes can now be loaded and mapped from cache files, reducing both loading times and memory usage.
- New xdgkit-icon-theme-indexer utility for generating cache files.
- Replaced most std::string instances with std::string_view to enable direct mapping from cache.
- Added XDGIconThemeManager::reloadThemes() to dynamically rescan and reload themes.
- Added XDGIconThemeManager::evictCache() to free up memory by evicting theme data that won’t be accessed in the near future.
