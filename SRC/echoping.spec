# $Id$

Summary: Program to test network services
Name: echoping
Version: 5.0.1
Release: 1
Copyright: GNU
Group: Utilities/Network
Packager: Stephane Bortzmeyer <bortz@users.sourceforge.net>
Source0: echoping-5.0.1.tar.gz
Url: http://echoping.sourceforge.net
BuildRoot: /var/tmp/echoping
Prefix: /usr/local
# Requires: openssl
# BuildRequires: openssl-devel openssl 


%description

"echoping" is a small program to test (approximatively) performances
of a remote host by sending it TCP "echo" (or other protocol, like
HTTP) packets.



%prep

%setup -q -n echoping-5.0.1

%build
./configure --prefix=$RPM_BUILD_ROOT/usr/local --enable-http --enable-icp --enable-smtp --enable-ttcp 
make


%clean 
rm -rf $RPM_BUILD_ROOT

%install
make install


%files
/
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog DETAILS  NEWS README TODO 
%doc  test-echoping-crypto    test-echoping-icp   test-echoping-local   test-echoping-remote

%changelog
* Thu Feb 20 2001 Paco <francisco.monserrat@rediris.es>
-  RPM package


