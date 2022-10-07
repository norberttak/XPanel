find_package(PkgConfig QUIET)
pkg_check_modules(hidapi QUIET IMPORTED_TARGET GLOBAL hidapi-libusb)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(hidapi
  REQUIRED_VARS
    hidapi_LIBRARIES
    hidapi_INCLUDE_DIRS
  VERSION_VAR hidapi_VERSION
)

if(hidapi_FOUND AND NOT TARGET hidapi::hidapi)
    add_library(hidapi::hidapi ALIAS PkgConfig::hidapi)
endif()
