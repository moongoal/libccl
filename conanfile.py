import os.path as path
import re

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps
from conan.tools.files import load


class LibcclConan(ConanFile):
    name = "libccl"
    package_type = "header-library"
    license = "MIT"
    author = "Alfredo Mungo"
    url = "https://github.com/moongoal/libccl"
    description = "CCL Library"
    topics = ("collections",)
    settings = "os", "build_type", "arch"
    options = {}
    default_options = {}
    exports_sources = (
        "include/*",
        "test/*",
        "cmake/*",
        "CMakeLists.txt",
        "version.cmake",
        "README.md"
    )

    exports = "version.cmake", "LICENSE", "README.md"

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()
        cmake.install()

    def package_id(self):
        self.info.clear()

    def package_info(self):
        self.cpp_info.libs = ["libccl"]

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def set_version(self):
        cmake_version_file = load(self, path.join(self.recipe_folder, "version.cmake"))
        major_version_match = re.search(r"set\s*\(\s*CCL_VERSION_MAJOR\s*(\d+)\s*\)", cmake_version_file)
        minor_version_match = re.search(r"set\s*\(\s*CCL_VERSION_MINOR\s*(\d+)\s*\)", cmake_version_file)
        patch_version_match = re.search(r"set\s*\(\s*CCL_VERSION_PATCH\s*(\d+)\s*\)", cmake_version_file)

        if (
            not major_version_match
            or not minor_version_match
            or not patch_version_match
        ):
            raise RuntimeError("Unable to determine libccl version.")

        major_version = major_version_match.group(1)
        minor_version = minor_version_match.group(1)
        patch_version = patch_version_match.group(1)

        self.version = f"{major_version}.{minor_version}.{patch_version}"
