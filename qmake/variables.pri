# This produces the installation rule for the program and resources.
# Use a default destination prefix if none is given.
isEmpty(PREFIX) {
    win32:PREFIX = C:/BambooTracker
    else:PREFIX = /usr/local
}

# Common path variables

win32|install_flat {
  DIST_INSTALL_PATH = $$PREFIX
  DATA_INSTALL_PATH = $$PREFIX
  LICENSE_INSTALL_PATH = $$PREFIX
}
else {
  DIST_INSTALL_PATH = $$PREFIX/share
  DATA_INSTALL_PATH = $${DIST_INSTALL_PATH}/BambooTracker
  LICENSE_INSTALL_PATH = $${DIST_INSTALL_PATH}/doc/BambooTracker
}

# Detects the most commonly used C/C++ compilers, sets variables to
# - help subprojects distinguish between them if needed
#   (CONFIG += clang-normal vs clang-apple),
# - get the compiler version details via generic variable names
#   (COMPILER_*_VERSION, COMPILER_VERSION) and
# - offer (shared C/C++) compiler-specific flags for pedantic warnings,
#   standard compliance and warning-disabling if needed.
#   may be too strong/optimistic when including external projects,
#   so you need to manually append/overwrite CPP_WARNING_FLAGS and
#   append it to QMAKE_CFLAGS_WARN_ON and QMAKE_CXXFLAGS_WARN_ON
message("Qt is version" $$QT_VERSION)
msvc|clang|if(gcc:!intel_icc) {
  msvc {
    message("Configured compiler is MSVC")
    CPP_WARNING_FLAGS += /W3
    !no_warnings_are_errors {
      CPP_WARNING_FLAGS += /WX
    }
    CPP_WARNING_FLAGS += /utf-8
    CPP_WARNING_FLAGS += /D_CRT_SECURE_NO_WARNINGS # non _s functions cause errors, cross-platform support for them is terrible
    COMPILER_MAJOR_VERSION = $$QT_MSC_VER
    COMPILER_MINOR_VERSION = $$QT_MSC_FULL_VER
    COMPILER_MINOR_VERSION ~= s/^$$QT_MSC_VER//
    COMPILER_PATCH_VERSION = # No patch versions

    COMPILER_VERSION = $$QT_MSC_FULL_VER
  }
  else {
    # Compiler is most likely GCC-ish
    CPP_WARNING_FLAGS += -Wall -Wextra -pedantic
    !no_warnings_are_errors {
      CPP_WARNING_FLAGS += -Werror -pedantic-errors
    }

    clang {
      defined(QMAKE_APPLE_CLANG_MAJOR_VERSION, var) {
        message("Configured compiler is Apple LLVM")
        CONFIG += clang-apple
        COMPILER_MAJOR_VERSION = $$QT_APPLE_CLANG_MAJOR_VERSION
        COMPILER_MINOR_VERSION = $$QT_APPLE_CLANG_MINOR_VERSION
        COMPILER_PATCH_VERSION = $$QT_APPLE_CLANG_PATCH_VERSION
      }
      else {
        message("Configured compiler is LLVM")
        CONFIG += clang-normal
        COMPILER_MAJOR_VERSION = $$QT_CLANG_MAJOR_VERSION
        COMPILER_MINOR_VERSION = $$QT_CLANG_MINOR_VERSION
        COMPILER_PATCH_VERSION = $$QT_CLANG_PATCH_VERSION
      }
    }
    else {
      message("Configured compiler is GCC")
      COMPILER_MAJOR_VERSION = $$QT_GCC_MAJOR_VERSION
      COMPILER_MINOR_VERSION = $$QT_GCC_MINOR_VERSION
      COMPILER_PATCH_VERSION = $$QT_GCC_PATCH_VERSION
    }

    COMPILER_VERSION = $${COMPILER_MAJOR_VERSION}.$${COMPILER_MINOR_VERSION}.$${COMPILER_PATCH_VERSION}
  }
  message("Compiler is version" $$COMPILER_VERSION)
}
else {
  message("Configured compiler is unknown, no attempt to add warning & pedantic compiler switches")
}
