!
! Fortran Binding for libsaxsdocument
! Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
!
! This file is part of libsaxsdocument.
!
! libsaxsdocument is free software: you can redistribute it
! and/or modify it under the terms of the GNU Lesser General
! Public License as published by the Free Software Foundation,
! either version 3 of the License, or (at your option) any
! later version.
!
! libsaxsdocument is distributed in the hope that it will be
! useful, but WITHOUT ANY WARRANTY; without even the implied
! warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
! PURPOSE. See the GNU Lesser General Public License for
! more details.
!
! You should have received a copy of the GNU Lesser General
! Public License along with libsaxsdocument. If not,
! see <http://www.gnu.org/licenses/>.
!
MODULE libsaxsdocument
  USE ISO_C_BINDING
  INTEGER, PARAMETER :: DBL = SELECTED_REAL_KIND(p=13, r=200)

  TYPE :: saxs_document
    TYPE(C_PTR) :: c_ptr
  END TYPE

  ENUM, BIND(C)
    ENUMERATOR :: SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA = 1
    ENUMERATOR :: SAXS_CURVE_THEORETICAL_SCATTERING_DATA = 2
    ENUMERATOR :: SAXS_CURVE_SCATTERING_DATA = 3
    ENUMERATOR :: SAXS_CURVE_PROBABILITY_DATA = 4
    ENUMERATOR :: SAXS_CURVE_USER_DATA = 100
  END ENUM

  INTERFACE saxs_document_add_property
    MODULE PROCEDURE saxs_document_add_real_property
    MODULE PROCEDURE saxs_document_add_int_property
    MODULE PROCEDURE saxs_document_add_string_property
  END INTERFACE

  PRIVATE
  PUBLIC :: SAXS_CURVE_EXPERIMENTAL_SCATTERING_DATA
  PUBLIC :: SAXS_CURVE_THEORETICAL_SCATTERING_DATA
  PUBLIC :: SAXS_CURVE_PROBABILITY_DATA
  PUBLIC :: SAXS_CURVE_USER_DATA

  PUBLIC :: saxs_document
  PUBLIC :: saxs_document_create
  PUBLIC :: saxs_document_read
  PUBLIC :: saxs_document_write
  PUBLIC :: saxs_document_free
  PUBLIC :: saxs_document_filename
  PUBLIC :: saxs_document_properties            ! all properties
  PUBLIC :: saxs_document_data
  PUBLIC :: saxs_document_add_property
  PUBLIC :: saxs_document_add_data

CONTAINS
  SUBROUTINE C_F_STRING(cstr, fstr, status)
    TYPE(C_PTR), INTENT(in), TARGET :: cstr
    CHARACTER(len=*), INTENT(out)   :: fstr
    INTEGER, INTENT(out), OPTIONAL  :: status

    INTERFACE
      PURE FUNCTION strlen(string) BIND(C, NAME="strlen")
        IMPORT C_PTR, C_INT
        TYPE(C_PTR), INTENT(in), VALUE :: string
        INTEGER(C_INT) :: strlen
      END FUNCTION
    END INTERFACE

    CHARACTER(len=1, kind=C_CHAR), DIMENSION(:), POINTER :: pstr
    INTEGER :: i

    fstr = ""
    IF (C_ASSOCIATED(cstr)) THEN
      CALL C_F_POINTER(cstr, pstr, (/ strlen(cstr) /))
      DO i = 1, MIN(size(pstr), len(fstr))
        fstr(i:i) = pstr(i)
      END DO
      IF (PRESENT(status)) status = 0
    ELSE
      IF (PRESENT(status)) status = -1
    END IF
  END SUBROUTINE

  SUBROUTINE saxs_document_create(doc)
    TYPE(saxs_document), INTENT(inout) :: doc

    INTERFACE
      FUNCTION c_saxs_document_create() &
               BIND(C, NAME="saxs_document_create")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_document_create
      END FUNCTION
    END INTERFACE

    doc%c_ptr = c_saxs_document_create()
  END SUBROUTINE

  SUBROUTINE saxs_document_read(doc, filename, fmt, status)
    TYPE(saxs_document), INTENT(inout)     :: doc
    CHARACTER(len=*), INTENT(in)           :: filename
    CHARACTER(len=*), INTENT(in), OPTIONAL :: fmt
    INTEGER, INTENT(out), OPTIONAL         :: status

    INTERFACE
      FUNCTION c_saxs_document_read(doc, filename, fmt) &
               BIND(C, NAME="saxs_document_read")
        IMPORT C_PTR, C_INT, C_CHAR
        INTEGER(C_INT)                :: c_saxs_document_read
        TYPE(C_PTR), VALUE            :: doc
        CHARACTER(len=1, kind=C_CHAR) :: filename
        CHARACTER(len=1, kind=C_CHAR) :: fmt
      END FUNCTION
    END INTERFACE

    INTEGER :: result

    IF (PRESENT(fmt)) THEN
      result = c_saxs_document_read(doc%c_ptr, TRIM(filename) // C_NULL_CHAR, TRIM(fmt) // C_NULL_CHAR)
    ELSE
      result = c_saxs_document_read(doc%c_ptr, TRIM(filename) // C_NULL_CHAR, C_NULL_CHAR)
    END IF

    IF (PRESENT(status)) status = result
  END SUBROUTINE

  SUBROUTINE saxs_document_write(doc, filename, fmt, status)
    TYPE(saxs_document), INTENT(inout) :: doc
    CHARACTER(len=*), INTENT(in)       :: filename
    CHARACTER(len=*), INTENT(in)       :: fmt
    INTEGER, INTENT(out), OPTIONAL     :: status

    INTERFACE
      FUNCTION c_saxs_document_write(doc, filename, fmt) &
               BIND(C, NAME="saxs_document_write")
        IMPORT C_PTR, C_INT, C_CHAR
        INTEGER(C_INT)                :: c_saxs_document_write
        TYPE(C_PTR), INTENT(in), VALUE  :: doc
        CHARACTER(len=1, kind=C_CHAR) :: filename
        CHARACTER(len=1, kind=C_CHAR) :: fmt
      END FUNCTION
    END INTERFACE

    INTEGER :: result
    result = c_saxs_document_write(doc%c_ptr, TRIM(filename) // C_NULL_CHAR, TRIM(fmt) // C_NULL_CHAR)
    IF (PRESENT(status)) status = result
  END SUBROUTINE

  SUBROUTINE saxs_document_free(doc)
    TYPE(saxs_document), INTENT(inout) :: doc

    INTERFACE
      SUBROUTINE c_saxs_document_free(doc) &
                 BIND(C, NAME="saxs_document_free")
        IMPORT C_PTR
        TYPE(C_PTR), INTENT(in), VALUE :: doc
      END SUBROUTINE
    END INTERFACE

    CALL c_saxs_document_free(doc%c_ptr)
  END SUBROUTINE

  SUBROUTINE saxs_document_filename(doc, filename)
    TYPE(saxs_document), INTENT(inout) :: doc
    CHARACTER(len=*), INTENT(out)      :: filename

    INTERFACE
      SUBROUTINE c_saxs_document_filename(doc, filename) &
                 BIND(C, NAME="saxs_document_filename")
        IMPORT C_PTR
        TYPE(C_PTR), INTENT(in), VALUE :: doc
        TYPE(C_PTR), INTENT(in), VALUE :: filename
      END SUBROUTINE
    END INTERFACE

    TYPE(C_PTR), TARGET :: cfilename

    CALL c_saxs_document_filename(doc%c_ptr, C_LOC(cfilename))
    CALL C_F_STRING(cfilename, filename)
  END SUBROUTINE

  SUBROUTINE saxs_document_properties(doc, properties)
    TYPE(saxs_document), INTENT(inout)                         :: doc
    CHARACTER(len=*), DIMENSION(:,:), ALLOCATABLE, INTENT(out) :: properties

    INTERFACE
      FUNCTION c_saxs_document_property_count(doc) &
               BIND(C, NAME="saxs_document_property_count")
        IMPORT C_INT, C_PTR
        INTEGER(C_INT) :: c_saxs_document_property_count
        TYPE(C_PTR), INTENT(in), VALUE :: doc
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_document_property_first(doc) &
               BIND(C, NAME="saxs_document_property_first")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_document_property_first
        TYPE(C_PTR), INTENT(in), VALUE :: doc
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_property_next(prop) &
               BIND(C, NAME="saxs_property_next")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_property_next
        TYPE(C_PTR), INTENT(in), VALUE :: prop
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_property_name(prop) &
               BIND(C, NAME="saxs_property_name")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_property_name
        TYPE(C_PTR), INTENT(in), VALUE :: prop
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_property_value(prop) &
               BIND(C, NAME="saxs_property_value")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_property_value
        TYPE(C_PTR), INTENT(in), VALUE :: prop
      END FUNCTION
    END INTERFACE

    INTEGER     :: n
    TYPE(C_PTR) :: prop

    n = c_saxs_document_property_count(doc%c_ptr)
    ALLOCATE(properties(n,2))

    n = 0
    prop = c_saxs_document_property_first(doc%c_ptr)
    DO WHILE (C_ASSOCIATED(prop))
      n = n + 1
      CALL C_F_STRING(c_saxs_property_name(prop), properties(n, 1))
      CALL C_F_STRING(c_saxs_property_value(prop), properties(n, 2))
      prop = c_saxs_property_next(prop)
    END DO
  END SUBROUTINE

  SUBROUTINE saxs_document_data(doc, x, y, y_err, k, status)
    TYPE(saxs_document), INTENT(inout)                :: doc
    REAL(DBL), DIMENSION(:), ALLOCATABLE, INTENT(out) :: x, y, y_err
    INTEGER, INTENT(in)                               :: k
    INTEGER, INTENT(out)                              :: status

    INTEGER :: i

    INTERFACE
      FUNCTION c_saxs_document_curve(doc) &
               BIND(C, NAME="saxs_document_curve")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_document_curve
        TYPE(C_PTR), INTENT(in), VALUE :: doc
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_document_curve_find(doc, type) &
               BIND(C, NAME="saxs_document_curve_find")
        IMPORT C_INT, C_PTR
        TYPE(C_PTR) :: c_saxs_document_curve_find
        TYPE(C_PTR), INTENT(in), VALUE :: doc
        INTEGER(C_INT), INTENT(in), VALUE :: type
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_curve_next(curve) &
               BIND(C, NAME="saxs_curve_next")
        IMPORT C_INT, C_PTR
        TYPE(C_PTR) :: c_saxs_curve_next
        TYPE(C_PTR), INTENT(in), VALUE :: curve
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_curve_data_count(curve) &
               BIND(C, NAME="saxs_curve_data_count")
        IMPORT C_INT, C_PTR
        INTEGER(C_INT) :: c_saxs_curve_data_count
        TYPE(C_PTR), INTENT(in), VALUE :: curve
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_curve_data(curve) &
               BIND(C, NAME="saxs_curve_data")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_curve_data
        TYPE(C_PTR), INTENT(in), VALUE :: curve
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_data_next(data) &
               BIND(C, NAME="saxs_data_next")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_data_next
        TYPE(C_PTR), INTENT(in), VALUE :: data
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_data_x(data) &
               BIND(C, NAME="saxs_data_x")
        IMPORT C_DOUBLE, C_PTR
        REAL(C_DOUBLE)     :: c_saxs_data_x
        TYPE(C_PTR), INTENT(in), VALUE :: data
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_data_y(data) &
               BIND(C, NAME="saxs_data_y")
        IMPORT C_DOUBLE, C_PTR
        REAL(C_DOUBLE)     :: c_saxs_data_y
        TYPE(C_PTR), INTENT(in), VALUE :: data
      END FUNCTION
    END INTERFACE

    INTERFACE
      FUNCTION c_saxs_data_y_err(data) &
               BIND(C, NAME="saxs_data_y_err")
        IMPORT C_DOUBLE, C_PTR
        REAL(C_DOUBLE)     :: c_saxs_data_y_err
        TYPE(C_PTR), INTENT(in), VALUE :: data
      END FUNCTION
    END INTERFACE

    TYPE(C_PTR) :: curve
    TYPE(C_PTR) :: data
    INTEGER     :: n

    i = 1
    curve = c_saxs_document_curve(doc%c_ptr)
    DO WHILE(i < k .AND. C_ASSOCIATED(curve))
      curve = c_saxs_curve_next(curve)
      i     = i + 1
    END DO

    IF (C_ASSOCIATED(curve)) THEN
      n = c_saxs_curve_data_count(curve)
      ALLOCATE(x(n), y(n), y_err(n))

      n = 0
      data = c_saxs_curve_data(curve)
      DO WHILE (C_ASSOCIATED(data))
        n        = n + 1
        x(n)     = c_saxs_data_x(data)
        y(n)     = c_saxs_data_y(data)
        y_err(n) = c_saxs_data_y_err(data)
        data     = c_saxs_data_next(data)
      END DO
      status = 0
    ELSE
      status = 99
    END IF
  END SUBROUTINE

  SUBROUTINE saxs_document_add_real_property(doc, name, value)
    TYPE(saxs_document), INTENT(inout) :: doc
    CHARACTER(len=*), INTENT(in)       :: name
    REAL(DBL), INTENT(in)              :: value

    CHARACTER(len=128) :: buf
    WRITE (buf, FMT="(F0.4)") value
    CALL saxs_document_add_string_property(doc, name, ADJUSTL(buf))
  END SUBROUTINE

  SUBROUTINE saxs_document_add_int_property(doc, name, value)
    TYPE(saxs_document), INTENT(inout) :: doc
    CHARACTER(len=*), INTENT(in)       :: name
    INTEGER, INTENT(in)                :: value

    CHARACTER(len=128) :: buf
    WRITE (buf, *) value
    CALL saxs_document_add_string_property(doc, name, ADJUSTL(buf))
  END SUBROUTINE

  SUBROUTINE saxs_document_add_string_property(doc, name, value)
    TYPE(saxs_document), INTENT(inout) :: doc
    CHARACTER(len=*), INTENT(in)       :: name, value

    INTERFACE
      SUBROUTINE c_saxs_document_add_property(doc, name, value) &
                 BIND(C, NAME="saxs_document_add_property")
        IMPORT C_PTR, C_CHAR
        TYPE(C_PTR), INTENT(in), VALUE :: doc
        CHARACTER(len=1, kind=C_CHAR) :: name, value
      END SUBROUTINE
    END INTERFACE

    CALL c_saxs_document_add_property(doc%c_ptr,                  &
                                      TRIM(name) // C_NULL_CHAR,  &
                                      TRIM(value) // C_NULL_CHAR)
  END SUBROUTINE

  SUBROUTINE saxs_document_add_data(doc, x, y, y_err, title, type)
    TYPE(saxs_document), INTENT(inout)     :: doc
    REAL(DBL), DIMENSION(:), INTENT(in)    :: x, y, y_err
    CHARACTER(len=*), INTENT(in), OPTIONAL :: title
    INTEGER, INTENT(in), OPTIONAL          :: type

    INTERFACE
      FUNCTION c_saxs_document_add_curve(doc, title, type) &
                 BIND(C, NAME="saxs_document_add_curve")
        IMPORT C_CHAR, C_INT, C_PTR
        TYPE(C_PTR) :: c_saxs_document_add_curve
        TYPE(C_PTR), INTENT(in), VALUE :: doc
        CHARACTER(len=1, kind=C_CHAR) :: title
        INTEGER(C_INT), INTENT(in), VALUE :: type
      END FUNCTION
    END INTERFACE

    INTERFACE
      SUBROUTINE c_saxs_curve_add_data(curve, x, x_err, y, y_err) &
                 BIND(C, NAME="saxs_curve_add_data")
        IMPORT C_PTR, C_DOUBLE
        TYPE(C_PTR), INTENT(in),    VALUE :: curve
        REAL(C_DOUBLE), INTENT(in), VALUE :: x, x_err, y, y_err
      END SUBROUTINE
    END INTERFACE


    INTEGER :: k

    TYPE(C_PTR) :: curve
    IF (PRESENT(title) .AND. PRESENT(type)) THEN
      curve = c_saxs_document_add_curve(doc%c_ptr, TRIM(title) // C_NULL_CHAR, type)
    ELSE IF (PRESENT(title) .AND. .NOT. PRESENT(type)) THEN
      curve = c_saxs_document_add_curve(doc%c_ptr, TRIM(title) // C_NULL_CHAR, SAXS_CURVE_SCATTERING_DATA)
    ELSE IF (.NOT.PRESENT(title) .AND. PRESENT(type)) THEN
      curve = c_saxs_document_add_curve(doc%c_ptr, C_NULL_CHAR, type)
    ELSE
      curve = c_saxs_document_add_curve(doc%c_ptr, C_NULL_CHAR, SAXS_CURVE_SCATTERING_DATA)
    END IF

    DO k = 1, MIN(size(x), size(y), size(y_err))
      CALL c_saxs_curve_add_data(curve, x(k), 0.0_C_DOUBLE, y(k), y_err(k))
    END DO
  END SUBROUTINE

END MODULE

