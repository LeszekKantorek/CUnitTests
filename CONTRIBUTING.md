## Pull Requests
Changes to the codebase are done by Pull Requests only.

1. Fork the repo.
2. Create feature branch from `develop`.
3. Add or adjust existing tests if code changes should be tested.
4. Update the documentation if APIs have changed.
5. Test your changes locally on different platforms if it possible.
6. Create pull request.

## License
By contributing, you agree that your contributions will be licensed under MIT License.

## Naming convention 
* Library namespace: __CUnitTests*
* Test API: test_*  (test_assert_true())
* Trivial Variables: i, n, c, etc... (If isn't clear, then make it a Local Variable)
* Local Variables: lowerCamelCase
* Global Variables: __CUNIT_TESTS_ALL_CAPS
* Pointer Variables: avoid prefixes
* Structs: __CUnitTestsCamelCase
* Struct Member Variables: lowerCamelCase
* Enums: __CUnitTestsCamelCase
* Enum Values: __CUnitTestsCamelCase_ALL_CAPS
* Functions: __CUnitTests_CamelCase
* Macros: __CUnitTests_CamelCase

## Development environment
The CUnitTests library is developed using Visual Studio 2019 and Windows Subsystem for Linux.

## Ubuntu WSL setup
``` 
sudo apt-get install gcc g++ make clang gdb build-essential rsync zip
wget https://github.com/Kitware/CMake/releases/download/v3.17.0/cmake-3.17.0-Linux-x86_64.tar.gz
tar -zxvf cmake-3.17.0-Linux-x86_64.tar.gz 
sudo cp -a cmake-3.17.0-Linux-x86_64/.  /usr/ 
```

## Local testing
```
mkdir out
cd out
cmake -DCUNITTESTS_FLAGS=ON -DCUNITTESTS_TESTING=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make && make test
```

## Debugging
``` launch.vs.json
{
	"version": "0.2.1",
	"defaults": {},
	"configurations": [
		{
			"type": "cppdbg",
			"name": "CUnitTestsSuite", //should be the same as current startup item
			"project": "CMakeLists.txt",
			"projectTarget": "CUnitTestsSuite", //should be the same as current startup item
			"debuggerConfiguration": "gdb",
			"args": [ "-e", "-s", "suite1", "-s", "suite2" ]
		}
	]
}
```

