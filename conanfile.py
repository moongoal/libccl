from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake


class LibcclConan(ConanFile):
    name = "libccl"
    version = "0.0.1"
    license = "MIT"
    author = "Alfredo Mungo"
    url = "https://github.com/moongoal/libccl"
    description = "CCL Library"
    topics = ("collections",)
    settings = "os", "build_type", "arch"
    options = {}
    default_options = {}
    generators = ("cmake_find_package", "cmake_paths", "CMakeToolchain")
    requires = "xxhash/0.8.1"
    exports_sources = (
        "include/*",
        "test/*",
        "cmake/*",
        "CMakeLists.txt",
        "README.md"
    )

    def configure(self):
        xxhash = self.options["xxhash"]

        xxhash.shared = False
        xxhash.utility = False

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.test()

    def package_id(self):
        self.info.header_only()

    def package(self):
        self.copy("*.hpp", dst="include", src="include")

    def package_info(self):
        self.cpp_info.libs = ["libccl"]

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
