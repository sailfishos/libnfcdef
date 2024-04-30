Name: libnfcdef

Version: 1.0.0
Release: 0
Summary: Library for parsing and building NDEF messages
License: BSD
URL: https://github.com/sailfishos/libnfcdef
Source: %{name}-%{version}.tar.bz2

%define glib_version 2.32
%define libglibutil_version 1.0.52

BuildRequires: pkgconfig
BuildRequires: pkgconfig(glib-2.0) >= %{glib_version}
BuildRequires: pkgconfig(libglibutil) >= %{libglibutil_version}

# license macro requires rpm >= 4.11
BuildRequires: pkgconfig(rpm)
%define license_support %(pkg-config --exists 'rpm >= 4.11'; echo $?)

# make_build macro appeared in rpm 4.12
%{!?make_build: %define make_build make %{_smp_mflags}}

Requires: glib2 >= %{glib_version}
Requires: libglibutil >= %{libglibutil_version}
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
The NFC Data Exchange Format (NDEF) is a common data format used for
exchanging data between NFC devices and NFC tags, or between two NFC
devices.

%package devel
Summary: Development library for %{name}
Requires: %{name} = %{version}

%description devel
This package contains the development library for %{name}.

%prep
%setup -q

%build
%make_build LIBDIR=%{_libdir} KEEP_SYMBOLS=1 release pkgconfig

%install
make LIBDIR=%{_libdir} DESTDIR=%{buildroot} install-dev

%check
make test

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/%{name}.so.*
%if %{license_support} == 0
%license LICENSE
%endif

%files devel
%defattr(-,root,root,-)
%dir %{_includedir}/nfcdef
%{_libdir}/pkgconfig/*.pc
%{_libdir}/%{name}.so
%{_includedir}/nfcdef/*.h
