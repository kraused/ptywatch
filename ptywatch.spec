
# We follow the Fedora naming scheme:
# https://fedoraproject.org/wiki/Packaging:NamingGuidelines?rd=Packaging/NamingGuidelines#NonNumericRelease
%define checkout	20160328git0d21653

Name:		ptywatch
Version:	1.0
Release:	1.%{checkout}%{?dist}
Summary:	PtyWatch notification manager
Group:		System Environment/Base
License:	GPLv2

Source:		%{name}-%{version}-%{checkout}.tar.gz

Requires:	libnotify glib libutempter
BuildRequires:	libnotify-devel glib-devel libutempter-devel

%description
This package provides the ptywatch notification daemon and utility tools
surrounding it. Ptywatch allows to capture pty messages (generated, for example,
with wall).

%prep
%setup -q -n %{name}-%{version}-%{checkout}

%build
make

%install
make PREFIX=%{buildroot} install

%files
%defattr(-,root,root)
/usr/sbin/ptywatch.exe
/usr/bin/wait4signal.exe
%dir %attr(0755,root,root)
%dir /usr/libexec/ptywatch
/usr/libexec/ptywatch/libnotify.so
/usr/libexec/ptywatch/dbus-signal.so
%dir /usr/libexec/ptywatch
/usr/lib/systemd/system/ptywatch.service

