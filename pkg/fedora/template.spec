%global basever __MAJOR__.__MINOR__.__PATCH__
%global origrel __BUILD__
%global somajor __MAJOR__

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
* __WEEK_DAY__ __MONTH__ __MONTH_DAY__ __YEAR__ Eduardo Hopperdietzel <ehopperdietzel@gmail.com> - %{basever}-%{origrel}
