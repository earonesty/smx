# Components #

MAJOR.MINOR-RELEASE

# Policy #

The major version number will change if there are any language constructs that would make significant amounts of old code break or have to be rewritten.  In other words, code written in 1.X may not work in 2.X.

The minor version number will change if there are new features or changes that would make code **forward** incompatible.  In other words, code written in 1.3 will probably work in 1.4, but code written in 1.4 may not work in 1.3.

The release number changes every time new code is checked in to the trunk.  Packages release numbers are taken directly form the repository, so you know exactly what code went in to the release you have installed, and, you can, at any time, grab a snapshot of that release using subversion.