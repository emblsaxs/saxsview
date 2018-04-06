!
! Fortran Binding for libsaxsimage
! Copyright (C) 2011 Daniel Franke <dfranke@users.sourceforge.net>
!
! This file is part of libsaxsimage.
!
! libsaxsimage is free software: you can redistribute it 
! and/or modify it under the terms of the GNU Lesser General
! Public License as published by the Free Software Foundation,
! either version 3 of the License, or (at your option) any
! later version.
!
! libsaxsimage is distributed in the hope that it will be
! useful, but WITHOUT ANY WARRANTY; without even the implied
! warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
! PURPOSE. See the GNU Lesser General Public License for
! more details.
!
! You should have received a copy of the GNU Lesser General
! Public License along with libsaxsimage. If not,
! see <http://www.gnu.org/licenses/>.
!
MODULE saxsimage
  USE ISO_C_BINDING
  INTEGER, PARAMETER :: DBL = SELECTED_REAL_KIND(p=13, r=200)

  TYPE :: saxs_image
    TYPE(C_PTR) :: c_ptr
  END TYPE

  PRIVATE
  PUBLIC saxs_image
  PUBLIC saxs_image_create
  PUBLIC saxs_image_read
  PUBLIC saxs_image_read_frame
  PUBLIC saxs_image_write
  PUBLIC saxs_image_free
  PUBLIC saxs_image_width
  PUBLIC saxs_image_height
  PUBLIC saxs_image_value
  PUBLIC saxs_image_set_value
  PUBLIC saxs_image_data
  PUBLIC saxs_image_set_size

CONTAINS
  SUBROUTINE saxs_image_create(img)
    TYPE(saxs_image), INTENT(inout) :: img

    INTERFACE
      FUNCTION c_saxs_image_create() &
               BIND(C, NAME="saxs_image_create")
        IMPORT C_PTR
        TYPE(C_PTR) :: c_saxs_image_create
      END FUNCTION
    END INTERFACE

    img%c_ptr = c_saxs_image_create()
  END SUBROUTINE

  SUBROUTINE saxs_image_read(img, filename, fmt, status)
    TYPE(saxs_image), INTENT(inout)        :: img
    CHARACTER(len=*), INTENT(in)           :: filename
    CHARACTER(len=*), INTENT(in), OPTIONAL :: fmt
    INTEGER, INTENT(out), OPTIONAL         :: status

    INTERFACE
      FUNCTION c_saxs_image_read(img, filename, fmt) &
               BIND(C, NAME="saxs_image_read")
        IMPORT C_PTR, C_INT, C_CHAR
        INTEGER(C_INT)                :: c_saxs_image_read
        TYPE(C_PTR), INTENT(in), VALUE            :: img
        CHARACTER(len=1, kind=C_CHAR), INTENT(in) :: filename
        CHARACTER(len=1, kind=C_CHAR), INTENT(in) :: fmt
      END FUNCTION
    END INTERFACE

    INTEGER :: result

    IF (PRESENT(fmt)) THEN
      result = c_saxs_image_read(img%c_ptr, TRIM(filename) // C_NULL_CHAR, TRIM(fmt) // C_NULL_CHAR)
    ELSE
      result = c_saxs_image_read(img%c_ptr, TRIM(filename) // C_NULL_CHAR, C_NULL_CHAR)
    END IF

    IF (PRESENT(status)) status = result
  END SUBROUTINE

  SUBROUTINE saxs_image_read_frame(img, filename, fmt, frame, status)
    TYPE(saxs_image), INTENT(inout)        :: img
    CHARACTER(len=*), INTENT(in)           :: filename
    CHARACTER(len=*), INTENT(in), OPTIONAL :: fmt
    INTEGER, INTENT(in), OPTIONAL          :: frame
    INTEGER, INTENT(out), OPTIONAL         :: status

    INTERFACE
      FUNCTION c_saxs_image_read_frame(img, frame) &
               BIND(C, NAME="saxs_image_read_frame")
        IMPORT C_PTR, C_INT, C_CHAR
        INTEGER(C_INT)                :: c_saxs_image_read_frame
        TYPE(C_PTR), INTENT(in), VALUE            :: img
        INTEGER(C_INT), INTENT(in), VALUE         :: frame
      END FUNCTION
    END INTERFACE

    INTEGER :: result

    CALL saxs_image_read(img, filename, fmt, result)
    IF (result == 0) THEN
      result = c_saxs_image_read_frame(img%c_ptr, frame)
    END IF

    IF (PRESENT(status)) status = result
  END SUBROUTINE

  SUBROUTINE saxs_image_write(img, filename, fmt, status)
    TYPE(saxs_image), INTENT(inout)        :: img
    CHARACTER(len=*), INTENT(in)           :: filename
    CHARACTER(len=*), INTENT(in), OPTIONAL :: fmt
    INTEGER, INTENT(out), OPTIONAL         :: status

    INTERFACE
      FUNCTION c_saxs_image_write(img, filename, fmt) &
               BIND(C, NAME="saxs_image_write")
        IMPORT C_PTR, C_INT, C_CHAR
        INTEGER(C_INT)                :: c_saxs_image_write
        TYPE(C_PTR), INTENT(in), VALUE            :: img
        CHARACTER(len=1, kind=C_CHAR), INTENT(in) :: filename
        CHARACTER(len=1, kind=C_CHAR), INTENT(in) :: fmt
      END FUNCTION
    END INTERFACE

    INTEGER :: result

    IF (PRESENT(fmt)) THEN
      result = c_saxs_image_write(img%c_ptr, TRIM(filename) // C_NULL_CHAR, TRIM(fmt) // C_NULL_CHAR)
    ELSE
      result = c_saxs_image_write(img%c_ptr, TRIM(filename) // C_NULL_CHAR, C_NULL_CHAR)
    END IF

    IF (PRESENT(status)) status = result
  END SUBROUTINE

  SUBROUTINE saxs_image_free(img)
    TYPE(saxs_image), INTENT(inout) :: img

    INTERFACE
      SUBROUTINE c_saxs_image_free(img) &
                 BIND(C, NAME="saxs_image_free")
        IMPORT C_PTR
        TYPE(C_PTR), VALUE :: img
      END SUBROUTINE
    END INTERFACE

    CALL c_saxs_image_free(img%c_ptr)
  END SUBROUTINE

  PURE INTEGER FUNCTION saxs_image_width(img)
    TYPE(saxs_image), INTENT(in) :: img

    INTERFACE
      PURE FUNCTION c_saxs_image_width(img) &
                 BIND(C, NAME="saxs_image_width")
        IMPORT C_SIZE_T, C_PTR
        INTEGER(C_SIZE_T)  :: c_saxs_image_width
        TYPE(C_PTR), INTENT(in), VALUE :: img
      END FUNCTION
    END INTERFACE

    ! Explicit conversion from size_t to default integer kind
    ! avoids -Wconversion warnings
    saxs_image_width = INT(c_saxs_image_width(img%c_ptr))
  END FUNCTION

  PURE INTEGER FUNCTION saxs_image_height(img)
    TYPE(saxs_image), INTENT(in) :: img

    INTERFACE
      PURE FUNCTION c_saxs_image_height(img) &
                 BIND(C, NAME="saxs_image_height")
        IMPORT C_SIZE_T, C_PTR
        INTEGER(C_SIZE_T)  :: c_saxs_image_height
        TYPE(C_PTR), INTENT(in), VALUE :: img
      END FUNCTION
    END INTERFACE

    ! Explicit conversion from size_t to default integer kind
    ! avoids -Wconversion warnings
    saxs_image_height = INT(c_saxs_image_height(img%c_ptr))
  END FUNCTION

  PURE FUNCTION saxs_image_value(img, x, y)
    TYPE(saxs_image), INTENT(in) :: img
    INTEGER, INTENT(in)          :: x, y
    REAL(DBL)                    :: saxs_image_value

    INTERFACE
      PURE FUNCTION c_saxs_image_value(img, x, y) &
                 BIND(C, NAME="saxs_image_value")
        IMPORT                :: C_DOUBLE, C_INT, C_LONG, C_PTR
        REAL(C_DOUBLE)        :: c_saxs_image_value
        TYPE(C_PTR), INTENT(in), VALUE    :: img
        INTEGER(C_INT), INTENT(in), VALUE :: x, y
      END FUNCTION
    END INTERFACE

    saxs_image_value = c_saxs_image_value(img%c_ptr, x, y)
  END FUNCTION

  PURE SUBROUTINE saxs_image_set_value(img, x, y, value)
    TYPE(saxs_image), INTENT(inout) :: img
    INTEGER, INTENT(in)             :: x, y
    REAL(DBL), INTENT(in)           :: value

    INTERFACE
      PURE SUBROUTINE c_saxs_image_set_value(img, x, y, value) &
                 BIND(C, NAME="saxs_image_set_value")
        IMPORT                :: C_DOUBLE, C_INT, C_LONG, C_PTR
        INTEGER(C_LONG)       :: c_saxs_image_value
        TYPE(C_PTR), INTENT(in), VALUE    :: img
        INTEGER(C_INT), INTENT(in), VALUE :: x, y
        REAL(C_DOUBLE), INTENT(in), VALUE :: value
      END SUBROUTINE
    END INTERFACE

    CALL c_saxs_image_set_value(img%c_ptr, x, y, value)
  END SUBROUTINE

  PURE SUBROUTINE saxs_image_data(img, data)
    TYPE(saxs_image), INTENT(in)        :: img
    REAL(DBL), ALLOCATABLE, INTENT(out) :: data(:,:)

    INTEGER :: i, j, width, height

    width = saxs_image_width(img)
    height = saxs_image_height(img)
    ALLOCATE(data(width, height))

    DO j = 1, height
      DO i = 1, width
        data(i,j) = saxs_image_value(img, i-1, j-1)
      END DO
    END DO
  END SUBROUTINE

  SUBROUTINE saxs_image_set_size(img, x, y, n, m)
    TYPE(saxs_image), INTENT(inout) :: img
    INTEGER, INTENT(in)             :: x, y
    INTEGER, INTENT(in)             :: n, m

    INTERFACE
      PURE SUBROUTINE c_saxs_image_set_size(img, x, y, framecount, currentframe) &
                 BIND(C, NAME="saxs_image_set_size")
        IMPORT                :: C_DOUBLE, C_INT, C_LONG, C_PTR
        INTEGER(C_LONG)       :: c_saxs_image_value
        TYPE(C_PTR), VALUE    :: img
        INTEGER(C_INT), VALUE :: x, y
        INTEGER(C_INT), VALUE :: framecount, currentframe
      END SUBROUTINE
    END INTERFACE

    CALL c_saxs_image_set_size(img%c_ptr, x, y, n, m)
  END SUBROUTINE
END MODULE
