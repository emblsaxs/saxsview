#! /bin/bash
#
# After so many attempts of copying dependencies and fixing-up libraries and
# applications directly in cmake-code, eventually we go another route; there
# is only so much one can do with INSTALL(CODE ...) without going nuts escaping
# strings and variables and attempting to re-create bash functionality in cmake
# functions. Better use bash straight away.
#
# Here is what this script is supposed to do: post-install and pre-packaging,
# all files are in their correct relative location. Library dependencies still
# anywhere in the system have to be copied to this and absolute lookup paths
# in these libraries must be updated.
#
# For all binaries in the $PREFIX/bin and libraries in $PREFIX/lib do:
#  - query dependencies; for each dependecy
#  - check if it is a @rpath dependency
#  - if not, check if the file resides in /usr/local or /opt/homebrew (developer install, everything else is system)
#  - if yes, check if a file with the same basename as the dependency already
#    already exists in $PREFIX/lib
#  - if not, copy it
#  - change the absolute path of the dependency to @rpath
#
# And do all of the above recursively, of course.
#

LIBDIR=lib/saxsview
PREFIX=${1}/${LIBDIR}

libname() {
  local dylib=${1}

  # If this library is part of a framework, make sure to append the .dylib extension
  if [[ ${dylib} == *".framework"* ]] ; then
    local version=$(objdump --private-headers -macho ${dylib} | grep version | head -n1 | xargs | cut -d' ' -f3)
    echo lib$(basename ${dylib}).${version}.dylib
  else
    echo $(basename ${dylib})
  fi
}


fixup() {
  local filename=${1}
  local rpath=${2}
  local dylib=""

  echo "---"
  echo "fixing up '${filename}' with '${rpath}' ..."
  

  while read -r dylib ; do
    case ${dylib} in
      /usr/local/* | /opt/homebrew/*)
        echo "'${filename}' depends on developer library (@rpath not set): '${dylib}'"

        local dylibname=$(libname ${dylib})
        locallib=${PREFIX}/${dylibname}

        if [ ! -e ${locallib} ] ; then
          echo "copying '${dylib}' to '${locallib}'"
          cp ${dylib} ${locallib}

          #
          # Remove any signature, if present.
          # The following change in rpath invalidates any signature present,
          # resulting in an instant 'killed' of the process.
          #
          if codesign -d ${locallib} ; then
            echo "removing signature of ${locallib}"
            codesign --remove-signature ${locallib}
          fi

          echo "changing id and installing @rpath of '${locallib}'"
          install_name_tool -id "@rpath/${dylibname}" ${locallib}
          install_name_tool -add_rpath ${rpath} ${locallib}

          # Finally, sign with an ad-hoc certificate.
          codesign --sign - ${locallib}

          fixup ${locallib} ${rpath}
        fi

        echo "changing absolute path '${dylib}' to @rpath/${dylibname} in '${filename}'"
        codesign --remove-signature ${filename}
        install_name_tool -change ${dylib} \@rpath/${dylibname} ${filename}
        codesign --sign - ${filename}
        ;;

      @rpath*)
        echo "'${1}' depends on local library (@rpath already set): '${dylib}'"
        ;;

      *)
        echo "'${1}' depends on system library: '${dylib}' (do nothing)"
        ;;
    esac
  done < <(objdump -macho -dylibs-used -no-leading-headers ${filename} | cut -f1 -d'(')

  # make sure the @rpath is set, if not, set it.
  echo "Verifying presence of RPATH ${rpath} in ${filename}"
  otool -l ${filename} | grep ${rpath}
  if [ $? != 0 ] ; then
    echo "no RPATH found in ${filename}, adding RPATH '${rpath}''"
    codesign --remove-signature ${filename}
    install_name_tool -add_rpath ${rpath} ${filename}
    codesign --sign - ${filename}
  fi
}


for lib in ${1}/${LIBDIR}/* ; do
  fixup ${lib} "@loader_path"
done

for bin in ${1}/bin/* ; do
  fixup ${bin} "@executable_path/../${LIBDIR}"
done

packagesdir=$(find ${1} -name "*-packages" -type d)
for module in `find ${packagesdir} -name "*.so"` ; do
  fixup ${module} "@loader_path/../../../../../${LIBDIR}"
done


# Install Qt5 platform plugins and fix'em up.
for plugin in "platforms" "imageformats" "styles" ; do
  pluginsourcedir=$(qmake -query QT_INSTALL_PLUGINS)/${plugin}
  plugintargetdir=${1}/bin/${plugin}
  cp -r ${pluginsourcedir} ${plugintargetdir}

  for dylib in ${plugintargetdir}/* ; do
    codesign --remove-signature ${dylib}
    install_name_tool -id "@rpath/$(basename ${dylib})" ${dylib}
    codesign --sign - ${locallib}

    fixup ${dylib}  "@loader_path/../../${LIBDIR}"
  done
done


touch ${1}/bin/qt.conf