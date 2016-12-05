PROGRAM readtest
  USE libsaxsdocument
  USE iso_fortran_env
  IMPLICIT NONE

  CHARACTER(len=1024) :: filename
  CHARACTER(len=64) :: fmt

  TYPE(saxs_document) :: doc
  INTEGER :: ierr

  CALL saxs_document_create(doc)

  IF (COMMAND_ARGUMENT_COUNT() .GE. 1) THEN
    CALL GET_COMMAND_ARGUMENT(1, filename)
  ELSE
    filename = "-"
  END IF

  IF (COMMAND_ARGUMENT_COUNT() .GE. 2) THEN
    CALL GET_COMMAND_ARGUMENT(2, fmt)
    WRITE(error_unit, *) "Reading from '", TRIM(filename), "' with format '", TRIM(fmt), "'"
    CALL saxs_document_read(doc, filename, fmt=fmt, status=ierr)
  ELSE
    WRITE(error_unit, *) "Reading from '", TRIM(filename), "'"
    CALL saxs_document_read(doc, filename, status=ierr)
  END IF

  WRITE(error_unit, *) "Result:", ierr

  CALL saxs_document_free(doc)

END PROGRAM readtest
