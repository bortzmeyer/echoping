Summary: Program to test network services
Name: echoping
Version: 4.1.0
Release: 1
Copyright: GNU
Group: Utilities/Network
Source0: echoping-4.1.0.tar.gz
Url: http://echoping.sourceforge.net
BuildRoot: /var/tmp/echoping
Requires: openssl-devel openssl
BuildRequires: openssl-devel openssl 


%description

"echoping" is a small program to test (approximatively) performances
of a remote host by sending it TCP "echo" (or other protocol, like
HTTP) packets.



%prep

%setup -q -n echoping-4.1.0

%build
../configure --prefix=/usr/ --enable-http --enable-icp --enable-smtp --enable-ttcp --with-ssl
make


%clean 
rm -rf $RPM_BUILD_ROOT

%install
../configure --prefix=$RPM_BUILD_ROOT/usr/ --with-ssl --enable-http --enable-icp --enable-smtp --enable-ttcp
make  install


%files  
%defattr(-,root,root)
%doc AUTHORS COPYING ChangeLog DETAILS INSTALL NEWS README TODO 
%doc  test-echoping-crypto    test-echoping-icp   test-echoping-local   test-echoping-remote

%changelog
* Thu Feb 20 2001 Paco <francisco.monserrat@rediris.es>
-  RPM package


