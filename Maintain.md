#### Release Maintenance

Info for maintenance moved here.

#### Compiling an RPM distribution

    make check
    make dist
    cp smx-1.1.4.tar.gz ~/rpmbuild/SOURCES/
    rpmbuild -bb --clean smx.spec

#### Uploading a release:

Example for Centos 7:

    go get github.com/aktau/github-release
    github-release release --user earonesty --repo smx --tag v1.1.4-458 --name "smx-1.1.4-458" --description ""
    github-release upload --user earonesty --repo smx --tag v1.1.4-458 --name "smx-1.1.4-458.el7.x86_64.rpm"  --file smx-1.1.4-458.el7.x86_64.rpm
