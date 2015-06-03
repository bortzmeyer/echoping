# $Id$

Summary: Program to test network services
Name: echoping
Version: 6.0.0
Release: 1
Copyright: GNU
Group: Utilities/Network
Packager: Stephane Bortzmeyer <bortz@users.sourceforge.net>
Source:  https://github.com/bortzmeyer/echoping/archive/master.zip
BuildRoot:    %{_tmppath}/%{name}-%{version}-build
Url: http://bortzmeyer.github.io/echoping/
Prefix: /usr/local
# Requires: openssl
# BuildRequires: openssl-devel openssl 

%description

"echoping" is a small program to test (approximatively) performances
of a remote host by sending it TCP "echo" (or other protocol, like
HTTP) packets.

%prep

%setup  -q # -n %{name}-%{version}

%build
./configure --prefix=$RPM_BUILD_ROOT/usr/local --enable-http --enable-icp --enable-smtp 
make


%clean 
rm -rf $RPM_BUILD_ROOT

%install
make DESTDIR=$RPM_BUILD_ROOT install


%files
/
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog DETAILS  NEWS README TODO 
%doc %{_mandir}/man1/echoping.1*
%doc  test-echoping-crypto    test-echoping-icp   test-echoping-local   test-echoping-remote

%changelog
* Wed Jun 30 2004 - cougar@random.ee
- Merge
* Thu Feb 20 2001 Paco <francisco.monserrat@rediris.es>
-  RPM package


