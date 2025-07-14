from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout


class PlaygroundConan(ConanFile):
    name = 'playground'
    license = 'ISC'
    author = 'Ayaz Salikhov <asalikhov@ripple.com>'
    url = 'https://github.com/mathbunnyry/cpp-playground'
    description = 'C++ Playground'
    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
    }

    requires = [
        'boost/1.83.0',
        'gtest/1.14.0'
    ]

    default_options = {
    }

    exports_sources = (
        'CMakeLists.txt', 'cmake/*', 'src/*'
    )

    def configure(self):
        if self.settings.compiler == 'apple-clang':
            self.options['boost'].visibility = 'global'

    def layout(self):
        cmake_layout(self)
        # Fix this setting to follow the default introduced in Conan 1.48
        # to align with our build instructions.
        self.folders.generators = 'build/generators'

    generators = 'CMakeDeps'

    def generate(self):
        tc = CMakeToolchain(self)
        for option_name, option_value in self.options.items():
            tc.variables[option_name] = option_value
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
