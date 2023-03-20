from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, CMakeDeps


class LibcclConan(ConanFile):
    name = "libccl"
    version = "0.0.1"
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
        "README.md"
    )

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
