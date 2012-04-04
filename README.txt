libsaxsdocument and saxsview
============================

Read, convert and view files related to Small Angle X-ray Scattering (SAXS).

The libraries, libsaxsimage, libsaxsdocument and libsaxsview, are released
under the LGPLv3, the saxsview application under the GPLv3. See the respective
COPYING-files for details.

In the initial proof-of-concept implementation, file formats of the ATSAS
programs are implemented. However, the scope of libsaxsdocument is to integrate
as many formats as are being used in SAXS. Collaborations and contribution of
reading/writing code would be highly welcome!

The saxsview application requires Qt-4.4 or later and Qwt (qwt.sourceforge.net),
the latter is included in this repository to allow for local modifications. The
build system is CMake (www.cmake.org), version 2.6.x should work. To compile
the Fortran bindings for libsaxsdocument, gcc-4.3 or later will be necessary.
Without these, an earlier version should be fine.

The code is plain C for libsaxsdocument and based on Qt/Qwt for saxsview. It
has been successfully compiled for Linux, MacOSX 10.5 and MinGW.

2010/08/02

	Daniel Franke
